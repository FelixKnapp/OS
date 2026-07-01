#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "memdefs.h"
#include "memory.h"
#include "fat.h"


#define SECTOR_SIZE             512
#define MAX_PATH_SIZE           256
#define MAX_FILE_HANDLES        10
#define ROOT_DIRECTORY_HANDLE   -1


#pragma pack(push, 1)
typedef struct
{
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entry_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;

    uint8_t drive_number;
    uint8_t _Reserved;
    uint8_t signature;
    uint32_t volume_id;          // serial number, value doesn't matter
    uint8_t volume_label[11];    // 11 bytes, padded with spaces
    uint8_t system_id[8];

} FAT_bootsector;
#pragma pack(pop)

typedef struct
{
    uint8_t buffer[SECTOR_SIZE];
    FAT_File open;
    bool opened;
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector_in_cluster;

} FAT_FileData;

typedef struct
{
    union
    {
        FAT_bootsector bootsector;
        uint8_t bootsector_bytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;

    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
    
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t far* g_Fat = NULL;
static uint32_t g_DataSectionLba;


bool FAT_Readbootsector(disk_t* disk)
{
    return DiskReadSectors(disk, 0, 1, g_Data->BS.bootsector_bytes);
}

bool FAT_ReadFat(disk_t* disk)
{
    return DiskReadSectors(disk, g_Data->BS.bootsector.reserved_sectors, g_Data->BS.bootsector.sectors_per_fat, g_Fat);
}

bool FAT_Init(disk_t* disk)
{
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    // read boot sector
    if (!FAT_Readbootsector(disk))
    {
        printf("FAT: read boot sector failed\r\n");
        return false;
    }

    // read FAT
    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fat_size = g_Data->BS.bootsector.bytes_per_sector * g_Data->BS.bootsector.sectors_per_fat;
    if (sizeof(FAT_Data) + fat_size >= MEMORY_FAT_SIZE)
    {
        printf("FAT: not enough memory to read FAT! Required %lu, only have %u\r\n", sizeof(FAT_Data) + fat_size, MEMORY_FAT_SIZE);
        return false;
    }
    if (!FAT_ReadFat(disk))
    {
        printf("FAT: couldnt read FAT!\r\n");
        return false;
    }

    // read root dir
    uint32_t root_dir_lba = g_Data->BS.bootsector.reserved_sectors + g_Data->BS.bootsector.sectors_per_fat * g_Data->BS.bootsector.fat_count;
    uint32_t root_dir_size = sizeof(FAT_DirectoryEntry) * g_Data->BS.bootsector.dir_entry_count;

    g_Data->RootDirectory.open.handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.open.is_directory = true;
    g_Data->RootDirectory.open.position = 0;
    g_Data->RootDirectory.open.size = sizeof(FAT_DirectoryEntry) * g_Data->BS.bootsector.dir_entry_count;
    g_Data->RootDirectory.opened = true;
    g_Data->RootDirectory.first_cluster = root_dir_lba;
    g_Data->RootDirectory.current_cluster = root_dir_lba;
    g_Data->RootDirectory.current_sector_in_cluster = 0;

    if (!DiskReadSectors(disk, root_dir_lba, 1, g_Data->RootDirectory.buffer))
    {
        printf("FAT: read root directory failed\r\n");
        return false;
    }

    // calculate data section
    uint32_t rootDirSectors = (root_dir_size + g_Data->BS.bootsector.bytes_per_sector - 1) / g_Data->BS.bootsector.bytes_per_sector;
    g_DataSectionLba = root_dir_lba + rootDirSectors;

    // reset opened files
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].opened = false;

    return true;

}

uint32_t FAT_ClusterToLba(uint32_t cluster)
{
    return g_DataSectionLba + (cluster - 2) * g_Data->BS.bootsector.sectors_per_cluster;
}


FAT_File far* FAT_OpenEntry(disk_t* disk, FAT_DirectoryEntry* entry)
{
    // find empty handle
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++)
    {
        if (!g_Data->OpenedFiles[i].opened) handle = i;
    }

    // out of handles
    if (handle < 0)
    {
        printf("FAT: out of file handles\r\n");
        return false;
    }

    // setup vars
    FAT_FileData far* fd = &g_Data->OpenedFiles[handle];
    fd->open.handle = handle;
    fd->open.is_directory = (entry->attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->open.position = 0;
    fd->open.size = entry->size;
    fd->first_cluster = entry->first_cluster_low + ((uint32_t)entry->first_cluster_high << 16);
    fd->current_cluster = fd->first_cluster;
    fd->current_sector_in_cluster = 0;

    if (!DiskReadSectors(disk, FAT_ClusterToLba(fd->current_cluster), 1, fd->buffer))
    {
        printf("FAT(Fuck All This): read error\r\n");
        return false;
    }

    fd->opened = true;
    return &fd->open;
}

bool FAT_FindFile(FAT_File far* file, const char* name, FAT_DirectoryEntry* entry_out)
{
    for (uint32_t i = 0; i < g_BootSector.dir_entry_count; i++)
    {
        if (memcmp(name, g_RootDirectory[i].name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL;
}

FAT_File* far Fat_Open(disk_t* disk, const char* path)
{
    char buffer[MAX_PATH_SIZE];

    // leading '/' ignored
    if(path[0] == '/') path++;
    
    FAT_File far* parent = NULL;
    FAT_File far* current = NULL;

    while (*path) 
    {
        // get next file/folder name from path
        bool is_last = false;
        const char* delim = strchr(path, '/');
        if(delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            is_last = true;
        }

        // find dir entry in current dir
        FAT_DirectoryEntry entry;
        if (!FAT_FindFile(current, name, &entry))
        {
            printf("FAT: Couldnt find File\r\n");
            return false;
        }
        else
        {
            // check if dir
            

            // close parent 
            FAT_Close(parent);
            parent = current;
            current = 
        }
    }
}

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "fat.h"
#include "ctype.h"


#define SECTOR_SIZE 512


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

} FAT_BootSector;
#pragma pack(pop)

typedef struct
{
    union
    {
        FAT_BootSector bootsector;
        uint8_t bootsector_bytes[SECTOR_SIZE];
    } BS;
    
} FAT_Data;

static FAT_Data* far g_Data;

uint8_t* g_Fat = NULL;
DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd;

bool FAT_ReadBootSector(DISK* disk)
{
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_Init(DISK* disk);



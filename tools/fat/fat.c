#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
} __attribute__((packed)) BootSector;


typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _Reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed)) DirectoryEntry;


BootSector g_BootSector;
uint8_t* g_Fat = NULL;
DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd;

bool readBootSector(FILE* disk)
{
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_BootSector.bytes_per_sector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.bytes_per_sector, count, disk) == count);
    return ok;
}

bool readFat(FILE* disk)
{
    g_Fat = (uint8_t*) malloc(g_BootSector.sectors_per_fat * g_BootSector.bytes_per_sector);
    return readSectors(disk, g_BootSector.reserved_sectors, g_BootSector.sectors_per_fat, g_Fat);
}

bool readRootDirectory(FILE* disk)
{
    uint32_t lba = g_BootSector.reserved_sectors + g_BootSector.sectors_per_fat * g_BootSector.fat_count;
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.dir_entry_count;
    uint32_t sectors = (size / g_BootSector.bytes_per_sector);
    if (size % g_BootSector.bytes_per_sector > 0)
        sectors++;

    g_RootDirectoryEnd = lba + sectors;
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_BootSector.bytes_per_sector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}

DirectoryEntry* findFile(const char* name)
{
    for (uint32_t i = 0; i < g_BootSector.dir_entry_count; i++)
    {
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL;
}

bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer)
{
    bool ok = true;
    uint16_t currentCluster = fileEntry->first_cluster_low;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.sectors_per_cluster;
        ok = ok && readSectors(disk, lba, g_BootSector.sectors_per_cluster, outputBuffer);
        outputBuffer += g_BootSector.sectors_per_cluster * g_BootSector.bytes_per_sector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        else
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;

    } while (ok && currentCluster < 0x0FF8);

    return ok;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        return -3;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.bytes_per_sector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(buffer);
        return -5;
    }

    for (size_t i = 0; i < fileEntry->Size; i++)
    {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(buffer);
    free(g_Fat);
    free(g_RootDirectory);
    return 0;
}

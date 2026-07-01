#pragma once 
#include "stdint.h"
#include "stdbool.h"
#include "disk.h"

#pragma pack(push, 1)

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
} FAT_DirectoryEntry;

#pragma pack(pop)

typedef struct
{
    int handle;
    bool is_directory;
    uint32_t position;
    uint32_t size;
} FAT_File;

enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};



// Initialize FAT system
bool FAT_Init(disk_t* disk);
// Open FAT path 
FAT_File far* FAT_Open(disk_t* disk, const char* path);
// Read from FAT File
uint32_t FAT_Read(disk_t* disk, FAT_File far* file, uint32_t byte_count, void* data_out);
// Read from FAT Directory Entry
bool FAT_ReadEntry(disk_t* disk, FAT_File far* file, FAT_DirectoryEntry* dir_entry);
// Close FAT File
void FAT_Close(FAT_File far* file);

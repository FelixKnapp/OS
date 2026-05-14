#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t SystemId[8];
} __attribute__((packed)) Bootsector;

Bootsector g_Bootsector;
uint8_t* g_fat = NULL;

bool readBootsector(FILE* disk)
{
    return fread(&g_Bootsector, sizeof(g_Bootsector), 1, disk) > 0;
}

bool readSectors(FILE *disk, uint32_t lba, uint32_t count, void* buffer_out)
{
    bool ok = true;
    // seek right location in file
    ok = ok && (fseek(disk, lba * g_Bootsector.BytesPerSector, SEEK_SET) == 0);
    // read number of sectors on location
    ok = ok && (fread(buffer_out, g_Bootsector.BytesPerSector, count, disk) == count);
    return ok;
}

int main(int argc, char** argv)
{
    if(argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if(!disk) {
        fprintf(stderr, "Could not open disk image %s!", argv[1]);
        return -1;
    }

    if(!readBootsector(disk)) {
        fprintf(stderr, "Could read from bootsector!");
        return -2;
    }

    return 0;
}
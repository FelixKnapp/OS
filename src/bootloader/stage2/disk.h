#pragma once

#include "stdint.h"
#include "stdbool.h"

typedef struct{
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
}disk_t;

bool DiskInit(disk_t* disk, uint8_t drive_number);
bool DiskReadSectors(disk_t* disk, uint16_t sectors, uint32_t lba, uint8_t far* data_out);

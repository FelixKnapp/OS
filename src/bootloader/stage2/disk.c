#include "disk.h"
#include "x86.h"

bool DiskInit(disk_t* disk, , uint8_t drive_number)
{
    uint8_t drive_type;
    uint16_t cylinders, sectors, heads;


    if(!x86_Disk_GetDriveParams(disk->id, &drive_type, &cylinders, &sectors, &heads)) return false;

    disk->id = drive_number;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return true;
}

void DiskLBAtoCHS(disk_t* disk, uint32_t lba, uint16_t* cylinder_out, uint16_t* sector_out, uint16_t* head_out)
{
    // sector = lba % sectors per track + 1
    *sector_out = lba % disk->sectors + 1;

    // cylinder = (lba / sectors per track) / heads
    *cylinder_out = (lba / disk->sectors) / disk->heads;

    // head = (lba / sectors per track) % heads
    *head_out = (lba / disk->sectors) % disk->heads;
}

bool DiskReadSectors(disk_t* disk, uint16_t sectors, uint32_t lba, uint8_t far* data_out)
{
    uint16_t cylinder, sector, head;

    DiskLBAtoCHS(disk, lba, &cylinder, &sector, &head);

    
}

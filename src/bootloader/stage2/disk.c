#include "disk.h"
#include "x86.h"

bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    if(!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads)) {
        return false;
    }

    disk->id = driveNumber;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;
}

bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t far* dataOut) {
}
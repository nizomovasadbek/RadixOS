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

    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut) {
    *sectorsOut = lba % disk->sectors + 1;
    *cylindersOut = (lba / disk->sectors) / disk->heads;
    *headsOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, uint8_t far* dataOut) {
    uint16_t sector, cylinder, head;
    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for(int i = 0; i < 3; i++) {
        if(x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut))
            return true;

        x86_Disk_Reset(disk->id);
    }

    return false;
}
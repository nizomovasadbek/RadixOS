#include "disk.h"
#include "x86.h"

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
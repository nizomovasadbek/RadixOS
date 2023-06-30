#include "fat.h"
#include "disk.h"
#include "stdio.h"
#include "memdefs.h"

#define SECTOR_SIZE 512

#pragma pack(push, 1)
typedef struct {

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

    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];

} FAT_BootSector;
#pragma pack(pop)

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;
} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t fat* g_Fat = NULL;
static FAT_DirectoryEntry* g_RootDirectory = NULL;
static uint32_t g_RootDirectoryEnd;

bool FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(DISK* disk) {
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_ReadRootDirectory(DISK* disk) {
    uint32_t lba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    uint32_t sectors = (size + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;

    g_RootDirectoryEnd = lba + sectors;
    return DISK_ReadSectors(disk, lba, sectors, g_RootDirectory);
}

bool FAT_Initialize(DISK* disk) {
    g_Data = (FAT_Data far*) MEMORY_FAT_ADDR;

    if(!FAT_ReadBootSector(disk)) {
        printf("FAT read boot sector failed\r\n");
        return false;
    }
    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if(sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("Not enough memory to read FAT\r\n");
        return false;
    }

    if(!FAT_ReadFat(disk)) {
        printf("Error: couldn't read FAT sector \r\n");
        return false;
    }

    g_RootDirectory = (FAT_DirectoryEntry far*) (g_Fat + fatSize);
    uint32_t rootDirSize = sizeof(DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    rootDirSize = align(rootDirSize, g_Data->BS.BootSector.BytesPerSector);

    if(sizeof(FAT_Data) + fatSize + rootDirSize >= MEMORY_FAT_SIZE) {
        printf("Not enough memory \r\n");
        return false;
    }

    if(!FAT_ReadRootDirectory(disk)) {
        printf("Error with reading root directory\r\n");
        return 0;
    }
}

FAT_File* FAT_Open(DISK* disk, const char* path) {
    if(path[0]) {
        path++;
    }
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut){
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk)== count);
    return ok;
}

DirectoryEntry* findFile(const char* name){
    for(uint32_t i = 0; i < g_BootSector.DirEntryCount; i++){
        if(memcmp(name, g_RootDirectory[i].Name, 11) == 0){
            return &g_RootDirectory[i];
        }
    }

    return NULL;
}

bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer){
    bool ok = true;
    uint16_t currentCluster = fileEntry->FirstClusterLow;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if(!(currentCluster & 1)){
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0xFFF;
        } else {
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
        }

    } while(ok && currentCluster < 0xFF8);

    return ok;
}

int main(int argc, char** argv){
    if(argc < 3){
        printf("Sintaksis: %s <disk image> <filename>\n", *argv);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");

    if(!disk){
        fprintf(stderr, "Disk faylni ochib bo'lmadi %s\n", argv[1]);
        return -1;
    }
    
    if(!readBootSector(disk)){
        fprintf(stderr, "Boot sektorini o'qib bo'lmadi\n");
        return -2;
    }

    if(!readFat(disk)){
        fprintf(stderr, "FAT formatida diskni o'qib bo'lmadi\n");
        free(g_Fat);
        return -3;
    }

    if(!readRootDirectory(disk)){
        fprintf(stderr, "Asosiy papkani o'qib bo'lmadi\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if(!fileEntry){
        fprintf(stderr, "Fayl topilmadi %s\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if(!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Faylni o'qib bo'madi %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(buffer);
        return -6;
    }

    for(size_t i = 0; i < fileEntry->Size; i++){
        if(isprint(buffer[i])) {
            fputc(buffer[i], stdout);
        } else {
            printf("<%02x>", buffer[i]);
        }
    }
    printf("\n");

    free(g_RootDirectory);
    free(g_Fat);
    return 0;
}
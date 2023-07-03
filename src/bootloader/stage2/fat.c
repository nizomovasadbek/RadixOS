#include "fat.h"
#include "disk.h"
#include "stdio.h"
#include "memdefs.h"
#include "string.h"
#include "memory.h"

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1
#define MAX_PATH_SIZE 256

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
    FAT_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint8_t Buffer[SECTOR_SIZE];
} FAT_FileData;

typedef struct {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];

} FAT_Data;

static FAT_Data far* g_Data;
static uint8_t fat* g_Fat = NULL;
static uint32_t g_DataSectionLba;

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

    uint32_t rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    g_RootDirectory = (FAT_DirectoryEntry far*) (g_Fat + fatSize);
    uint32_t rootDirSize = sizeof(DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;

    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    g_Data->RootDirectory.CurrentCluster = 0;
    g_Data->RootDirectory.FirstCluster = 0;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if(!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
        printf("Error with reading root directory\r\n");
        return false;
    }

    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;

    for(int i = 0; i < MAX_FILE_HANDLES; i++) {
        g_Data->OpenedFiles[i].Opened = false;
    }
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
    return g_DataSectionLba +  (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

FAT_File far* FAT_OpenEntry(DISK* disk, FAT_DirectoryEntry* entry) {
    int handle = -1;
    for(int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
        if(!g_Data->OpenedFiles[i].Opened) {
            handle = i;
        }
    }

    if(handle < 0) {
        printf("Error fat out of handles\r\n");
        return false;
    }

    FAT_FileData far* fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = 0;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t) entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)) {
        printf("Error reading FAT filesystem\r\n");
        return false;
    }

    fd->Opened = true;
    return &fd->Public;
}

DirectoryEntry* FAT_FindFile(const char* name){
    for(uint32_t i = 0; i < g_BootSector.DirEntryCount; i++){
        if(memcmp(name, g_RootDirectory[i].Name, 11) == 0){
            return &g_RootDirectory[i];
        }
    }

    return NULL;
}

FAT_File* FAT_Open(DISK* disk, const char* path) {
    char buffer[MAX_PATH_SIZE];
    
    if(path[0] == '/') 
        path++;
    
    FAT_File far* parent = NULL;
    FAT_File far* current = &g_Data->RootDirectory.Public;

    while(*path) {
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if(delim != NULL) {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
        } else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        FAT_DirectoryEntry entry;
        if(FAT_FindFile(current, name, &entry)) {
            if(!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
                printf("FAT: %s is not a directory\n", name);
                return NULL;
            }
            FAT_Close(parent);
            current = FAT_OpenEntry(disk, &entry);
        }
    }

    return current;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut){
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk)== count);
    return ok;
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
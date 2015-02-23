
#ifndef _SDCARD_FAT_H_
#define _SDCARD_FAT_H_

#include <stdint.h>
#include <stdbool.h>
//#include <fcntl.h>
#include "sdcard.h"
#include "fat-structs.h"

#ifndef SDCARD_PATH_PREFIX
# define SDCARD_PATH_PREFIX "/mnt/sdcard/"
#endif

#ifndef O_RDONLY
# define O_RDONLY   0x00000000
#endif
#ifndef O_WRONLY
# define O_WRONLY   0x00000001
#endif
#ifndef O_WRITE
# define O_WRITE      O_WRONLY
#endif
#ifndef O_RDWR
# define O_RDWR     0x00000002
#endif
#ifndef O_READ
# define O_READ       O_RDONLY
#endif
#ifndef O_ACCMODE
# define O_ACCMODE  0x00000003
#endif
#ifndef O_APPEND
# define O_APPEND   0x00000008
#endif
#ifndef O_SYNC
# define O_SYNC     0x00000080
#endif
#ifndef O_CREAT
# define O_CREAT    0x00000200
#endif
#ifndef O_TRUNC
# define O_TRUNC    0x00000400
#endif
#ifndef O_EXCL
# define O_EXCL     0x00000800
#endif

/** Default date for file timestamps is 1 Jan 2000 */
extern uint16_t const FAT_DEFAULT_DATE;

/** Default time for file timestamp is 1 am */
extern uint16_t const FAT_DEFAULT_TIME;

/** date field for FAT directory entry */
static inline uint16_t FAT_DATE(uint16_t year, uint8_t month, uint8_t day) {
  return (year - 1980) << 9 | month << 5 | day;
}

/** year part of FAT directory date field */
static inline uint16_t FAT_YEAR(uint16_t fatDate) {
  return 1980 + (fatDate >> 9);
}

/** month part of FAT directory date field */
static inline uint8_t FAT_MONTH(uint16_t fatDate) {
  return (fatDate >> 5) & 0XF;
}

/** day part of FAT directory date field */
static inline uint8_t FAT_DAY(uint16_t fatDate) {
  return fatDate & 0X1F;
}

/** time field for FAT directory entry */
static inline uint16_t FAT_TIME(uint8_t hour, uint8_t minute, uint8_t second) {
  return hour << 11 | minute << 5 | second >> 1;
}

/** hour part of FAT directory time field */
static inline uint8_t FAT_HOUR(uint16_t fatTime) {
  return fatTime >> 11;
}

/** minute part of FAT directory time field */
static inline uint8_t FAT_MINUTE(uint16_t fatTime) {
  return (fatTime >> 5) & 0X3F;
}

/** second part of FAT directory time field */
static inline uint8_t FAT_SECOND(uint16_t fatTime) {
  return 2 * (fatTime & 0X1F);
}

typedef struct {
  uint8_t type;
  uint32_t fileSize;
  uint32_t currentPosition;
  uint32_t currentCluster;
  uint32_t firstCluster;
  uint8_t dirIndex;
  uint32_t dirBlock;
  uint8_t flags;
  uint8_t attributes;
} SDCardFATFile;

typedef union {
  uint8_t  data[512]; // Used to access cached file data blocks.
  uint16_t fat16[256]; // Used to access cached FAT16 entries.
  uint32_t fat32[128]; // Used to access cached FAT32 entries.
  dir_t    dir[16]; // Used to access cached directory entries.
  mbr_t    mbr; // Used to access a cached MasterBoot Record.
  fbs_t    fbs; // Used to access to a cached FAT boot sector.
} SDCardCache;

typedef struct {
  SDCard_InitParams sdcard;
  SDCardCache cache;
  uint32_t cacheBlockNumber;
  uint8_t cacheDirty;
  uint32_t cacheMirrorBlock;   // block number for mirror FAT
  uint32_t allocSearchStart;   // start cluster for alloc search
  uint8_t blocksPerCluster;    // cluster size in blocks
  uint32_t blocksPerFat;       // FAT size in blocks
  uint32_t clusterCount;       // clusters in one FAT
  uint8_t clusterSizeShift;    // shift to convert cluster count to block count
  uint32_t dataStartBlock;     // first data block number
  uint8_t fatCount;            // number of FATs on volume
  uint32_t fatStartBlock;      // start block for first FAT
  uint8_t fatType;             // volume type (12, 16, OR 32)
  uint16_t rootDirEntryCount;  // number of entries in FAT16 root dir
  uint32_t rootDirStart;       // root start block for FAT16, cluster for FAT32
  SDCardFATFile root;
} SDCardFAT;

void SDCardFAT_getTime(SDCardFAT *fat, SDCardFATFile *f, uint16_t *creationDate, uint16_t *creationTime);

void SDCardFAT_initParamsInit(SDCardFAT *fat);
bool SDCardFAT_init(SDCardFAT *fat);
bool SDCardFATFile_open(SDCardFAT *fat, SDCardFATFile *f, const char *filePath, int mode);
void SDCardFATFile_close(SDCardFAT *fat, SDCardFATFile *f);
bool SDCardFATFile_seek(SDCardFAT *fat, SDCardFATFile *f, uint32_t pos);
uint32_t SDCardFATFile_available(SDCardFATFile *f);
uint32_t SDCardFATFile_size(SDCardFATFile *f);
uint32_t SDCardFATFile_position(SDCardFATFile *f);
int16_t SDCardFATFile_readByte(SDCardFAT *fat, SDCardFATFile *f);
int16_t SDCardFATFile_read(SDCardFAT *fat, SDCardFATFile *f, uint8_t *buf, uint16_t nbyte);
int16_t SDCardFATFile_write(SDCardFAT *fat, SDCardFATFile *f, uint8_t *buf, uint16_t nbyte);

#endif

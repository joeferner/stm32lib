// Adapted from https://github.com/adafruit/SD

#include <stdio.h>
#include <string.h>
#include "fat.h"

static uint8_t const CACHE_FOR_READ  = 0;
static uint8_t const CACHE_FOR_WRITE = 1;

static uint8_t const FAT_FILE_TYPE_CLOSED  = 0;
static uint8_t const FAT_FILE_TYPE_NORMAL  = 1;
static uint8_t const FAT_FILE_TYPE_ROOT16  = 2;
static uint8_t const FAT_FILE_TYPE_ROOT32  = 3;
static uint8_t const FAT_FILE_TYPE_SUBDIR  = 4;
#define              FAT_FILE_TYPE_MIN_DIR   FAT_FILE_TYPE_ROOT16

/** Default date for file timestamps is 1 Jan 2000 */
uint16_t const FAT_DEFAULT_DATE = ((2000 - 1980) << 9) | (1 << 5) | 1;

/** Default time for file timestamp is 1 am */
uint16_t const FAT_DEFAULT_TIME = (1 << 11);

// available bits
static uint8_t const F_UNUSED = 0X30;
// use unbuffered SD read
static uint8_t const F_FILE_UNBUFFERED_READ = 0X40;
// sync of directory entry required
static uint8_t const F_FILE_DIR_DIRTY = 0X80;

#define IS_CLOSED(f)     ( (f)->type == FAT_FILE_TYPE_CLOSED )
#define IS_OPEN(f)       ( (f)->type != FAT_FILE_TYPE_CLOSED )
#define IS_ROOT(f)       ( ((f)->type == FAT_FILE_TYPE_ROOT16) || ((f)->type == FAT_FILE_TYPE_ROOT32) )
#define IS_DIR(f)        ( (f)->type >= FAT_FILE_TYPE_MIN_DIR )
#define IS_FILE(f)       ( (f)->type == FAT_FILE_TYPE_NORMAL )
#define DIR_IS_FILE(f)   ( ((f)->attributes & DIR_ATT_FILE_TYPE_MASK) == 0 )
#define DIR_IS_SUBDIR(f) ( ((f)->attributes & DIR_ATT_FILE_TYPE_MASK) == DIR_ATT_DIRECTORY )

bool _SDCardFAT_openRoot(SDCardFAT *fat);
bool _SDCardFAT_readVolume(SDCardFAT *fat, int part);
bool _SDCardFAT_cacheRead(SDCardFAT *fat, uint32_t block, int action);
bool _SDCardFAT_cacheFlush(SDCardFAT *fat);
bool _SDCardFAT_get(SDCardFAT *fat, uint32_t cluster, uint32_t *value);
bool _SDCardFAT_put(SDCardFAT *fat, uint32_t cluster, uint32_t value);
bool _SDCardFAT_fatPutEOC(SDCardFAT *fat, uint32_t cluster);
bool _SDCardFAT_make83Name(const char *fileName, char *name);
bool _SDCardFAT_chainSize(SDCardFAT *fat, uint32_t cluster, uint32_t *size);
bool _SDCardFAT_isEOC(SDCardFAT *fat, uint32_t cluster);
uint8_t _SDCardFAT_blockOfCluster(SDCardFAT *fat, uint32_t position);
uint32_t _SDCardFAT_clusterStartBlock(SDCardFAT *fat, uint32_t cluster);
bool _SDCardFAT_cacheRawBlock(SDCardFAT *fat, uint32_t blockNumber, uint8_t action);
bool _SDCardFAT_freeChain(SDCardFAT *fat, uint32_t cluster);
bool _SDCardFAT_allocContiguous(SDCardFAT *fat, uint32_t count, uint32_t *curCluster);
bool _SDCardFAT_cacheZeroBlock(SDCardFAT *fat, uint32_t blockNumber);

bool _SDCardFATFile_getParentDir(SDCardFAT *fat, SDCardFATFile *f, const char *filePath, int *pathIdx);
bool _SDCardFATFile_openFile(SDCardFAT *fat, SDCardFATFile *f, SDCardFATFile *parentDir, const char *fileName, int mode);
dir_t *_SDCardFATFile_readDirCache(SDCardFAT *fat, SDCardFATFile *f);
dir_t *_SDCardFATFile_cacheDirEntry(SDCardFAT *fat, SDCardFATFile *f, uint8_t action);
bool _SDCardFATFile_addDirCluster(SDCardFAT *fat, SDCardFATFile *f);
bool _SDCardFATFile_truncate(SDCardFAT *fat, SDCardFATFile *f, uint32_t length);
bool _SDCardFATFile_sync(SDCardFAT *fat, SDCardFATFile *f);
bool _SDCardFATFile_addCluster(SDCardFAT *fat, SDCardFATFile *f);

bool SDCardFAT_setup(SDCardFAT *fat) {
  printf("BEGIN SDCard Fat Setup\n");
  fat->cacheBlockNumber = 0XFFFFFFFF;
  fat->cacheDirty = 0;
  fat->cacheMirrorBlock = 0;

  if (!_SDCardFAT_readVolume(fat, 1)) {
    if (!_SDCardFAT_readVolume(fat, 0)) {
      return false;
    }
  }

  if (!_SDCardFAT_openRoot(fat)) {
    return false;
  }

  printf("END SDCard Fat Setup\n");
  return true;
}

bool _SDCardFAT_readVolume(SDCardFAT *fat, int part) {
  uint32_t volumeStartBlock = 0;

  if (part > 4) {
    return false;
  }

  // if part == 0 assume super floppy with FAT boot sector in block zero
  // if part > 0 assume mbr volume with partition table
  if (part) {
    if (!_SDCardFAT_cacheRead(fat, volumeStartBlock, CACHE_FOR_READ)) {
      return false;
    }
    part_t *p = &fat->cache.mbr.part[part - 1];
    if ((p->boot & 0X7F) != 0 || p->totalSectors < 100 || p->firstSector == 0) {
      // not a valid partition
      return false;
    }
    volumeStartBlock = p->firstSector;
  }

  if (!_SDCardFAT_cacheRead(fat, volumeStartBlock, CACHE_FOR_READ)) {
    return false;
  }

  bpb_t *bpb = &fat->cache.fbs.bpb;
  if (bpb->bytesPerSector != 512 || bpb->fatCount == 0 || bpb->reservedSectorCount == 0 || bpb->sectorsPerCluster == 0) {
    // not valid FAT volume
    return false;
  }

  fat->fatCount = bpb->fatCount;
  fat->blocksPerCluster = bpb->sectorsPerCluster;

  // determine shift that is same as multiply by blocksPerCluster_
  fat->clusterSizeShift = 0;
  while (fat->blocksPerCluster != (1 << fat->clusterSizeShift)) {
    // error if not power of 2
    if (fat->clusterSizeShift++ > 7) {
      return false;
    }
  }
  fat->blocksPerFat = bpb->sectorsPerFat16 ? bpb->sectorsPerFat16 : bpb->sectorsPerFat32;

  fat->fatStartBlock = volumeStartBlock + bpb->reservedSectorCount;

  // count for FAT16 zero for FAT32
  fat->rootDirEntryCount = bpb->rootDirEntryCount;

  // directory start for FAT16 dataStart for FAT32
  fat->rootDirStart = fat->fatStartBlock + bpb->fatCount * fat->blocksPerFat;

  // data start for FAT16 and FAT32
  fat->dataStartBlock = fat->rootDirStart + ((32 * bpb->rootDirEntryCount + 511) / 512);

  // total blocks for FAT16 or FAT32
  uint32_t totalBlocks = bpb->totalSectors16 ? bpb->totalSectors16 : bpb->totalSectors32;

  // total data blocks
  fat->clusterCount = totalBlocks - (fat->dataStartBlock - volumeStartBlock);

  // divide by cluster size to get cluster count
  fat->clusterCount >>= fat->clusterSizeShift;

  // FAT type is determined by cluster count
  if (fat->clusterCount < 4085) {
    fat->fatType = 12;
  } else if (fat->clusterCount < 65525) {
    fat->fatType = 16;
  } else {
    fat->rootDirStart = bpb->fat32RootCluster;
    fat->fatType = 32;
  }

  printf("blocksPerFat: %lu\n", fat->blocksPerFat);
  printf("fatStartBlock: %lu\n", fat->fatStartBlock);
  printf("rootDirEntryCount: %u\n", fat->rootDirEntryCount);
  printf("rootDirStart: %lu\n", fat->rootDirStart);
  printf("dataStartBlock: %lu\n", fat->dataStartBlock);
  printf("totalBlocks: %lu\n", totalBlocks);
  printf("clusterCount: %lu\n", fat->clusterCount);
  printf("fatType: %u\n", fat->fatType);

  return true;
}

bool _SDCardFATFile_getParentDir(SDCardFAT *fat, SDCardFATFile *f, const char *filePath, int *pathIdx) {
  SDCardFATFile *parent = &fat->root; // start with the mostparent, root!
  SDCardFATFile *subdir;
  SDCardFATFile *t;
  char subDirName[13];
  uint8_t idx;
  const char *originalPath = filePath;

  while (strchr(filePath, '/')) {
    // get rid of leading /'s
    if (filePath[0] == '/') {
      filePath++;
      continue;
    }

    if (!strchr(filePath, '/')) {
      // it was in the root directory, so leave now
      break;
    }

    // extract just the name of the next subdirectory
    idx = strchr(filePath, '/') - filePath;
    if (idx > 12) {
      idx = 12; // dont let them specify long names
    }
    strncpy(subDirName, filePath, idx);
    subDirName[idx] = 0;

    // close the subdir (we reuse them) if open
    SDCardFATFile_close(fat, subdir);
    if (!SDCardFATFile_open(fat, parent, subDirName, O_RDONLY)) {
      return false;
    }

    // move forward to the next subdirectory
    filePath += idx;

    // we reuse the objects, close it.
    SDCardFATFile_close(fat, parent);

    // swap the pointers
    t = parent;
    parent = subdir;
    subdir = t;
  }

  *pathIdx = (int)(filePath - originalPath);

  memcpy(f, parent, sizeof(SDCardFATFile));
  return true;
}

bool _SDCardFAT_get(SDCardFAT *fat, uint32_t cluster, uint32_t *value) {
  if (cluster > (fat->clusterCount + 1)) {
    return false;
  }

  uint32_t lba = fat->fatStartBlock;
  lba += fat->fatType == 16 ? cluster >> 8 : cluster >> 7;
  if (lba != fat->cacheBlockNumber) {
    if (!_SDCardFAT_cacheRead(fat, lba, CACHE_FOR_READ)) {
      return false;
    }
  }
  if (fat->fatType == 16) {
    *value = fat->cache.fat16[cluster & 0XFF];
  } else {
    *value = fat->cache.fat32[cluster & 0X7F] & FAT32MASK;
  }
  return true;
}

bool SDCardFATFile_seek(SDCardFAT *fat, SDCardFATFile *f, uint32_t pos) {
  // error if file not open or seek past end of file
  if (IS_CLOSED(f) || (pos > f->fileSize)) {
    return false;
  }

  if (f->type == FAT_FILE_TYPE_ROOT16) {
    f->currentPosition = pos;
    return true;
  }

  if (pos == 0) {
    // set position to start of file
    f->currentCluster = 0;
    f->currentPosition = 0;
    return true;
  }

  // calculate cluster index for cur and new position
  uint32_t nCur = (f->currentPosition - 1) >> (fat->clusterSizeShift + 9);
  uint32_t nNew = (pos - 1) >> (fat->clusterSizeShift + 9);

  if (nNew < nCur || f->currentPosition == 0) {
    // must follow chain from first cluster
    f->currentCluster = f->firstCluster;
  } else {
    // advance from curPosition
    nNew -= nCur;
  }

  while (nNew--) {
    if (!_SDCardFAT_get(fat, f->currentCluster, &f->currentCluster)) {
      return false;
    }
  }

  f->currentPosition = pos;
  return true;
}

bool _SDCardFAT_make83Name(const char *fileName, char *name) {
  uint8_t c;
  uint8_t n = 7;  // max index for part before dot
  uint8_t i = 0;
  // blank fill name and extension
  while (i < 11) {
    name[i++] = ' ';
  }
  i = 0;
  while ((c = *fileName++) != '\0') {
    if (c == '.') {
      if (n == 10) {
        return false; // only one dot allowed
      }
      n = 10;  // max index for full 8.3 name
      i = 8;   // place for extension
    } else {
      // illegal FAT characters
      uint8_t b;
      const uint8_t valid[] = "|<>^+=?/[];,*\"\\";
      const uint8_t *p = valid;
      while ((b = *p++)) {
        if (b == c) {
          return false;
        }
      }

      // check size and only allow ASCII printable characters
      if (i > n || c < 0X21 || c > 0X7E) {
        return false;
      }

      // only upper case allowed in 8.3 names - convert lower to upper
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }

  // must have a file name, extension is optional
  return name[0] != ' ';
}

bool _SDCardFAT_isEOC(SDCardFAT *fat, uint32_t cluster) {
  return cluster >= (fat->fatType == 16 ? FAT16EOC_MIN : FAT32EOC_MIN);
}

bool _SDCardFAT_chainSize(SDCardFAT *fat, uint32_t cluster, uint32_t *size) {
  uint32_t s = 0;
  do {
    if (!_SDCardFAT_get(fat, cluster, &cluster)) {
      return false;
    }
    s += 512UL << fat->clusterSizeShift;
  } while (!_SDCardFAT_isEOC(fat, cluster));
  *size = s;
  return true;
}

bool _SDCardFAT_openCachedEntry(SDCardFAT *fat, SDCardFATFile *f, uint8_t dirIndex, uint8_t mode) {
  // location of entry in cache
  dir_t *p = fat->cache.dir + dirIndex;

  // write or truncate is an error for a directory or read-only file
  if (p->attributes & (DIR_ATT_READ_ONLY | DIR_ATT_DIRECTORY)) {
    if (mode & (O_WRITE | O_TRUNC)) {
      return false;
    }
  }

  // remember location of directory entry on SD
  f->dirIndex = dirIndex;
  f->dirBlock = fat->cacheBlockNumber;

  // copy first cluster number for directory fields
  f->firstCluster = (uint32_t)p->firstClusterHigh << 16;
  f->firstCluster |= p->firstClusterLow;

  // make sure it is a normal file or subdirectory
  if (DIR_IS_FILE(p)) {
    f->fileSize = p->fileSize;
    f->type = FAT_FILE_TYPE_NORMAL;
  } else if (DIR_IS_SUBDIR(p)) {
    if (!_SDCardFAT_chainSize(fat, f->firstCluster, &f->fileSize)) {
      return false;
    }
    f->type = FAT_FILE_TYPE_SUBDIR;
  } else {
    return false;
  }
  // save open flags for read/write
  f->flags = mode & (O_ACCMODE | O_SYNC | O_APPEND);

  // set to start of file
  f->currentCluster = 0;
  f->currentPosition = 0;

  // truncate file to zero length if requested
  if (mode & O_TRUNC) {
    return _SDCardFATFile_truncate(fat, f, 0);
  }
  return true;
}

uint8_t _SDCardFAT_blockOfCluster(SDCardFAT *fat, uint32_t position) {
  return (position >> 9) & (fat->blocksPerCluster - 1);
}

uint32_t _SDCardFAT_clusterStartBlock(SDCardFAT *fat, uint32_t cluster) {
  return fat->dataStartBlock + ((cluster - 2) << fat->clusterSizeShift);
}

bool _SDCardFAT_cacheRawBlock(SDCardFAT *fat, uint32_t blockNumber, uint8_t action) {
  if (fat->cacheBlockNumber != blockNumber) {
    if (!_SDCardFAT_cacheFlush(fat)) {
      return false;
    }
    if (!SDCard_readBlock(&fat->sdcard, blockNumber, fat->cache.data)) {
      return false;
    }
    fat->cacheBlockNumber = blockNumber;
  }
  fat->cacheDirty |= action;
  return true;
}

//------------------------------------------------------------------------------
/**
 * Write data to an open file.
 *
 * \note Data is moved to the cache but may not be written to the
 * storage device until sync() is called.
 *
 * \param[in] buf Pointer to the location of the data to be written.
 *
 * \param[in] nbyte Number of bytes to write.
 *
 * \return For success write() returns the number of bytes written, always
 * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
 * include write() is called before a file has been opened, write is called
 * for a read-only file, device is full, a corrupt file system or an I/O error.
 *
 */
int16_t SDCardFATFile_write(SDCardFAT *fat, SDCardFATFile *f, uint8_t *buf, uint16_t nbyte) {
  // number of bytes left to write  -  must be before goto statements
  uint16_t nToWrite = nbyte;

  // error if not a normal file or is read-only
  if (!IS_FILE(f) || !(f->flags & O_WRITE)) {
    goto writeErrorReturn;
  }

  // seek to end of file if append flag
  if ((f->flags & O_APPEND) && f->currentPosition != f->fileSize) {
    if (!SDCardFATFile_seek(fat, f, f->fileSize)) {
      goto writeErrorReturn;
    }
  }

  while (nToWrite > 0) {
    uint8_t blockOfCluster = _SDCardFAT_blockOfCluster(fat, f->currentPosition);
    uint16_t blockOffset = f->currentPosition & 0X1FF;
    if (blockOfCluster == 0 && blockOffset == 0) {
      // start of new cluster
      if (f->currentCluster == 0) {
        if (f->firstCluster == 0) {
          // allocate first cluster of file
          if (!_SDCardFATFile_addCluster(fat, f)) {
            goto writeErrorReturn;
          }
        } else {
          f->currentCluster = f->firstCluster;
        }
      } else {
        uint32_t next;
        if (!_SDCardFAT_get(fat, f->currentCluster, &next)) {
          goto writeErrorReturn;
        }
        if (_SDCardFAT_isEOC(fat, next)) {
          // add cluster if at end of chain
          if (!_SDCardFATFile_addCluster(fat, f)) {
            goto writeErrorReturn;
          }
        } else {
          f->currentCluster = next;
        }
      }
    }
    // max space in block
    uint16_t n = 512 - blockOffset;

    // lesser of space and amount to write
    if (n > nToWrite) {
      n = nToWrite;
    }

    // block for data write
    uint32_t block = _SDCardFAT_clusterStartBlock(fat, f->currentCluster) + blockOfCluster;
    if (n == 512) {
      // full block - don't need to use cache
      // invalidate cache if block is in cache
      if (fat->cacheBlockNumber == block) {
        fat->cacheBlockNumber = 0XFFFFFFFF;
      }
      if (!SDCard_writeBlock(&fat->sdcard, block, buf)) {
        goto writeErrorReturn;
      }
      buf += 512;
    } else {
      if (blockOffset == 0 && f->currentPosition >= f->fileSize) {
        // start of new block don't need to read into cache
        if (!_SDCardFAT_cacheFlush(fat)) {
          goto writeErrorReturn;
        }
        fat->cacheBlockNumber = block;
        fat->cacheDirty |= CACHE_FOR_WRITE;
      } else {
        // rewrite part of block
        if (!_SDCardFAT_cacheRawBlock(fat, block, CACHE_FOR_WRITE)) {
          goto writeErrorReturn;
        }
      }
      uint8_t *dst = fat->cache.data + blockOffset;
      uint8_t *end = dst + n;
      while (dst != end) {
        *dst++ = *buf++;
      }
    }
    nToWrite -= n;
    f->currentPosition += n;
  }
  if (f->currentPosition > f->fileSize) {
    // update fileSize and insure sync will update dir entry
    f->fileSize = f->currentPosition;
    f->flags |= F_FILE_DIR_DIRTY;
  } else if (nbyte) {
    // insure sync will update modified date and time
    f->flags |= F_FILE_DIR_DIRTY;
  }

  if (f->flags & O_SYNC) {
    if (!_SDCardFATFile_sync(fat, f)) {
      goto writeErrorReturn;
    }
  }
  return nbyte;

writeErrorReturn:
  return 0;
}

int16_t SDCardFATFile_read(SDCardFAT *fat, SDCardFATFile *f, uint8_t *buf, uint16_t nbyte) {
  // error if not open or write only
  if (IS_CLOSED(f)) {
    return -1;
  }

  // max bytes left in file
  if (nbyte > (f->fileSize - f->currentPosition)) {
    nbyte = f->fileSize - f->currentPosition;
  }

  // amount left to read
  uint16_t toRead = nbyte;
  while (toRead > 0) {
    uint32_t block;  // raw device block number
    uint16_t offset = f->currentPosition & 0X1FF;  // offset in block
    if (f->type == FAT_FILE_TYPE_ROOT16) {
      block = fat->rootDirStart + (f->currentPosition >> 9);
    } else {
      uint8_t blockOfCluster = _SDCardFAT_blockOfCluster(fat, f->currentPosition);
      if (offset == 0 && blockOfCluster == 0) {
        // start of new cluster
        if (f->currentPosition == 0) {
          // use first cluster in file
          f->currentCluster = f->firstCluster;
        } else {
          // get next cluster from FAT
          if (!_SDCardFAT_get(fat, f->currentCluster, &f->currentCluster)) {
            return -1;
          }
        }
      }
      block = _SDCardFAT_clusterStartBlock(fat, f->currentCluster) + blockOfCluster;
    }
    uint16_t n = toRead;

    // amount to be read from current block
    if (n > (512 - offset)) {
      n = 512 - offset;
    }

    // read block to cache and copy data to caller
    if (!_SDCardFAT_cacheRawBlock(fat, block, CACHE_FOR_READ)) {
      return -1;
    }
    uint8_t *src = fat->cache.data + offset;
    uint8_t *end = src + n;
    while (src != end) {
      *buf++ = *src++;
    }
    f->currentPosition += n;
    toRead -= n;
  }
  return nbyte;
}

int16_t _SDCardFATFile_readByte(SDCardFAT *fat, SDCardFATFile *f) {
  uint8_t b;
  return SDCardFATFile_read(fat, f, &b, 1) == 1 ? b : -1;
}

//------------------------------------------------------------------------------
// Read next directory entry into the cache
// Assumes file is correctly positioned
dir_t *_SDCardFATFile_readDirCache(SDCardFAT *fat, SDCardFATFile *f) {
  // error if not directory
  if (!IS_DIR(f)) {
    return NULL;
  }

  // index of entry in cache
  uint8_t i = (f->currentPosition >> 5) & 0XF;

  // use read to locate and cache block
  if (_SDCardFATFile_readByte(fat, f) < 0) {
    return NULL;
  }

  // advance to next entry
  f->currentPosition += 31;

  // return pointer to entry
  return (fat->cache.dir + i);
}

//------------------------------------------------------------------------------
// cache a file's directory entry
// return pointer to cached entry or null for failure
dir_t *_SDCardFATFile_cacheDirEntry(SDCardFAT *fat, SDCardFATFile *f, uint8_t action) {
  if (!_SDCardFAT_cacheRawBlock(fat, f->dirBlock, action)) {
    return NULL;
  }
  return fat->cache.dir + f->dirIndex;
}

bool _SDCardFAT_fatPutEOC(SDCardFAT *fat, uint32_t cluster) {
  return _SDCardFAT_put(fat, cluster, 0x0FFFFFFF);
}

//------------------------------------------------------------------------------
// find a contiguous group of clusters
bool _SDCardFAT_allocContiguous(SDCardFAT *fat, uint32_t count, uint32_t *curCluster) {
  // start of group
  uint32_t bgnCluster;

  // flag to save place to start next search
  uint8_t setStart;

  // set search start cluster
  if (*curCluster) {
    // try to make file contiguous
    bgnCluster = *curCluster + 1;

    // don't save new start location
    setStart = false;
  } else {
    // start at likely place for free cluster
    bgnCluster = fat->allocSearchStart;

    // save next search start if one cluster
    setStart = 1 == count;
  }

  // end of group
  uint32_t endCluster = bgnCluster;

  // last cluster of FAT
  uint32_t fatEnd = fat->clusterCount + 1;

  // search the FAT for free clusters
  for (uint32_t n = 0;; n++, endCluster++) {
    // can't find space checked all clusters
    if (n >= fat->clusterCount) {
      return false;
    }

    // past end - start from beginning of FAT
    if (endCluster > fatEnd) {
      bgnCluster = endCluster = 2;
    }
    uint32_t f;
    if (!_SDCardFAT_get(fat, endCluster, &f)) {
      return false;
    }

    if (f != 0) {
      // cluster in use try next cluster as bgnCluster
      bgnCluster = endCluster + 1;
    } else if ((endCluster - bgnCluster + 1) == count) {
      // done - found space
      break;
    }
  }
  // mark end of chain
  if (!_SDCardFAT_fatPutEOC(fat, endCluster)) {
    return false;
  }

  // link clusters
  while (endCluster > bgnCluster) {
    if (!_SDCardFAT_put(fat, endCluster - 1, endCluster)) {
      return false;
    }
    endCluster--;
  }
  if (*curCluster != 0) {
    // connect chains
    if (!_SDCardFAT_put(fat, *curCluster, bgnCluster)) {
      return false;
    }
  }
  // return first cluster number to caller
  *curCluster = bgnCluster;

  // remember possible next free cluster
  if (setStart) {
    fat->allocSearchStart = bgnCluster + 1;
  }

  return true;
}

//------------------------------------------------------------------------------
// add a cluster to a file
bool _SDCardFATFile_addCluster(SDCardFAT *fat, SDCardFATFile *f) {
  if (!_SDCardFAT_allocContiguous(fat, 1, &f->currentCluster)) {
    return false;
  }

  // if first cluster of file link to directory entry
  if (f->firstCluster == 0) {
    f->firstCluster = f->currentCluster;
    f->flags |= F_FILE_DIR_DIRTY;
  }
  return true;
}

//------------------------------------------------------------------------------
/**
 * The sync() call causes all modified data and directory fields
 * to be written to the storage device.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include a call to sync() before a file has been
 * opened or an I/O error.
 */
bool _SDCardFATFile_sync(SDCardFAT *fat, SDCardFATFile *f) {
  // only allow open files and directories
  if (IS_CLOSED(f)) {
    return false;
  }

  if (f->flags & F_FILE_DIR_DIRTY) {
    dir_t *d = _SDCardFATFile_cacheDirEntry(fat, f, CACHE_FOR_WRITE);
    if (!d) {
      return false;
    }

    // do not set filesize for dir files
    if (!IS_DIR(f)) {
      d->fileSize = f->fileSize;
    }

    // update first cluster fields
    d->firstClusterLow = f->firstCluster & 0XFFFF;
    d->firstClusterHigh = f->firstCluster >> 16;

    // set modify time if user supplied a callback date/time function
    SDCardFAT_getTime(fat, f, &d->lastWriteDate, &d->lastWriteTime);
    d->lastAccessDate = d->lastWriteDate;
    // clear directory dirty
    f->flags &= ~F_FILE_DIR_DIRTY;
  }
  return _SDCardFAT_cacheFlush(fat);
}

//------------------------------------------------------------------------------
/**
 * Truncate a file to a specified length.  The current file position
 * will be maintained if it is less than or equal to \a length otherwise
 * it will be set to end of file.
 *
 * \param[in] length The desired length for the file.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include file is read only, file is a directory,
 * \a length is greater than the current file size or an I/O error occurs.
 */
bool _SDCardFATFile_truncate(SDCardFAT *fat, SDCardFATFile *f, uint32_t length) {
// error if not a normal file or read-only
  if (!IS_FILE(f) || !(f->flags & O_WRITE)) {
    return false;
  }

  // error if length is greater than current size
  if (length > f->fileSize) {
    return false;
  }

  // fileSize and length are zero - nothing to do
  if (f->fileSize == 0) {
    return true;
  }

  // remember position for seek after truncation
  uint32_t newPos = f->currentPosition > length ? length : f->currentPosition;

  // position to last cluster in truncated file
  if (!SDCardFATFile_seek(fat, f, length)) {
    return false;
  }

  if (length == 0) {
    // free all clusters
    if (!_SDCardFAT_freeChain(fat, f->firstCluster)) {
      return false;
    }
    f->firstCluster = 0;
  } else {
    uint32_t toFree;
    if (!_SDCardFAT_get(fat, f->currentCluster, &toFree)) {
      return false;
    }

    if (!_SDCardFAT_isEOC(fat, toFree)) {
      // free extra clusters
      if (!_SDCardFAT_freeChain(fat, toFree)) {
        return false;
      }

      // current cluster is end of chain
      if (!_SDCardFAT_fatPutEOC(fat, f->currentCluster)) {
        return false;
      }
    }
  }
  f->fileSize = length;

  // need to update directory entry
  f->flags |= F_FILE_DIR_DIRTY;

  if (!_SDCardFATFile_sync(fat, f)) {
    return false;
  }

  // set file to correct position
  return SDCardFATFile_seek(fat, f, newPos);
}

//------------------------------------------------------------------------------
// cache a zero block for blockNumber
bool _SDCardFAT_cacheZeroBlock(SDCardFAT *fat, uint32_t blockNumber) {
  if (!_SDCardFAT_cacheFlush(fat)) {
    return false;
  }

  // loop take less flash than memset(cacheBuffer_.data, 0, 512);
  for (uint16_t i = 0; i < 512; i++) {
    fat->cache.data[i] = 0;
  }
  fat->cacheBlockNumber = blockNumber;
  fat->cacheDirty |= CACHE_FOR_WRITE;
  return true;
}

//------------------------------------------------------------------------------
// Add a cluster to a directory file and zero the cluster.
// return with first block of cluster in the cache
bool _SDCardFATFile_addDirCluster(SDCardFAT *fat, SDCardFATFile *f) {
  if (!_SDCardFATFile_addCluster(fat, f)) {
    return false;
  }

  // zero data in cluster insure first cluster is in cache
  uint32_t block = _SDCardFAT_clusterStartBlock(fat, f->currentCluster);
  for (uint8_t i = fat->blocksPerCluster; i != 0; i--) {
    if (!_SDCardFAT_cacheZeroBlock(fat, block + i - 1)) {
      return false;
    }
  }
  // Increase directory file size by cluster size
  f->fileSize += 512UL << fat->clusterSizeShift;
  return true;
}

bool _SDCardFATFile_openFile(SDCardFAT *fat, SDCardFATFile *f, SDCardFATFile *dirFile, const char *fileName, int mode) {
  char dname[11];
  dir_t *p;

  if (IS_OPEN(f)) {
    return false;
  }

  if (!_SDCardFAT_make83Name(fileName, dname)) {
    return false;
  }

  dirFile->currentCluster = 0;
  dirFile->currentPosition = 0;

  // bool for empty entry found
  bool emptyFound = false;

  // search for file
  while (dirFile->currentPosition < dirFile->fileSize) {
    uint8_t index = 0XF & (dirFile->currentPosition >> 5);
    p = _SDCardFATFile_readDirCache(fat, dirFile);
    if (p == NULL) {
      return false;
    }

    if (p->name[0] == DIR_NAME_FREE || p->name[0] == DIR_NAME_DELETED) {
      // remember first empty slot
      if (!emptyFound) {
        emptyFound = true;
        f->dirIndex = index;
        f->dirBlock = fat->cacheBlockNumber;
      }
      // done if no entries follow
      if (p->name[0] == DIR_NAME_FREE) {
        break;
      }
    } else if (!memcmp(dname, p->name, 11)) {
      // don't open existing file if O_CREAT and O_EXCL
      if ((mode & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
        return false;
      }

      return _SDCardFAT_openCachedEntry(fat, f, 0XF & index, mode);
    }
  }
  // only create file if O_CREAT and O_WRITE
  if ((mode & (O_CREAT | O_WRITE)) != (O_CREAT | O_WRITE)) {
    return false;
  }

  // cache found slot or add cluster if end of file
  if (emptyFound) {
    p = _SDCardFATFile_cacheDirEntry(fat, f, CACHE_FOR_WRITE);
    if (!p) {
      return false;
    }
  } else {
    if (dirFile->type == FAT_FILE_TYPE_ROOT16) {
      return false;
    }

    // add and zero cluster for dirFile - first cluster is in cache for write
    if (!_SDCardFATFile_addDirCluster(fat, dirFile)) {
      return false;
    }

    // use first entry in cluster
    f->dirIndex = 0;
    p = fat->cache.dir;
  }

  // initialize as empty file
  memset(p, 0, sizeof(dir_t));
  memcpy(p->name, dname, 11);

  SDCardFAT_getTime(fat, f, &p->creationDate, &p->creationTime);
  p->lastAccessDate = p->creationDate;
  p->lastWriteDate = p->creationDate;
  p->lastWriteTime = p->creationTime;

  // force write of entry to SD
  if (!_SDCardFAT_cacheFlush(fat)) {
    return false;
  }

  return _SDCardFAT_openCachedEntry(fat, f, f->dirIndex, mode);
}

//------------------------------------------------------------------------------
// Store a FAT entry
bool _SDCardFAT_put(SDCardFAT *fat, uint32_t cluster, uint32_t value) {
  // error if reserved cluster
  if (cluster < 2) {
    return false;
  }

  // error if not in FAT
  if (cluster > (fat->clusterCount + 1)) {
    return false;
  }

  // calculate block address for entry
  uint32_t lba = fat->fatStartBlock;
  lba += fat->fatType == 16 ? cluster >> 8 : cluster >> 7;

  if (lba != fat->cacheBlockNumber) {
    if (!_SDCardFAT_cacheRawBlock(fat, lba, CACHE_FOR_READ)) {
      return false;
    }
  }
  // store entry
  if (fat->fatType == 16) {
    fat->cache.fat16[cluster & 0XFF] = value;
  } else {
    fat->cache.fat32[cluster & 0X7F] = value;
  }
  fat->cacheDirty |= CACHE_FOR_WRITE;

  // mirror second FAT
  if (fat->fatCount > 1) {
    fat->cacheMirrorBlock = lba + fat->blocksPerFat;
  }
  return true;
}

//------------------------------------------------------------------------------
// free a cluster chain
bool _SDCardFAT_freeChain(SDCardFAT *fat, uint32_t cluster) {
  // clear free cluster location
  fat->allocSearchStart = 2;

  do {
    uint32_t next;
    if (!_SDCardFAT_get(fat, cluster, &next)) {
      return false;
    }

    // free cluster
    if (!_SDCardFAT_put(fat, cluster, 0)) {
      return false;
    }

    cluster = next;
  } while (!_SDCardFAT_isEOC(fat, cluster));

  return true;
}

uint32_t SDCardFATFile_size(SDCardFATFile *f) {
  return f->fileSize;
}

uint32_t SDCardFATFile_position(SDCardFATFile *f) {
  return f->currentPosition;
}

uint32_t SDCardFATFile_available(SDCardFATFile *f) {
  uint32_t n = SDCardFATFile_size(f) - SDCardFATFile_position(f);
  return n > 0X7FFF ? 0X7FFF : n;
}

bool SDCardFATFile_open(SDCardFAT *fat, SDCardFATFile *f, const char *filePath, int mode) {
  int pathidx;
  SDCardFATFile parentDir;

  memset(f, 0, sizeof(SDCardFATFile));

  if (!_SDCardFATFile_getParentDir(fat, &parentDir, filePath, &pathidx)) {
    return false;
  }
  filePath += pathidx;

  // it was the directory itself!
  if (!filePath[0]) {
    return true;
  }

  // failed to open a subdir!
  if (IS_CLOSED(&parentDir)) {
    return false;
  }

  // there is a special case for the Root directory since its a static dir
  if (IS_ROOT(&parentDir)) {
    if (!_SDCardFATFile_openFile(fat, f, &fat->root, filePath, mode)) {
      return false;
    }
    // dont close the root!
  } else {
    // TODO: close parent dir
    if (!_SDCardFATFile_openFile(fat, f, &parentDir, filePath, mode)) {
      return false;
    }
  }

  if (mode & O_APPEND) {
    SDCardFATFile_seek(fat, f, f->fileSize);
  }

  return true;
}

void SDCardFATFile_close(SDCardFAT *fat, SDCardFATFile *f) {
  _SDCardFAT_cacheFlush(fat);
  memset(f, 0, sizeof(SDCardFATFile));
}

bool _SDCardFAT_openRoot(SDCardFAT *fat) {
  if (IS_OPEN(&fat->root)) {
    return false;
  }

  if (fat->fatType == 16) {
    fat->root.type = FAT_FILE_TYPE_ROOT16;
    fat->root.firstCluster = 0;
    fat->root.fileSize = 32 * fat->rootDirEntryCount;
  } else if (fat->fatType == 32) {
    fat->root.type = FAT_FILE_TYPE_ROOT32;
    fat->root.firstCluster = fat->rootDirStart;
    if (!_SDCardFAT_chainSize(fat, fat->root.firstCluster, &fat->root.fileSize)) {
      return false;
    }
  } else {
    return false;
  }

  fat->root.flags = O_READ;
  fat->root.currentCluster = 0;
  fat->root.currentPosition = 0;
  fat->root.dirBlock = 0;
  fat->root.dirIndex = 0;
  return true;
}

bool _SDCardFAT_cacheFlush(SDCardFAT *fat) {
  if (fat->cacheDirty) {
    if (!SDCard_writeBlock(&fat->sdcard, fat->cacheBlockNumber, fat->cache.data)) {
      return false;
    }
    // mirror FAT tables
    if (fat->cacheMirrorBlock) {
      if (!SDCard_writeBlock(&fat->sdcard, fat->cacheBlockNumber, fat->cache.data)) {
        return false;
      }
      fat->cacheMirrorBlock = 0;
    }
    fat->cacheDirty = 0;
  }
  return true;
}

bool _SDCardFAT_cacheRead(SDCardFAT *fat, uint32_t blockNumber, int action) {
  if (fat->cacheBlockNumber != blockNumber) {
    if (!_SDCardFAT_cacheFlush(fat)) {
      return false;
    }
    if (!SDCard_readBlock(&fat->sdcard, blockNumber, fat->cache.data)) {
      return false;
    }
    fat->cacheBlockNumber = blockNumber;
  }
  fat->cacheDirty |= action;
  return true;
}

void __attribute__((weak)) SDCardFAT_getTime(SDCardFAT *fat, SDCardFATFile* f, uint16_t* creationDate, uint16_t* creationTime) {
  *creationDate = 0;
  *creationTime = 0;
}




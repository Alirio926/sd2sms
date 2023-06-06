/*******************************************************************//**
 *  \file fat32.c
 *  \author Alirio Oliveira
 *  \brief Arquivo responsavel pelo sistema de arquivo. 
 *
 *  \copyright Esse arquivo é parte do projeto SD2SMS.
 *
 *   SD2SMS is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   SD2SMS is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with SD2SMS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inc\SMSlib.h"
#include "inc\fat32.h"
#include "inc\sdcard_spi.h"
#include "inc\util.h"

#define BYTES_PER_SECTOR 512
struct FileSystem fs;

static u32 readLong(const u8 *ptr) {
  u32 val = ptr[3];
  val <<= 8;
  val |= ptr[2];
  val <<= 8;
  val |= ptr[1];
  val <<= 8;
  val |= ptr[0];
  return val;
}

static u16 readWord(const u8 *ptr) {
	u16 val = ptr[1];
	val <<= 8;
	val |= ptr[0];
	return val;
}

static u8 fatCacheBytes[BYTES_PER_SECTOR];
static u32 fatCacheAddress;

static u32 getNextCluster(const struct FileSystem *fs, u32 cluster) {
	u32 fatAddress;
	u16 fatOffset;
	fatAddress = fs->fatBeginAddress + (cluster * 4)/BYTES_PER_SECTOR;
	if ( fatAddress != fatCacheAddress ) {
		fatCacheAddress = fatAddress;
        SD_readSingleBlock(fatCacheBytes, fatAddress);
	}
	fatOffset = (cluster * 4) % BYTES_PER_SECTOR;
	return readLong(fatCacheBytes + fatOffset);
}

static u8 getFirstPartitionAddress(u32 *address) {
	u8 buffer[BYTES_PER_SECTOR];
    SD_readSingleBlock(buffer, 0);
	if (
		(buffer[446+4] == 0x0C || buffer[446+4] == 0x0B) &&
		buffer[510] == 0x55 && buffer[511] == 0xAA ) 
	{
		*address = readLong(buffer+446+4+4);
		return FAT_SUCCESS;
	} else {
		return FAT_FORMAT;
	}
}

#define fatCheckLongNotZero() if ( readLong(ptr) == 0 ) { retVal = FAT_FORMAT; goto cleanup; } ptr += 4
#define fatCheckWord(expected) if ( readWord(ptr) != expected ) { retVal = FAT_FORMAT; goto cleanup; } ptr += 2
#define fatCheckByte(expected) if ( *ptr++ != expected ) { retVal = FAT_FORMAT; goto cleanup; }
#define NUM_FATS        2
#define FAT_DIRENT_SIZE 32

#define READONLY        (1<<0)
#define HIDDEN          (1<<1)
#define SYSTEM          (1<<2)
#define VOLUMEID        (1<<3)
#define DIRECTORY       (1<<4)
#define ARCHIVE         (1<<5)
#define LFN             (READONLY|HIDDEN|SYSTEM|VOLUMEID)
#define FILEorDIR       (DIRECTORY|ARCHIVE)

u8 fatOpenFileSystem() {
	u32 vidAddress = 0;
	u16 numRsvdSectors;
	u8 retVal = 0;
	u8 buffer[BYTES_PER_SECTOR];
	const u8 *ptr = buffer + 11;

    SMS_mapROMBank(0);
    fs.records = (struct Record_t*)STARTADDR;

	fatCacheAddress = 0;
	retVal = getFirstPartitionAddress(&vidAddress);
	if ( retVal != FAT_SUCCESS ) goto cleanup;
    SD_readSingleBlock(buffer, vidAddress);
	fatCheckWord(BYTES_PER_SECTOR);                 // Word @ offset 11: BPB_BytsPerSec
	fs.numSectorsPerCluster = *ptr++;              // Byte @ offset 13: BPB_SecPerClus
	numRsvdSectors = readWord(ptr); ptr += 2;       // Word @ offset 14: BPB_RsvdSecCnt
	fatCheckByte(NUM_FATS);                         // Byte @ offset 16: BPB_NumFATs
	fatCheckWord(0x0000);                           // Word @ offset 17: BPB_RootEntCount
	fatCheckWord(0x0000);                           // Word @ offset 19: BPB_TotSec16
	ptr++;                                          // Byte @ offset 21: BPB_Media
	fatCheckWord(0x0000);                           // Word @ offset 22: BPB_FATSz16
	ptr += 2+2+4;                                   // 8    @ offset 24: BPB_SecPerTrk, BPB_NumHeads & BPB_HiddSec
	fatCheckLongNotZero();                          // Long @ offset 32: BPB_TotSec
	fs.numSectorsPerFat = readLong(ptr); ptr += 4; // Long @ offset 36: BPB_FATSz32
	ptr += 4;                                       // Long @ offset 40: BPB_ExtFlags & BPB_FSVer
	fs.rootDirCluster = readLong(ptr); ptr += 4;   // Long @ offset 44: BPB_RootClus
	ptr += 510-48;
	fs.fatBeginAddress = vidAddress + numRsvdSectors;
	fatCheckWord(0xAA55);
cleanup:
	return retVal;
}

#define fatGetNumBytesPerCluster() (fs.numSectorsPerCluster * BYTES_PER_SECTOR)
#define fatIsEndOfClusterChain(thisCluster) (thisCluster == 0x0FFFFFFF)
#define fatGetClusterAddress(cluster) (fs.fatBeginAddress + (NUM_FATS * fs.numSectorsPerFat) + (cluster - 2) * fs.numSectorsPerCluster)
#define fatIsDeletedFile(dirPtr) (*dirPtr == 0xE5)
#define fatIsLongFilename(dirPtr) ((dirPtr[11] & LFN) == LFN)
#define fatIsFileOrDir(dirPtr) (dirPtr[11] & FILEorDIR)
#define fatIsVolumeName(dirPtr) (dirPtr[11] & VOLUMEID)
#define fatIsSystemFolder(dirPtr) (dirPtr[11] & 0x16)
#define fatGetFileSize(dirPtr) readLong(dirPtr+28)
#define fatGetFirstCluster(dirPtr) ((readWord(dirPtr+20)<<16) + readWord(dirPtr+26))

void name(const u8 *ptr, signed char* fileName, u16* fnLen){
      u8 offset, last;
      offset = *ptr;
      last = offset & 0x40; // chegou ao fim?
      offset &= 0x1F;
      offset--;
      offset *= 13;   
      fileName[offset+0] = ptr[1];
      fileName[offset+1] = ptr[3];
      fileName[offset+2] = ptr[5];
      fileName[offset+3] = ptr[7];
      fileName[offset+4] = ptr[9];
      fileName[offset+5] = ptr[14];
      fileName[offset+6] = ptr[16];
      fileName[offset+7] = ptr[18];
      fileName[offset+8] = ptr[20];
      fileName[offset+9] = ptr[22];
      fileName[offset+10] = ptr[24];
      fileName[offset+11] = ptr[28];
      fileName[offset+12] = ptr[30];

      if ( last ) {
          fileName[offset+13] = '\0';
          fileName[offset+14] = '\0';
          offset += 12;
          while ( fileName[offset] == 0 || fileName[offset] == -1 ) {
              fileName[offset] = 0;
              offset--;
          }
          offset++;
          *fnLen = offset;
      }
}

u8 fatListDirectory(u32 dirCluster, u8 isRoot){
    const u16 entriesPerCluster = fatGetNumBytesPerCluster()/FAT_DIRENT_SIZE;
	const u16 bytesPerCluster = fatGetNumBytesPerCluster();
	char fileName[262];
    u8 buffer[512];
    u16 ptrPos=0;
    const u8 *ptr = &buffer[0];
	u16 i = 0;
	u8 retVal = 0;
	u16 fnLen = 0, fileCount=0, sector_cnt = 0;
    u8 numberOfPages = 0;

    SMS_mapROMBank(0);
    
    fs.records[numberOfPages].dirCluster   = dirCluster;
    fs.records[numberOfPages].dirEntryPos  = 0;
    fs.records[numberOfPages].ptrBufferPos = 0;
    fs.records[numberOfPages].numSector    = 0;
    
    SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster));
    
    for(;;){
        if(*ptr == 0x00)
            break;
        else if(fatIsLongFilename(ptr))
            name(ptr, fileName, &fnLen);
        else if(!fatIsDeletedFile(ptr) && !fatIsVolumeName(ptr)){
            if(isRoot > 2){
                isRoot--;
            }else{
                if(ptr[0xb] != 0x16){ // pasta do windows, System Volum...
                    fileCount++; // incrementa contador de file  
                    if(fileCount == NUM_FILES_PER_PAGE){
                        fileCount = 0;
                        numberOfPages++;
                        fs.records[numberOfPages].dirCluster   = dirCluster;
                        fs.records[numberOfPages].dirEntryPos  = i+1;
                        fs.records[numberOfPages].ptrBufferPos = ptrPos + FAT_DIRENT_SIZE;
                        if(ptrPos + FAT_DIRENT_SIZE == 512)
                            fs.records[numberOfPages].numSector    = sector_cnt+1;
                        else
                            fs.records[numberOfPages].numSector    = sector_cnt;
                    }
                }
            }
			fnLen = 0;
        }

        ptr     += FAT_DIRENT_SIZE; // incrementa ponteito
        ptrPos  += FAT_DIRENT_SIZE; // incremente contador
        // incrementa contador de dir entry
        i++;
        if( i == entriesPerCluster ){
            dirCluster = getNextCluster(&fs, dirCluster);
            // for esse for o ultimo cluster
            if(fatIsEndOfClusterChain(dirCluster)){
                break;
            }
            SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster));
			i           = 0;
            sector_cnt  = 0; // Incrementador de setor
            ptrPos      = 0; // posição do ponteiro no buffer
			ptr         = &buffer[0];        
        }

        if(ptrPos == BYTES_PER_SECTOR){
            sector_cnt++;
            SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster) + sector_cnt);
            ptrPos = 0;
			ptr = &buffer[0];
        }
    }
    fs.numberOfPages = numberOfPages+1;
    return retVal;
}

u8 loadPage(const u16 page, u8 isRoot, FileNameCallback callback){
    const u16 entriesPerCluster = fatGetNumBytesPerCluster()/FAT_DIRENT_SIZE;
	const u16 bytesPerCluster = fatGetNumBytesPerCluster();
	char fileName[262];
    u8 buffer[512];
    const u8 *ptr = &buffer[0];
    u16 ptrPos=0, i;
	u8 retVal = 0, flag;
	u32 firstCluster, dirCluster;
	u16 fnLen = 0, fileCount=0, sector_cnt;
    u8 numberOfPages = 0, x=2, y=3;

    SMS_mapROMBank(0);

    dirCluster  = fs.records[page].dirCluster;
    i           = fs.records[page].dirEntryPos;
    sector_cnt  = fs.records[page].numSector;
    ptrPos      += fs.records[page].ptrBufferPos;
    ptr         += fs.records[page].ptrBufferPos;
    //SD_readMultBlock(buffer, fatGetClusterAddress(dirCluster), fs->numSectorsPerCluster);
    SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster) + sector_cnt);

    for(;;){
        if(*ptr == 0x00)
            break;
        else if(fatIsLongFilename(ptr))
            name(ptr, fileName, &fnLen);
        else if(!fatIsDeletedFile(ptr) && !fatIsVolumeName(ptr) ){
            if(ptr[0xb] != 0x16){                
                // skip . e ..
                if(isRoot > 0){
                    isRoot--;
                }else{
                    //flag = ptr[0xb] & (0x20 | 0x10);
                    firstCluster = fatGetFirstCluster(ptr);
                    callback(fileName, firstCluster, fatGetFileSize(ptr), fatIsFileOrDir(ptr));
                    fileCount++; // incrementa contador de file  
                    if(fileCount == NUM_FILES_PER_PAGE){
                        break;
                    }
                }
            }
			fnLen = 0;
        }

        ptr += FAT_DIRENT_SIZE; // incrementa ponteito
        ptrPos += FAT_DIRENT_SIZE; // incremente contador
        // incrementa contador de dir entry
        i++;
        if( i == entriesPerCluster ){
            dirCluster = getNextCluster(&fs, dirCluster);
            // for esse for o ultimo cluster
            if(fatIsEndOfClusterChain(dirCluster)){
                break;
            }
            SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster));
			i = 0;
            ptrPos = 0;
            sector_cnt = 0;
			ptr = &buffer[0];           
        }

        if(ptrPos == BYTES_PER_SECTOR){
            sector_cnt++;
            SD_readSingleBlock(buffer, fatGetClusterAddress(dirCluster) + sector_cnt);
            ptrPos = 0;
			ptr = &buffer[0];
        }
    }
    return retVal;
}

u32 fatGetClusterLength() {
	u32 len = fs.numSectorsPerCluster;
	len *= BYTES_PER_SECTOR;
	return len;
}

//(fs.fatBeginAddress + (NUM_FATS * fs.numSectorsPerFat) + (cluster - 2) * fs.numSectorsPerCluster)
u32 preCalc;
void preCalcs(){
    preCalc = fs.fatBeginAddress + (NUM_FATS * fs.numSectorsPerFat);
}

#define CALC(cluster) (preCalc + (cluster - 2) * fs.numSectorsPerCluster)

u32 fatReadCluster(u16 address, u32 cluster) {
    u16 sector_cnt =0;
    u32 c = CALC(cluster);
    u8 *ptr = (u8*)address, retVal;
    //do{
    //    SD_readSingleBlockk(ptr, (c + sector_cnt));
    //    ptr += 512;
    //    sector_cnt++;        
    //}while(sector_cnt < fs.numSectorsPerCluster);
    SD_readMultBlock(ptr, c, fs.numSectorsPerCluster);
    //if(retVal > 0){
    //    draw_text("Sd Timeout", 5, 5);
    //    while(1)SMS_waitForVBlank();
    //}
	return getNextCluster(&fs, cluster);
}
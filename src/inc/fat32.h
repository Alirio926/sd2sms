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

#ifndef _FAT32_H
#define  _FAT32_H

#include "types.h"

#define STARTADDR   ((u8 *)0x8000)
#define NUM_FILES_PER_PAGE 19 // max 18 por tela
#define FILENAME_SIZE 28

typedef struct NodeFile_t{
    u8 name[FILENAME_SIZE];
    u8 flags;
    u32 cluster;
    u32 size;
} NodeFile_t;

typedef struct NodePage_t{
    struct NodeFile_t nFile[NUM_FILES_PER_PAGE];
    u8 pageSize;
} NodePage_t;

typedef struct Record_t{
	u16 ptrBufferPos;
	u16 dirEntryPos;
	u32 dirCluster;
	u16 numSector;
} Record_t;

struct FileSystem {
	u8 numSectorsPerCluster;
	u32 numSectorsPerFat;
	u32 rootDirCluster;
	u32 fatBeginAddress;
    struct Record_t *records;
    u8 numberOfPages;
};

typedef void (*FileNameCallback)(char *, u32, u32, u8);
enum {
	FAT_SUCCESS,
	FAT_FORMAT
};

u8 fatOpenFileSystem();
u8 fatListDirectory(u32 dirCluster, u8 isRoot);
u32 fatGetClusterLength();
u32 fatReadCluster(u16 address, u32 cluster);

void preCalcs();
u8 loadPage(const u16 page, u8 isRoot, FileNameCallback callback);

#endif  /* _FAT32_H */
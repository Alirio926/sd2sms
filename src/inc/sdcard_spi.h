/*******************************************************************//**
 *  \file sdcard_spi.c
 *  \author Alirio Oliveira
 *  \brief Arquivo que faz a comunicação com o sdcard atraves da cpld. 
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

#ifndef _SDCARD_SPI_H
#define _SDCARD_SPI_H

#include "types.h"

/***********************************************************************************
                               SD CARD INIT STATUS
 ***********************************************************************************/
#define SDCARD_INIT_SUCCESSFUL 0
#define SDCARD_NOT_DETECTED    1
#define SDCARD_INIT_FAILED     2
#define SDCARD_FAT_INVALID     3

#define SDCARD_TYPE_UNKNOWN        0
#define SDCARD_TYPE_STANDARD       1
#define SDCARD_TYPE_HIGH_CAPACITY  2
/***********************************************************************************/

//SD commands, many of these are not used here
#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_IF_COND       		 8
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define SD_SEND_OP_COND      	 (0x80 + 41)   //ACMD
#define APP_CMD          		 55
#define READ_OCR        		 58
#define CRC_ON_OFF               59

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


//////--DELETE DEPOIS-////////
/* Command code for disk_ioctrl() */
/* Generic command */
#define CTRL_SYNC			0	/* Mandatory for write functions */
#define GET_SECTOR_COUNT	1	/* Mandatory for only f_mkfs() */
#define GET_SECTOR_SIZE		2
#define GET_BLOCK_SIZE		3	/* Mandatory for only f_mkfs() */
#define CTRL_POWER			4
#define CTRL_LOCK			5
#define CTRL_EJECT			6
/* MMC/SDC command */
#define MMC_GET_TYPE		10
#define MMC_GET_CSD			11
#define MMC_GET_CID			12
#define MMC_GET_OCR			13
#define MMC_GET_SDSTAT		14
//////--DELETE DEPOIS-////////


/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

u8 init_SdCard(u8 *cardType) ;
u8 SD_sendCommand(u8 cmd, u32 arg) ;
u8 SD_readSingleBlock(u8 *buffer,u32 startBlock) ;
u8 SD_readSingleBlockk(u8 *inputbuffer, u32 startBlock);
u8 SD_readMultBlock(u8 *buff, u32 sector, u16 count) ;
u8 SD_writeSingleBlock(u8 *inputbuffer, u32 startBlock) ;
void sdReadBlockToROM(u16 address, u32 sector, u8 count) ;

#endif
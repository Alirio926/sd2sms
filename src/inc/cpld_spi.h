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

#include "types.h"

#define SPI_DATA				*((u8 *)0x4000)
#define SPI_CFG					*((u8 *)0x4004)

#define SPI_SD_ENABLE_BIT		0
#define KOREN_MAP_ENABLE_BIT	1
#define SD_TURBO_ENABLE_BIT		3
#define SPI_BUSY_BIT			7

#define SPI_WAIT_READY  		while((SPI_CFG & 0x80));

#define SPI_SD_ENABLE		cart_cfg &= ~(1 << SPI_SD_ENABLE_BIT); \
							SPI_CFG = cart_cfg;

#define SPI_SD_DISABLE		cart_cfg |= 1 << SPI_SD_ENABLE_BIT; \
							SPI_CFG = cart_cfg;

#define KOREN_MAP_DISABLE	cart_cfg &= ~(1 << KOREN_MAP_ENABLE_BIT); \
							SPI_CFG = cart_cfg;

#define KOREN_MAP_ENABLE	cart_cfg |= 1 << KOREN_MAP_ENABLE_BIT; \
							SPI_CFG = cart_cfg;

#define SD_TURBO_DISABLE	cart_cfg &= ~(1 << SD_TURBO_ENABLE_BIT); \
							SPI_CFG = cart_cfg;

#define SD_TURBO_ENABLE	    cart_cfg |= 1 << SD_TURBO_ENABLE_BIT; \
							SPI_CFG = cart_cfg;

extern u8 cart_cfg;

void spi_send(u8 *buff, u16 bc);
void spi_recv(u8 *buff, u16 bc);
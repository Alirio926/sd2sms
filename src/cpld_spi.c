/*******************************************************************//**
 *  \file cpld_spi.c
 *  \author Alirio Oliveira
 *  \brief Arquivo responsavel pela comunicação spi com o cpld . 
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

#include "inc\cpld_spi.h"

void spi_send(u8 *buff, u16 bc){
	u8 d;
	do{
		d = *buff++;
		SPI_WAIT_READY
		SPI_DATA = d;
	}while(--bc);
}

void spi_recv(u8 *buff, u16 bc){
	do{
		SPI_WAIT_READY
		*buff++ = SPI_DATA;		
	}while(--bc);
}



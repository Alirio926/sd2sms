/*******************************************************************//**
 *  \file util.h
 *  \author Alirio Oliveira
 *  \brief Arquivo com helps . 
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


void draw_num(u8 *coment, u32 num, u8 x, u8 y);
void draw_text(unsigned char* text, unsigned char x, unsigned char y);
void myMemCpy(void *dest, void *src, u16 n);
int strcpy(char *dest, char *orig);
int strncmp( const char * s1, const char * s2, u8 n );
const BYTE strstr(const BYTE *xxx);
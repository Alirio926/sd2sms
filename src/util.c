/*******************************************************************//**
 *  \file util.c
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

#include "inc\SMSlib.h"
#include "inc\types.h"
#include "inc\util.h"

extern unsigned char devkitSMS_font__tiles__1bpp;
extern signed int SMS_TextRenderer_offset;

u8 str_buff_1[30];
u8 str_buff_2[10];

u8 STR_intToDecString(u32 val, u8 *str) {

    u8 len = 0;
    u8 ret_len;
    u8 *buff = str_buff_2;


    str += len;
    *buff++ = 0;

    while (val) {

        *buff++ = '0' + val % 10;
        val /= 10;
        len++;
    }
    if (len == 0) {
        *buff++ = '0';
        len++;
    }
    ret_len = len - 1;
    len++;
    while (len--) {
        *str++ = *--buff;
    }

    return ret_len;
}

void draw_num(u8 *coment, u32 num, u8 x, u8 y) {

    u8 comment_len = 0;
    while (coment[comment_len] != 0) {
        str_buff_1[comment_len] = coment[comment_len];
        comment_len++;
    }
    STR_intToDecString(num, &str_buff_1[comment_len]);
    draw_text(str_buff_1, x, y);
}

void draw_text(unsigned char* text, unsigned char x, unsigned char y)
{
    //const unsigned int* pnt = devkitSMS_font__tiles__1bpp; 
    unsigned char idx = 0;
    while ('\0' != text[idx])
    {
        signed char tile = text[idx];// -TEXT_ROOT;
        SMS_setNextTileatXY(x++, y);
        SMS_setTile(SMS_TextRenderer_offset + tile);
        idx++;
    }
}

void myMemCpy(void *dest, void *src, u16 n)
{
    u16 i;
   // Typecast src and dest addresses to (char *)
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
  
   // Copy contents of src[] to dest[]
   for (i=0; i<n; i++)
       cdest[i] = csrc[i];
}

int strcpy(char *dest, char *orig){
    int i = 0;

    while(*(orig + i) != '\0'){
        *(dest + i) = *(orig + i);
        i++;
    }

    *(dest + i) = '\0';

    return i;
}

int strncmp( const char * s1, const char * s2, u8 n )
{
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

const BYTE strstr(const BYTE *xxx){
	BYTE r = 3, c=0;
	while(xxx[r+3] != '\0' && r < 29){
		if(((xxx[r]) == '.') ||
		 ((xxx[r+1]) == 's') ||
		 ((xxx[r+2]) == 'm') ||
		 ((xxx[r+3]) == 's')){

			return r;
		}
		r++;
	}
    while(c < 30){
        if(xxx[c] == 0x00 || xxx[c] == 0xFF)
            return c-1;
        
        c++;
    }

	return 0;
}
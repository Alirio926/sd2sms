/*******************************************************************//**
 *  \file graphics.c
 *  \author Alirio Oliveira
 *  \brief Arquivo que carrega os graficos do menu. 
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
#include "inc\gfx.h"
#include "inc\types.h"
#include "inc\graphics.h"

extern unsigned char devkitSMS_font__tiles__1bpp[];
extern signed int SMS_TextRenderer_offset;

unsigned int splitline=90;
unsigned char BWpalette[16];
unsigned char toBW[10]={0x00,0x00, 0x15,0x15,0x15, 0x2A,0x2A,0x2A, 0x3F,0x3F};

void lineHandler (void) {
  SMS_disableLineInterrupt();
  SMS_loadBGPalette(BWpalette);  
}

void prepareBWpalette (unsigned char *palette) {
  unsigned char i;
  for (i=0;i<16;i++) {
    BWpalette[i]=toBW[((*palette & 0x03)+((*palette>>2) & 0x03)+((*palette>>4) & 0x03))];
    palette++;
  }
}

void logoScreen(){
    u16 t = 60;
    u8 x=60;
    SMS_VRAMmemset (0x0000, 0x00, 16384);
    load_LogoTiles();
    /* prepare B/W palette from image original palette */
    prepareBWpalette(logopalette_bin);
    SMS_loadBGPalette(BWpalette);  
    SMS_setBGScrollY(154);
    SMS_setLineInterruptHandler(lineHandler);
    SMS_displayOn();
    
    while(t--)
        SMS_waitForVBlank();

    t=60;
    while(t--){
        SMS_waitForVBlank();
        SMS_enableLineInterrupt();
        SMS_setLineCounter(x++);
        SMS_loadBGPalette(logopalette_bin);
    }
    SMS_disableLineInterrupt();

    t=60;
    while(t--)
        SMS_waitForVBlank();
}

void load_MenuTiles()
{
	SMS_loadBGPalette(backgroundpalette_bin);
  SMS_loadPSGaidencompressedTiles(backgroundtiles_psgcompr, 0);
  SMS_loadTileMap(0, 0, backgroundtilemap_bin, backgroundtilemap_bin_size);

}

void load_LogoTiles()
{
	SMS_loadBGPalette(logopalette_bin);
  SMS_loadPSGaidencompressedTiles(logotiles_psgcompr, 0);
  SMS_loadTileMap(0, 0, logotilemap_bin, logotilemap_bin_size);

}

void load_LoadingTiles()
{
	// Splash tiles.
	SMS_loadPSGaidencompressedTiles(Loading_Tiles_psgcompr, 112);
	SMS_loadSTMcompressedTileMap(0, 0, Loading_Tilemap_stmcompr);
	SMS_loadBGPalette(Loading_Palette_bin);
}
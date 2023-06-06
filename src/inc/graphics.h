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

#ifndef _GRAPHICS
#define _GRAPHICS

void lineHandler (void) ;
void prepareBWpalette (unsigned char *palette);
void logoScreen();
void load_MenuTiles();
void load_LogoTiles();
void load_LoadingTiles();

#endif
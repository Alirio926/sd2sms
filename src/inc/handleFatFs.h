/*******************************************************************//**
 *  \file handleFatFs.c
 *  \author Alirio Oliveira
 *  \brief Arquivo que carrega arquivos na estrutura que será carregada na tela. 
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

#ifndef _HANDLEFATFS
#define _HANDLEFATFS

#include "types.h"
#include "fat.h"

void changeLine(u8 pos, u8 updown);
void loadPage(u8 page);
void printFiles(u8 pageIndex, u8 pos);
void showListMenu(u8 pageIndex, u8 pos);

#endif
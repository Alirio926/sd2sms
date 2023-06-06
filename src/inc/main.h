/*******************************************************************//**
 *  \file main.c
 *  \author Alirio Oliveira
 *  \brief Arquivo principal do menu. 
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

void addFileOnPage(u8 *name, u32 cluster, u32 fileSize, u8 flag);
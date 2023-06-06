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

#include "inc\SMSlib.h"
#include "inc\handleFatFs.h"
#include "inc\util.h"
#include "inc\fat.h"

#define MAX_SCREEN_FILENAME 26
#define MAX_SCREEN_FILES 19

void loadPage(u8 page){
    DIR directory;
    FILINFO fno;
    FRESULT res;
    u8 fileCnt = page;
    u8 pageCnt = 0;
    u8 *ptr;
    u16 idx;
    u8 x=2,y=3;

    root = (ROOT*) STARTADDR; 
    
    do{
        SMS_mapROMBank(0); 
        directory = root->dir[fileCnt++];

        res = f_readdir(&directory, &fno);

        if ((res != FR_OK) || !fno.fname[0]) break; 
        if(fno.fattrib != AM_ARC && fno.fattrib != AM_DIR)
            continue;         

        idx = pageCnt * _MAX_LFN;
        SMS_mapROMBank(1); 
        ptr = FILENAME(idx);
        for(u16 j=0; j < _MAX_LFN; j++){
            ptr[j] = fno.fname[j];
        }
        //myMemCpy(ptr, fno.fname, _MAX_LFN);
        pageCnt++;
    }while((res == FR_OK) && (pageCnt < MAX_SCREEN_FILES));

    SMS_mapROMBank(1); 
    pSize = pageCnt;    
}

/*
void changeLine(u8 pos, u8 updown){
    char tmp[30];
    u8 x=2, a, b, *ptr, data, j;
    u16 idx;
    
    SMS_mapROMBank(1); 

    idx = pos * _MAX_LFN;
    ptr = FILENAME(idx);
    
    do{
        data = ptr[j];
        tmp[j++] = data;
        if(data == 0x00 || j == MAX_SCREEN_FILENAME){
            tmp[j] = '\0';
        }
    }while(data != 0x00 && j < MAX_SCREEN_FILENAME);
    
    SMS_configureTextRenderer(64-32); 
    
    draw_text(tmp, x, pos+3);

    // up
    if(updown == 0){  
        a = pos-1;   
        b = pos+2; 
    }// down
    else{
        a = pos+1;
        b = pos+4;
    }
    idx = a * _MAX_LFN;
    ptr = FILENAME(idx);
    do{
        data = ptr[j];
        tmp[j++] = data;
        if(data == 0x00 || j == MAX_SCREEN_FILENAME){
            tmp[j] = '\0';
        }
    }while(data != 0x00 && j < MAX_SCREEN_FILENAME);
    SMS_configureTextRenderer(160-32);
    
    draw_text(tmp, x, b);
}

void showListMenu(u8 pageIndex, u8 pos){    
    u8 x=2, y=3, cnt=0;
    u8 menu_items=MAX_SCREEN_FILES;
    u8 pages_num=0;
    u8 page_size;
    u8 page = 0;
    u8 idx_offset;
    u8 seletor;
    u8 i;
    u8 idx;
    
    // Garante que esta no bank correto
    SMS_mapROMBank(0); 

    pages_num = dir->size / menu_items;
    if (pages_num == 0)pages_num = 1;
    if (pages_num * menu_items < dir->size)pages_num++;
    idx_offset = page * menu_items;

    if((dir->size - idx_offset) >= menu_items){
        page_size = menu_items;
    }else{
        page_size = dir->size - idx_offset;
    }

    if(seletor >= page_size){
        seletor = page_size - 1;
    }

    for(i = 0; i < page_size; i++){
        if(idx_offset > dir->size) break;
        idx = dir->order[idx_offset++];

        if(seletor == i){
            SMS_configureTextRenderer(160-32); 
        }else if((dir->records[idx].flags & FAT_FIL)){
           SMS_configureTextRenderer(64-32); 
        }else{
            SMS_configureTextRenderer(64-32); 
        }
        draw_text(dir->records[idx].name, x, y++);
    }

        //draw_text("                           ", x, y++);
}
*/
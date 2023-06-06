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

#include "inc\SMSlib.h"
#include <stdlib.h>

#include "inc\util.h"
#include "inc\fat32.h"
#include "inc\types.h"
#include "inc\cpld_spi.h"
#include "inc\sdcard_spi.h"
#include "inc\sdcard_spi.h"
#include "inc\graphics.h"
#include "inc\font.h"

#define BANK_SIZE  0x4000
#define BANK_START 0x8000

/* Define número maximo de arquivos por paginas. */
#define MAX_SCREEN_FILES 19

/* Motedo implementando no arquivo asm reset_a.s */
void soft_reset();
/* Motodo que recebe um struct file e carrega o jogo do sd para a sram */
void burnGame(NodeFile_t *file);
/* Esse motodo trava o console em caso de erro */
void freeze();
/* Estrutura que armazena a pagina exibita atual*/
static NodePage_t mainPage;
/* Guarda a configuração dos registradores no CPLD */
/* SD TURBO, inMenu, KOREN_MAP, sd ena/dis*/
u8 cart_cfg; 
/* Tipo de cartão sd*/
static u8 cardType;
/* Usado para controle de diretórios*/
Record_t dir_tree[9];
u8 seletor_tree[9];
u8 page_tree[9];
extern u32 fat_cluster_size;
extern unsigned char devkitSMS_font__tiles__1bpp[];
extern signed int SMS_TextRenderer_offset;
extern struct FileSystem fs;
void freeze(){
    while(1)
        SMS_waitForVBlank();
}

/* Metodo que inicia a fonta atual */
void initFont(){
    SMS_load1bppTiles(font_sms, 64,  1024, 0, 14);
    SMS_load1bppTiles(font_sms,160,  1024, 0, 15);
    SMS_setBGPaletteColor (14, RGB(5,5,5)); // Foreground  gray
    SMS_setBGPaletteColor (15, RGB(3,3,3)); // Foreground  white
}

/* Simples metodo para pular a intro da logo case seja pressionado para cima durante a inicialização */
void skipLogo(u8 newJoy){
    u8 y = 136;
    if(newJoy & PORT_A_KEY_UP){
        SMS_displayOff();
        load_MenuTiles();    
        SMS_displayOn();
    }else{
        // Load logo
        logoScreen();
        // Clean vram
        SMS_VRAMmemset (0x0000, 0x00, 16384);

        // seta posição do scroll
        SMS_setBGScrollY(y);
        SMS_displayOff();
        load_MenuTiles();    
        SMS_displayOn();
        
        // Scroll screen
        for( ; y < 225; y++){
            SMS_setBGScrollY(y);
            SMS_waitForVBlank();
        }
    }
}

/* Motodo que adiciona um arquivo a pagina que será exibida */
void addFileOnPage(u8 *name, u32 cluster, u32 fileSize, u8 flag){
  u8 fnCount = 0;
  const u8 *ptr = name;
  u8 pSize = mainPage.pageSize;
  NodeFile_t* nFile_t = &mainPage.nFile[pSize];

  nFile_t->cluster = cluster;
  nFile_t->flags   = flag;
  nFile_t->size    = fileSize;

  do{
    nFile_t->name[fnCount] = *ptr;    
    fnCount++;   
    ptr++;

    if(*ptr == 0x00 || fnCount == FILENAME_SIZE)
      nFile_t->name[fnCount] = '\0';

  }while(*ptr != '\0' && *ptr != 0x00 && fnCount < FILENAME_SIZE);

  mainPage.pageSize++;
}
/* Metodo principal */
void main (void)
{        
    
    static u8 newJoy, ret;
    // Local onde o logo vai iniciar a animação
    u8 j=0;
    // Paginação
    u8 posX=2, posY=3, cnt=0;
    u8 menu_items=MAX_SCREEN_FILES;
    u8 dir_tree_top = 0;
    //u8 pages_num=0;
    u8 page = 0;
    u8 seletor=0;
    u8 i;  
    u8 numberOfPages, retVal;
    u16 key_delay = 0;
    // fim variaveis da paginação

    cart_cfg = 5; // 0101, TURBO OFF, inMenu, KOREN_MAP disable, sd disable
    
    /* Limpa screem*/
    SMS_VRAMmemset (0x0000, 0x00, 16384);
        
    newJoy = SMS_getKeysStatus();
    /* Usado para skip animação inicial da logo */
    skipLogo(newJoy);
    /* Load fonte na VRAM */
    initFont();

    SMS_configureTextRenderer(64-32);  
    draw_text("Lendo SDCard, espere...", 2, 3);    

    SMS_waitForVBlank(); // <------

    /* Tenta iniciar o SDCard*/
    retVal = init_SdCard(&cardType);
    if(retVal != SDCARD_INIT_SUCCESSFUL){
        draw_num("Sdcard falhou: ", retVal, 2, 10);
        freeze();
    }
    /* Enable SPI 25Mhz */
    SD_TURBO_ENABLE

    /* Inicializa o sistema e arquivo FAT32 */
    fatOpenFileSystem();
    if(retVal != FAT_SUCCESS){
        draw_num("Fat32 não encontrado: ", retVal, 2, 10);
        freeze();
    }

    // Ler root dir.
    fatListDirectory(fs.rootDirCluster, 0);

    mainPage.pageSize   = 0;      
    // Apos carregar fatListDirectory com rootDirCluster, 
    // record[0] corresponde ao root dir, aos primeiros 19 files do rootdir  
    
    /* Fato interessante, o SDCC buga quando tento
     * Passar o mesmo struck como dois argumentos
     * por isso instanciei um novo obj
     */
    loadPage(0, 0, addFileOnPage);   

    numberOfPages = fs.numberOfPages;
    dir_tree[dir_tree_top] = fs.records[0];
    seletor_tree[dir_tree_top] = 0;
    page_tree[dir_tree_top] = 0;

    for (;;)
    {        
        /* Garante que esta no bank correto. bank 0 é utilizado para armazenar a estrutura de pastes e arquivos */
        SMS_mapROMBank(0);
        
        //////////////////////////////////////////////////////////////
        newJoy = SMS_getKeysStatus();              

        if(seletor >= mainPage.pageSize){
            seletor = mainPage.pageSize - 1;
        }

        posY = 3;
        i = 0;
        for(; i < mainPage.pageSize; i++){
            if(seletor == i){
                SMS_configureTextRenderer(160-32); 

            //}else if((mainPage.nFile[i].flags & 0x10)){
            //    SMS_configureTextRenderer(160-32); 

            }else{
                SMS_configureTextRenderer(64-32); 
            }
            
            draw_text(mainPage.nFile[i].name, posX, posY);
                    
            j = 0;
            while(mainPage.nFile[i].name[j] != '\0') j++;

            while(j < FILENAME_SIZE){
                draw_text(" ", j+posX, posY);
                j++;
            }

            //SMS_configureTextRenderer(64-32); 
            //while(j < 32) draw_text(" ", posX+j++, posY); 

            posY++;       
        }
        while(posY < menu_items+3) 
            draw_text("                                ", posX, posY++);

        SMS_configureTextRenderer(160-32); 
        draw_text("     ", 9, 23);
        draw_text("              ", 15, 23);
        draw_num("Pagina: ", page, 2, 23); 
        draw_num("Flags: ", mainPage.nFile[seletor].flags, 15, 23); 

        SMS_waitForVBlank();
        
        for(;;){
            newJoy = SMS_getKeysStatus();
            if(newJoy == 0 || key_delay > 11) break;
            SMS_waitForVBlank();
            key_delay++;
        }

        for(;;){
            SMS_waitForVBlank();
            newJoy = SMS_getKeysStatus();
            if(newJoy == 0) break;
            key_delay++;
            if((key_delay % 3) != 0)continue;
            break;
        }

        for(;;){
            newJoy = SMS_getKeysStatus();
            if(newJoy != 0)break;
            key_delay=0;
        }

        if(newJoy & PORT_A_KEY_UP){
            seletor = seletor == 0 ? mainPage.pageSize - 1 : seletor - 1;
            continue;
        }

        if(newJoy & PORT_A_KEY_DOWN){
            seletor = seletor ==  mainPage.pageSize - 1 ? 0 : seletor + 1;
        }
        
        if((newJoy & PORT_A_KEY_RIGHT) && numberOfPages > 1){
            page++;
            if(page == numberOfPages) page = 0;
            mainPage.pageSize = 0;  
            if(dir_tree_top == 0) // se estiver no root, n pula nada        
                loadPage(page, 0, addFileOnPage); 
            else // se estiver em um dir, skip . e ..
                loadPage(page, 2, addFileOnPage); 

            SMS_waitForVBlank();
            key_delay=0;
            continue;
        }

        if((newJoy & PORT_A_KEY_LEFT) && numberOfPages > 1){
            page = page == 0 ? numberOfPages - 1 : page - 1;
            mainPage.pageSize = 0;
            if(dir_tree_top == 0) // se estiver no root, n pula nada        
                loadPage(page, 0, addFileOnPage); 
            else // se estiver em um dir, skip . e ..
                loadPage(page, 2, addFileOnPage); 

            SMS_waitForVBlank();
            key_delay=0;
            continue;
        }

        if(newJoy & PORT_A_KEY_1 && dir_tree_top > 0){
            dir_tree_top--;
            // leio posição atual na pagina
            seletor = seletor_tree[dir_tree_top];
            // leio a pagina
            page = page_tree[dir_tree_top];
            // Carrego lista de pagina com iniciando pela primeira pagina               
            // carrego a mesma pagina de quando foi selecionado para entrar no diretorio          
            if(dir_tree_top == 0){ // se estiver no root, n pula nada        
                fatListDirectory(dir_tree[dir_tree_top].dirCluster, 0);
                mainPage.pageSize = 0;   
                loadPage(page, 0, addFileOnPage); 
            }else{ // se estiver em um dir, skip . e ..
                fatListDirectory(dir_tree[dir_tree_top].dirCluster, 0);
                mainPage.pageSize = 0;   
                loadPage(page, 2, addFileOnPage); 
            }
            numberOfPages = fs.numberOfPages;
            key_delay=0;
            continue;
        }

        if(newJoy & PORT_A_KEY_2){
            if(mainPage.pageSize == 0) continue;
            SMS_waitForVBlank();

            if(mainPage.nFile[seletor].flags != 0x10){ // FAT_FIL
                burnGame(&mainPage.nFile[seletor]);
            }else{   
                /* Ao armazenar uma pagina, salvo o cluster da primeira
                 * pagina, dessa forma quando retornar posso carregar 
                 * todas as paginas do diretorio.
                */             
                // salvo localização da primeira pagina do diretorio
                dir_tree[dir_tree_top] = fs.records[0];
                // salvo a pagina atual, assim posso retonar na mesma pagina
                page_tree[dir_tree_top] = page;
                // e retono na mesma possição da lista na pagina
                seletor_tree[dir_tree_top++] = seletor;                

                // Ler o diretorio e cria paginas com limite de arquivos
                // por pagina
                fatListDirectory(mainPage.nFile[seletor].cluster, 2);
                mainPage.pageSize = 0;                
                loadPage(0, 2, addFileOnPage);  
                numberOfPages = fs.numberOfPages;

                page = 0;
                seletor = 0;
                key_delay = 0;
                continue;
            }
        }
    }
}

void burnGame(NodeFile_t *file){
    u32 cluster             = file->cluster;
    u16 cluster_len         = fatGetClusterLength();
    u32 fSize               = file->size;    
    u16 progress            = 0; 
    u16 oldProgress         = 0;
    u16 currentBankAddress  = 0x8000; 
    u8 numero_de_clusters   = (u8) (fSize / cluster_len);
    u8 cluster_count        = 0;
    u8 current_bank         = 0;
    const u8 numberOfBanks  = fSize / 16384;
    u8 x=2, lastPos = 2;
    const u8 num_cluster = numero_de_clusters;
    u8 newJoy;
    SMS_VRAMmemset (0x0000, 0x00, 16384);

    load_LoadingTiles();
    SMS_autoSetUpTextRenderer();
    SMS_setBGPaletteColor(1, RGB(1,1,0));
    
    SMS_setNextTileatXY(1, 1);

    draw_num("fSize: ", fSize, 1, 2);
    draw_num("cSize: ", cluster_len, 1, 3);
    
    SMS_mapROMBank(current_bank);
    
    /* refere-se ao inicio do sistema de arquivo */
    preCalcs();

    while(numero_de_clusters){
        // Grava um cluster de 4096 no banco atual
        cluster = fatReadCluster(currentBankAddress, cluster);
        // decrementa numero de clusters pendentes
        --numero_de_clusters;

        // incrementa index da memoria no banco atual
        currentBankAddress += cluster_len;
        
        // com cluster de 4k, a cada 4 cluster, muda de banco(16k)
        if(currentBankAddress == (BANK_START + BANK_SIZE)){
            current_bank++;
            progress = (current_bank * 28) / numberOfBanks;
            for( ;lastPos < progress+2; lastPos++)
                draw_text("*", x++, 17);                
            SMS_mapROMBank(current_bank); // proximo banco
            currentBankAddress = BANK_START;            
            SMS_waitForVBlank();
        }        
    }
    
    newJoy = SMS_getKeysStatus();    
    /* serve para habilitar o Korean map, ou foi o inicio da implementação do som FM, não lembro, porem
     * o som FM não esta desenhado na pcb ainda */
    if(newJoy & PORT_A_KEY_UP){
        *((u8*)0xDFFF) = 0x03;
    }else{
        *((u8*)0xDFFF) = 0x01;
    }

    SMS_VRAMmemset (0x3800, 0x00, 0x5ff);
    /* Lindo soft reset */
    soft_reset();
}
/*
u8 burnGame(u32 c, u32 fSize){
    // progress
    const u8 numberOfBanks  = fSize / 16384;
    u16 progress            = 0; 
    u8 x=2, lastPos = 2;
    // end progress
    //u8 tmr[16];
    u8 resp=0;
    u8 current_bank = 0;
    u32 length = fSize;
    u16 cluster = c, setor_start_address;
    u8 num_of_clusters = (u8)(length / fat_cluster_size);
    u16 mapped_addres_cnt = 0;
    u16 current_cluster_address=0;
    u16 current_slot_address=0;
    //u16 mod;
    u8 setor_cnt=0;
    u8 *ptr;
    
    //SMS_VRAMmemset (0x0000, 0x00, 16384);

    load_LoadingTiles();


    if(length > 524288){
        draw_text("Arquivo muito grande.", 2, 3);
        SMS_waitForVBlank();
        return 1;
    }

    SMS_waitForVBlank();

    // remove header
    if((length & 1023) == 512){
        length -= 512;
        setor_cnt += 1;
    }
   
    setor_start_address = clusterToSector(cluster);
    SMS_mapROMBank(current_bank);
    while(num_of_clusters){
        do{
            ptr = SETOR_TO_SLOT2(current_cluster_address % 0x4000); // 16kB count

            resp = SD_readSingleBlock(ptr, (setor_start_address + setor_cnt)); // start cluster + current setor(512)
            if(resp != 0){
                draw_num("Erro 0", resp, 2, 11);
                freeze();
            }
            current_cluster_address += 512;
            setor_cnt += 1;
            if((current_cluster_address % 0x4000) == 0){
                current_bank++;
                SMS_mapROMBank(current_bank);

                progress = (current_bank * 28) / numberOfBanks;
                for( ;lastPos < progress+2; lastPos++)
                    draw_text("*", x++, 17);

                SMS_waitForVBlank();
            }
        }while(current_cluster_address < fat_cluster_size);
        num_of_clusters -= 1;
        current_cluster_address = 0;
        setor_cnt = 0;
        cluster = getNextCluster(cluster);
        setor_start_address = clusterToSector(cluster);        
    }

    //KOREN_MAP_ENABLE;

    draw_num("Finish: ", current_bank, 2, 10);
    
    soft_reset();

    return 0;
}
*/
SMS_EMBED_SEGA_ROM_HEADER_16KB(9999, 0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE_16KB(1, 0, "Squallsoft Studios", "SMS Everdrive", "Squallsoft first everdrive!");
//SMS_EMBED_SEGA_ROM_HEADER(9999, 0);
//SMS_EMBED_SDSC_HEADER(1, 0, 2022, 8, 9, "Squallsoft Studios", "SMS Everdrive", "Squallsoft first everdive!");
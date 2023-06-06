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

#include "inc\SMSlib.h"
#include "inc\sdcard_spi.h"
#include "inc\cpld_spi.h"
#include "inc\util.h"
#include "inc\fat32.h"

u8 V_SdHighcapacityFlag_u8 = 0;

void SPI_Write(u8 value) {
  SPI_WAIT_READY
  SPI_DATA = value;
}

u8 SPI_Read() {
    SPI_WAIT_READY
  return SPI_DATA;
}

void SPI_EnableChipSelect() {
  SPI_SD_ENABLE
}

void SPI_DisableChipSelect() {
  SPI_SD_DISABLE
}

void SPI_Write_Buffer(u8 *buff, u16 bc) {
	u8 d;
	do{
		d = *buff++;
		SPI_WAIT_READY
		SPI_DATA = d;
	}while(--bc);
}

void SPI_Read_Buffer(u8 *buff, u16 bc) {
	do{
		SPI_WAIT_READY
		*buff++ = SPI_DATA;		
	}while(--bc);
}

u8 init_SdCard(u8 *cardType)  {
  u8  i, response, sd_version;
  u16 retry = 0 ;

  SD_TURBO_DISABLE;
  
  for (i = 0; i < 10; i++)
    SPI_Write(0xff);   //80 clock pulses spent before sending the first command

  SPI_EnableChipSelect();
  do
  {
    response = SD_sendCommand(GO_IDLE_STATE, 0); //send 'reset & go idle' command
    retry++;
    if (retry > 0x20)
      return SDCARD_NOT_DETECTED;   //time out, card not detected

  } while (response != 0x01);

  SPI_DisableChipSelect();
  SPI_Write (0xff);
  SPI_Write (0xff);

  retry = 0;

  sd_version = 2; //default set to SD compliance with ver2.x;
  //this may change after checking the next command
  do
  {
    response = SD_sendCommand(SEND_IF_COND, 0x000001AA); //Check power supply status, mendatory for SDHC card
    retry++;
    if (retry > 0xfe)
    {
      sd_version = 1;
      *cardType = SDCARD_TYPE_STANDARD;
      break;
    } //time out

  } while (response != 0x01);

  retry = 0;

  do
  {
    //response = SD_sendCommand(APP_CMD, 0); //CMD55, must be sent before sending any ACMD command
    response = SD_sendCommand(SD_SEND_OP_COND, 0x40000000); //ACMD41

    retry++;
    if (retry > 0xFFFE)
    {
      return SDCARD_INIT_FAILED;  //time out, card initialization failed
    }
  } while (response != 0x00);

  retry = 0;
  V_SdHighcapacityFlag_u8 = 0;

  if (sd_version == 2)
  {
    do
    {
      response = SD_sendCommand(READ_OCR, 0);
      retry++;
      if (retry > 0xFFFE)
      {
        *cardType = SDCARD_TYPE_UNKNOWN;
        break;
      } //time out

    } while (response != 0x00);

    if (V_SdHighcapacityFlag_u8 == 1)
    {
      *cardType = SDCARD_TYPE_HIGH_CAPACITY;
    }
    else
    {
      *cardType = SDCARD_TYPE_STANDARD;
    }
  }
  
  return response;
}

u8 SD_sendCommand(u8 cmd, u32 arg) {
  u8 response, retry = 0, status, n;

  if (V_SdHighcapacityFlag_u8 == 0)
  {
    if (cmd == READ_SINGLE_BLOCK     ||
        cmd == READ_MULTIPLE_BLOCKS  /*||
        cmd == WRITE_SINGLE_BLOCK    ||
        cmd == WRITE_MULTIPLE_BLOCKS ||
        cmd == ERASE_BLOCK_START_ADDR ||
        cmd == ERASE_BLOCK_END_ADDR */)
    {
      arg = arg << 9;
    }
  }

  if(cmd & 0x80){
    cmd &= 0x7F;
    n = SD_sendCommand(APP_CMD, 0);
    if(n > 1) return n; // tratar erro aqui, como n é maior que 1, retorna sem enviar ACMD41
  }

  SPI_EnableChipSelect();

  SPI_Write(cmd | 0x40); //send command, first two bits always '01'
  SPI_Write(arg >> 24);
  SPI_Write(arg >> 16);
  SPI_Write(arg >> 8);
  SPI_Write(arg);

  if (cmd == SEND_IF_COND) //it is compulsory to send correct CRC for CMD8 (CRC=0x87) & CMD0 (CRC=0x95)
    SPI_Write(0x87);    //for remaining commands, CRC is ignored in SPI mode
  else
    SPI_Write(0x95);

  retry = 0xff;
  do{
    response = SPI_Read();
  }while((response == 0xFF) && --retry);
  //while ((response = SPI_Read()) == 0xff) //wait response
  //  if (retry++ > 0xfe) break; //time out error

  if (response == 0x00 && cmd == 58) //checking response of CMD58
  {
    status = SPI_Read() & 0x40;     //first byte of the OCR register (bit 31:24)
    if (status == 0x40)
    {
      V_SdHighcapacityFlag_u8 = 1;  //we need it to verify SDHC card
    }
    else
    {
      V_SdHighcapacityFlag_u8 = 0;
    }

    SPI_Read(); //remaining 3 bytes of the OCR register are ignored here
    SPI_Read(); //one can use these bytes to check power supply limits of SD
    SPI_Read();
  }

  SPI_Read(); //extra 8 CLK
  SPI_DisableChipSelect();

  return response; //return state
}

u8 SD_sendFastCommand(u8 cmd, u32 arg) {
  u8 response, retry = 0, status, n;

  if (V_SdHighcapacityFlag_u8 == 0)
  {
    if (cmd == READ_SINGLE_BLOCK     ||
        cmd == READ_MULTIPLE_BLOCKS  /*||
        cmd == WRITE_SINGLE_BLOCK    ||
        cmd == WRITE_MULTIPLE_BLOCKS ||
        cmd == ERASE_BLOCK_START_ADDR ||
        cmd == ERASE_BLOCK_END_ADDR */)
    {
      arg = arg << 9;
    }
  }

  if(cmd & 0x80){
    cmd &= 0x7F;
    n = SD_sendFastCommand(APP_CMD, 0);
    if(n > 1) return n; // tratar erro aqui, como n é maior que 1, retorna sem enviar ACMD41
  }

  SPI_SD_ENABLE

  SPI_DATA = (cmd | 0x40); //send command, first two bits always '01'
  SPI_DATA = (arg >> 24);
  SPI_DATA = (arg >> 16);
  SPI_DATA = (arg >> 8);
  SPI_DATA = (arg);

  if (cmd == SEND_IF_COND) //it is compulsory to send correct CRC for CMD8 (CRC=0x87) & CMD0 (CRC=0x95)
    SPI_DATA = 0x87;    //for remaining commands, CRC is ignored in SPI mode
  else
    SPI_DATA = 0x95;

  retry = 0xff;
  do{
    response = SPI_DATA;
  }while((response == 0xFF) && --retry);
  //while ((response = SPI_Read()) == 0xff) //wait response
  //  if (retry++ > 0xfe) break; //time out error

  if (response == 0x00 && cmd == 58) //checking response of CMD58
  {
    status = SPI_DATA & 0x40;     //first byte of the OCR register (bit 31:24)
    if (status == 0x40)
    {
      V_SdHighcapacityFlag_u8 = 1;  //we need it to verify SDHC card
    }
    else
    {
      V_SdHighcapacityFlag_u8 = 0;
    }

    SPI_DATA; //remaining 3 bytes of the OCR register are ignored here
    SPI_DATA; //one can use these bytes to check power supply limits of SD
    SPI_DATA;
  }

  SPI_DATA; //extra 8 CLK
  SPI_SD_DISABLE;

  return response; //return state
}

u8 SD_readSingleBlock(u8 *inputbuffer, u32 startBlock) {
  u8 response;
  u16 retry, bc = 0x200;

  response = SD_sendCommand(READ_SINGLE_BLOCK, startBlock); //read a Block command

  if (response != 0x00)
  {
    return response; //check for SD status: 0x00 - OK (No flags set)
  }

  SPI_EnableChipSelect();

  retry = 0;
  while (SPI_Read() != 0xfe) //wait for start block token 0xfe (0x11111110)
  {
    if (retry++ > 0xfffe)
    {
      SPI_DisableChipSelect();
      return 1; //return if time-out
    }
  }

  while(bc--){
    SPI_WAIT_READY
		*inputbuffer++ = SPI_DATA;		
  }
  //SPI_Read_Buffer(inputbuffer, 512);

  SPI_Read(); //receive incoming CRC (16-bit), CRC is ignored here
  SPI_Read();

  SPI_Read(); //extra 8 clock pulses
  SPI_DisableChipSelect();

  return 0;
}

u8 SD_readSingleBlockk(u8 *inputbuffer, u32 startBlock) {
  u8 response;
  u16 retry, bc = 0x200;

  response = SD_sendFastCommand(READ_SINGLE_BLOCK, startBlock); //read a Block command

  if (response != 0x00)
  {
    return response; //check for SD status: 0x00 - OK (No flags set)
  }

  SPI_SD_ENABLE

  retry = 0;
  while (SPI_DATA != 0xfe) //wait for start block token 0xfe (0x11111110)
  {
    if (retry++ > 0xfffe)
    {
      SPI_SD_DISABLE
      return 1; //return if time-out
    }
  }

  while(bc--){
		*inputbuffer++ = SPI_DATA;		
  }
  //SPI_Read_Buffer(inputbuffer, 512);

  SPI_DATA; //receive incoming CRC (16-bit), CRC is ignored here
  SPI_DATA;

  SPI_DATA; //extra 8 clock pulses
  SPI_SD_DISABLE

  return 0;
}

u8 waitToken(){
  u16 retry=0xFFFF;
  do{
    if(SPI_DATA == 0xFE)
      return 0;
  }while(retry--);
  return 1;
}

u8 SD_readMultBlock(u8 *buff, u32 sector, u16 count) {
  u8 response;
  u16 retry = 0xFFFF, bc = 0x200;
  
  response = SD_sendFastCommand(READ_MULTIPLE_BLOCKS, sector); //read a Block command

  if (response != 0x00)
    return response; //check for SD status: 0x00 - OK (No flags set)
  
  SPI_SD_ENABLE;
  
  do{
    while(SPI_DATA != 0xFE){
      if(--retry) continue;

      return 8;
    }

    while(bc--){
      *buff++ = SPI_DATA;		
    }

    SPI_DATA; // CRC 8-15
    SPI_DATA; // CRC 0-7
    bc = 0x200;
  }while(count--);

  SD_sendFastCommand(STOP_TRANSMISSION, 0);

  SPI_DATA; //extra 8 clock pulses
  SPI_SD_DISABLE;
  
  return 0;
}



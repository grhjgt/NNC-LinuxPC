/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_MTP.H         
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description:  
*Function List: 
	1  uint8_t MTP_init(void);
	2  void MTP_TRIM(STRUCT_MTP_TRIM TRIM);
	3  void flash_read(uint32_t start_addr,uint16_t *test_i);
	4  int8_t flash_buff_write(uint32_t start_addr, uint16_t *buff);
	5  int8_t flash_write_ctrl(uint16_t *buff , uint32_t start_addr);
	6  int8_t write_data(void);
History: 
1.V1.0
Date:          
Author: 
Modification: ³õ°æ
*/


#ifndef ENS1_MTP_H
#define ENS1_MTP_H
#include "CMSDK_CM0.h"
#include <stdlib.h>
#include <string.h>
#include "my_header.h"
#define  	MTP_BASE_ADDR      CMSDK_MTP_BASE                //0x10000000 
#define  	MTP_SIZE       	   0x8000                        //32KBytes
#define 	HW32_REG(ADDRESS)  (*((volatile unsigned int  *)(ADDRESS)))
#define     HW16_REG(ADDRESS)  (*((volatile uint16_t *)(ADDRESS)))
#define     DATA_SAVE_ADDR     (MTP_BASE_ADDR + 0x6F00)      //µÚ0x1BC0¿é

typedef struct STRUCT_MTP_TRIM
{
	__I	uint32_t DEV_ID1:32;
    __I uint32_t DEV_ID2:32;
    __I uint32_t DEV_ID3:32;
	__IO uint8_t OSCA_FT:5;
	__IO uint8_t OSC32K_RTRIM:5;
	__IO uint8_t BG_TRIM:8;
}STRUCT_MTP_TRIM;





extern uint8_t MTP_init(void);
extern void MTP_TRIM(STRUCT_MTP_TRIM TRIM);
extern void flash_read(uint32_t start_addr,uint16_t *test_i);
extern int8_t flash_buff_write(uint32_t start_addr, uint16_t *buff);
extern int8_t flash_write_ctrl(uint16_t *buff , uint32_t start_addr);
extern int8_t write_data(void);







#endif

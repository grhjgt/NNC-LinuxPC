/*
* Copyright (C), 2023, NANOCHAP
* File name:   ENS1_MTP.H          
* Description: MTP Flash ����ͷ�ļ� (��ģʽ�л��밲ȫ���̽ӿ�)
*/

#ifndef ENS1_MTP_H
#define ENS1_MTP_H

#include "CMSDK_CM0.h"
#include "ENS001_CONFIG.h"
#include <stdlib.h>
#include <string.h>
#include "my_header.h"

#define MTP_BASE_ADDR      CMSDK_MTP_BASE                // 0x10000000 
#define HW32_REG(ADDRESS)  (*((volatile unsigned int  *)(ADDRESS)))
#define HW16_REG(ADDRESS)  (*((volatile uint16_t *)(ADDRESS)))
#define DATA_SAVE_ADDR     (MTP_BASE_ADDR + 0x6F00)      // Sector 6 (0x1BC0 ��)

typedef struct STRUCT_MTP_TRIM
{
	__I	uint32_t DEV_ID1:32;
    __I uint32_t DEV_ID2:32;
    __I uint32_t DEV_ID3:32;
	__IO uint8_t OSCA_FT:5;
	__IO uint8_t OSC32K_RTRIM:5;
	__IO uint8_t BG_TRIM:8;
} STRUCT_MTP_TRIM;

extern uint8_t MTP_init(void);
extern void MTP_enter_read_mode(void);
extern void MTP_TRIM(STRUCT_MTP_TRIM TRIM);
extern void flash_read(uint32_t start_addr, uint16_t *test_i);
extern int8_t flash_buff_write(uint32_t start_addr, uint16_t *buff);
extern int8_t flash_write_ctrl(uint16_t *buff, uint32_t start_addr);
extern int8_t flash_erase_sector6(void);
extern int8_t flash_safe_write_4x16(uint16_t *ft_array);

#endif

/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_MTP.C         
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description:  
*Function List: 

History: 
1.V1.0
Date:          
Author: 
Modification: 初版
*/


/*
MTP说明
1、MTP部分只能按块写入,每块有1024字节大小
块的编号：
SECTOR 0 : 0000H - 03FFH
SECTOR 1 : 0400H - 07FFH
SECTOR 2 : 0800H - 0BFFH
SECTOR 3 : 0C00H - 0FFFH
SECTOR 4 : 1000H - 13FFH
SECTOR 5 : 1400H - 17FFH
SECTOR 6 : 1800H - 1BFFH
SECTOR 7 : 1C00H - 1FFFH 
*/

#include "ENS1_MTP.h"

uint16_t write_current_data[4]={0,0,0,0};
STRUCT_MTP_TRIM MTP_FT_SET=
{
	.OSCA_FT = 0x10,      //默认值为 0X10
	.OSC32K_RTRIM = 0x10, //默认值为 0X10
	.BG_TRIM = 0x88,

};

uint8_t MTP_init(void)
{
	#ifdef ENS1_HSI_16MHz
		CMSDK_MTPREG->MTP_CR = 0x00000001;
	#elif ENS1_HSI_32MHz
		CMSDK_MTPREG->MTP_CR = 0x00000003;
	#endif

	return  0;
}


//MTP内保存的电流检测数据读取
void flash_read(uint32_t start_addr,uint16_t *test_i){
	uint16_t  result = 0;
	result = HW16_REG(start_addr);
	*test_i    = result ;
}
			
//向MTP中写入数据（仅限于向用户自定义块写入！！！）
//可自定义读写的区域为MTP的第0x1BC0块（MTP_BASE_ADDR + 0x6F00）
int8_t flash_buff_write(uint32_t start_addr, uint16_t *buff) {
	HW32_REG(start_addr) =  ((uint32_t)((*(buff+1)<<16)&0xffff0000) + ((*buff)&0x0000ffff));	
//	while(!(CMSDK_MTPREG->MTP_SR&0x00000002)){};			
	if(HW32_REG(start_addr) != ((uint32_t)(*(buff+1)<<16) + *buff ))
	{
		printf("write error\n");
		return -1;			
	}
	return 0;
}

int8_t flash_write_ctrl(uint16_t *buff , uint32_t start_addr){
	CMSDK_MTPREG->MTP_CLR = 0xffffffff;   //SR寄存器
	CMSDK_MTPREG->MTP_CR = 0x00000002;    //2等待周期
	CMSDK_MTPREG->MTP_ACLR = 0x00000000;  //允许软件读写从sector0 -sector 6 （sector7 作为bootloader区）
	CMSDK_MTPREG->MTP_KEYR = 0x5a5a5a5a;  //key
	return flash_buff_write(start_addr, buff);
}

int8_t write_data(void){
	uint16_t *wr_data = (uint16_t *)malloc(8*sizeof(uint8_t));
	memcpy(wr_data , write_current_data, 8);
	int8_t val = flash_write_ctrl(wr_data,DATA_SAVE_ADDR);  //第 0x1BC0块
	val = flash_write_ctrl(wr_data+2, DATA_SAVE_ADDR + 4);
	return val;
}	


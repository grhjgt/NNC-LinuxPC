/*
* Copyright (C), 2023, NANOCHAP
* File name:   ENS1_MTP.c
* Description: MTP 控制驱动 (仅保留读模式切换与初始化, 校准参数现由 HK32 下发)
*/

#include "ENS1_MTP.h"

// 读模式强制切换
void MTP_enter_read_mode(void)
{
	#ifdef ENS1_HSI_16MHz
		CMSDK_MTPREG->MTP_CR = 0x00000001;
	#elif ENS1_HSI_32MHz
		CMSDK_MTPREG->MTP_CR = 0x00000003;
	#else
		CMSDK_MTPREG->MTP_CR = 0x00000003;
	#endif
}

// 初始化 MTP
uint8_t MTP_init(void)
{
	MTP_enter_read_mode();
	return 0;
}

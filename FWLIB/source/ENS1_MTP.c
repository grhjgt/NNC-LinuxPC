/*
* Copyright (C), 2023, NANOCHAP
* File name:   ENS1_MTP.c         
* Description: MTP 控制驱动 (支持模式切换、全擦除、加等待编程与读回校验)
*/

#include "ENS1_MTP.h"
#include <stdio.h>

STRUCT_MTP_TRIM MTP_FT_SET =
{
	.OSCA_FT = 0x10,      // 默认值为 0x10
	.OSC32K_RTRIM = 0x10, // 默认值为 0x10
	.BG_TRIM = 0x88,
};

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

// MTP 内保存的数据读取
void flash_read(uint32_t start_addr, uint16_t *test_i)
{
	MTP_enter_read_mode();
	*test_i = HW16_REG(start_addr);
}

// 向 MTP 中写入 32 位数据 (含 MTP_SR 编程等待与超时)
int8_t flash_buff_write(uint32_t start_addr, uint16_t *buff)
{
	uint32_t expect = ((uint32_t)((*(buff+1) << 16) & 0xffff0000) | ((*buff) & 0x0000ffff));
	HW32_REG(start_addr) = expect;
	
	// 轮询等待编程完成 (MTP_SR bit 1 = 1)
	uint32_t timeout = 3200000; // 【BUG修复】: 延长超时时间
	while (!(CMSDK_MTPREG->MTP_SR & 0x00000002)) {
		if (--timeout == 0) {
			MTP_enter_read_mode();
			return -1; // 编程超时失败
		}
	}
	
	// 编程完成后切回读模式再读取比对校验
	MTP_enter_read_mode();
	if (HW32_REG(start_addr) != expect) {
		return -1; // 读回比对校验不一致
	}
	return 0;
}

// 控制写包装
int8_t flash_write_ctrl(uint16_t *buff, uint32_t start_addr)
{
	CMSDK_MTPREG->MTP_CLR = 0xffffffff;   // 清 SR 寄存器
	CMSDK_MTPREG->MTP_CR = 0x00000002;    // 编程模式
	CMSDK_MTPREG->MTP_ACLR = 0x00000000;  // 允许软件读写 sector 0 - sector 6
	CMSDK_MTPREG->MTP_KEYR = 0x5a5a5a5a;  // 解锁 Key
	return flash_buff_write(start_addr, buff);
}

// 擦除 Sector 6 扇区 (全写 0xFFFF)
int8_t flash_erase_sector6(void)
{
	CMSDK_MTPREG->MTP_CLR = 0xffffffff;
	CMSDK_MTPREG->MTP_CR = 0x00000002;
	CMSDK_MTPREG->MTP_ACLR = 0x00000000;
	CMSDK_MTPREG->MTP_KEYR = 0x5a5a5a5a;
	HW32_REG(DATA_SAVE_ADDR) = 0xFFFFFFFF;
	
	uint32_t timeout = 3200000; // 【BUG修复】: 延长超时时间
	while (!(CMSDK_MTPREG->MTP_SR & 0x00000002)) {
		if (--timeout == 0) {
			MTP_enter_read_mode();
			return -1;
		}
	}
	MTP_enter_read_mode();
	return 0;
}


/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_CLOCK.C          
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


#include "ENS1_CLOCK.h"

uint32_t APB_Clock_Freq =0;
//设置后的时钟频率请查看时钟树计算
Clock_ConfigStructure CLOCKCFG=
{
	.MCO_SEL = MCO_HSI ,
	.HSI_FREQ = HSI_32MHZ ,
	.HSE_OSC_FREQ = 0,
	.LSE_OSC_FREQ = 0,
	.SYSCLK_SEL =  HSI_SYSCLK ,
	.ENS1_APB_PCLK_DIV_x = ENS1_APB_PCLK_DIV_1,
	.ENS1_AHB_PCLK_DIV_x = ENS1_AHB_HCLK_DIV_1,
	.LFCLK_SW_SEL = LSI_AS_LFCLK ,
};

uint32_t ClockInitSet(Clock_ConfigStructure* CLOCKCONFIG)
{
	uint32_t clockfreq = 0;
	//配置系统各时钟初始化
	//1 确定输入频率
	CMSDK_SYSCON->CLK_CFG |= (CLOCKCONFIG->MCO_SEL<<16);
	if(CLOCKCONFIG->MCO_SEL == MCO_HSI)
	{
		CMSDK_SYSCON->HSI_CTRL |= (CLOCKCONFIG->HSI_FREQ << 4);
		clockfreq = (uint8_t)pow(2,CLOCKCONFIG->HSI_FREQ+2)*1000000;
	}
	else if(CLOCKCONFIG->MCO_SEL == MCO_HSE)
	{
		clockfreq = CLOCKCONFIG->HSE_OSC_FREQ * 1000000;
	}
	else if(CLOCKCONFIG->MCO_SEL == MCO_LSI)
	{
		clockfreq = 32768 ;
	}
	else if(CLOCKCONFIG->MCO_SEL == MCO_LSE)
	{
		clockfreq = CLOCKCONFIG->LSE_OSC_FREQ ;
	}
	else
	{
		CMSDK_SYSCON->HSI_CTRL |= (CLOCKCONFIG->HSI_FREQ << 4);
		clockfreq = (uint8_t)pow(2,CLOCKCONFIG->HSI_FREQ+2);
	}		
	//2 选择系统的时钟源
	CMSDK_SYSCON->CLK_CFG |= (CLOCKCONFIG->SYSCLK_SEL);
	while((CMSDK_SYSCON->CLK_CFG >> 2 ) & 0x1);
	//3 基于系统时钟源的频率设置分频系数
	CMSDK_SYSCON->CLK_CFG |= (CLOCKCONFIG->ENS1_AHB_PCLK_DIV_x << 8);
	CMSDK_SYSCON->CLK_CFG |= (CLOCKCONFIG->ENS1_APB_PCLK_DIV_x << 12);
	if(CLOCKCONFIG->ENS1_AHB_PCLK_DIV_x > 0)
		clockfreq = (uint32_t)(clockfreq / pow(2,CLOCKCONFIG->ENS1_AHB_PCLK_DIV_x-3));
	if(CLOCKCONFIG->ENS1_APB_PCLK_DIV_x > 0)
		clockfreq = (uint32_t)(clockfreq /  pow(2,CLOCKCONFIG->ENS1_APB_PCLK_DIV_x-3));
	return   clockfreq; //返回时钟频率（分频后的APB上的时钟频率）
}

void ClockInit(void)
{
	APB_Clock_Freq = ClockInitSet(&CLOCKCFG);
	CMSDK_SYSCON->APB_CLKEN = 0;
}

//PCLK时钟使能
uint8_t PCLK_Enable(uint8_t APB_CLKEN_POS)
{
	CMSDK_SYSCON->APB_CLKEN |= (0x1 << APB_CLKEN_POS);
	return 0;
}

//PCLK时钟关闭
uint8_t PCLK_Disable(uint8_t APB_CLKEN_POS)
{
	CMSDK_SYSCON->APB_CLKEN &=~ (0x1 << APB_CLKEN_POS);
	return 0;
}
//配置外部晶振，配置前请保留足够延时，否则无法二次烧录程序
void HSE_ClockInit(uint32_t Clock_Freq)
{
	CMSDK_GPIO->IE = (CMSDK_GPIO->IE & ~(0x01ul << 0)) | (0x01 << 0);
	CMSDK_GPIO->ALTFL = (CMSDK_GPIO->ALTFL & ~(0x03ul << 0)) | (0x02 << 0);
	CMSDK_SYSCON->CLK_CFG  = (CMSDK_SYSCON->CLK_CFG & ~CMSDK_SYSCON_SYSCLK_SEL_Msk) | (0x1 << CMSDK_SYSCON_SYSCLK_SEL_Pos);
	while (((CMSDK_SYSCON->CLK_CFG & CMSDK_SYSCON_SYSCLK_SWSTS_Msk) >> CMSDK_SYSCON_SYSCLK_SWSTS_Pos) != 0x01) { }        
	CMSDK_SYSCON->HSI_CTRL = (CMSDK_SYSCON->HSI_CTRL & ~CMSDK_SYSCON_HSI_EN_Msk);	
	APB_Clock_Freq = Clock_Freq;//外部时钟频率
}

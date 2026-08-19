/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_WATCHDOG.c         
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

#include "ENS1_WATCHDOG.h"


//设置load值
void WatchDogLoad(uint32_t loadvalue_us)
{
	CMSDK_WATCHDOG->LOAD  = loadvalue_us * (uint8_t)(APB_Clock_Freq/1000000) ;
}
//看门狗喂狗
void WatchDogRefresh(void)
{
	WatchDog_LOCK_RegSet(UNLOCK);
	CMSDK_WATCHDOG->INTCLR = 1 ;
	WatchDog_LOCK_RegSet(LOCK);//
}

//读取watchdog 递减计数器当前值
uint32_t Read_WDOGVALUE(void)
{
	return ( CMSDK_WATCHDOG->VALUE & 0xffffffff );
}

//使能看门够中断事件
void WatchDog_Control(FunctionalState newstate)
{
	if(newstate == ENABLE)
	{
		CMSDK_WATCHDOG->CTRL |= (0x3);
	}
	else
	{
		CMSDK_WATCHDOG->CTRL &=~ (0x3);
	}
}

//清除看门狗中断事件,清除后将自动重载计数值
void WatchDog_IntClear(void)
{
	CMSDK_WATCHDOG->INTCLR = 1 ;
}

//解除看门狗锁寄存器或给看门狗寄存器上锁
uint8_t WatchDog_LOCK_RegSet(WdogLockState newstate)
{
	if(newstate == LOCK)
	{
		CMSDK_WATCHDOG->LOCK = 0;
	}
	else 
	{
		CMSDK_WATCHDOG->LOCK  = 0x1ACCE551;
	}
	return (CMSDK_WATCHDOG->LOCK &0x1);
}

//看门狗初始化设置
void WatchDog_Init(uint32_t loadvalue_us)
{
	PCLK_Enable(WDT_PCLK_EN);
	WatchDog_LOCK_RegSet(UNLOCK);
	WatchDogLoad(loadvalue_us);
	WatchDog_Control(ENABLE);
	WatchDog_LOCK_RegSet(LOCK);
}

void NMI_Handler(void)
{
//	WatchDog_IntClear();
}

/*
*Copyright ,2024 , NANOCHAP
*File name:   ENS1_EXTI.C         
*Author:      
*Version:     V1.0 
*Date:        2024-11-                  
*Description:  
*Function List: 
    
History: 
	1.V1.0
	Date:          
	Author: 
	Modification: 初版
*/ 


#include "ENS1_EXTI.h"
#include "ENS1_GPIO.h"



/*每一位在相应的线路上使能/失能事件或中断的上升沿触发  0-29*/ 
void EXTI_RisingTriggerSelect(EXTI_NUM NUM , FunctionalState newstate)
{	
	if(newstate == ENABLE)
		CMSDK_EXTI->RTSR |= (1<<NUM);
	else 
		CMSDK_EXTI->RTSR &=~ (1<<NUM);
}	

/*每一位在相应的线路上使能/失能事件或中断的下降沿触发  0-29*/
void EXTI_FallingTriggerSelect(EXTI_NUM NUM , FunctionalState newstate)
{
	if(newstate == ENABLE)
		CMSDK_EXTI->FTSR |= (1<<NUM);
	else 
		CMSDK_EXTI->FTSR &=~ (1<<NUM);
}

/*软件中断设置：通过软件设置某一位的上升沿发生从而导致上升沿中断（软件中断）发生，仅仅可写 0-29*/
void EXTI_SoftwareInterruptEventSet(EXTI_NUM NUM)
{
	CMSDK_EXTI->SWIER |= (1<<NUM);
}

/*读取上升沿触发请求数据（用于判断上升沿事件发生）*/
bool  Read_EXTI_RisingEdgePending(EXTI_NUM NUM)
{
	return (bool)((CMSDK_EXTI->RISE_PEND_STS >> NUM ) & 0x1);
}
//清除上升沿触发状态
void Clear_EXTI_RisingEdgePending(EXTI_NUM NUM)
{
	CMSDK_EXTI->RISE_PEND_CLR |= (1<<NUM);
}

/*读取下降沿触发请求数据（用于判断下降沿事件发生）*/
bool  Read_EXTI_FallingEdgePending(EXTI_NUM NUM)
{
	return (bool)((CMSDK_EXTI->FALL_PEND_STS >> NUM) & 0x1);
}
//清除下降沿触发状态
void Clear_EXTI_FallingEdgePending(EXTI_NUM NUM)
{
	CMSDK_EXTI->FALL_PEND_CLR |= (1<<NUM);
}

/*清除/屏蔽 唤醒CPU的中断线（0：用被屏蔽的中断唤醒 1：用未被屏蔽的中断唤醒）0-31*/ 
void  EXTI_INT_MASK(uint8_t NUM)
{	
	if(NUM >31)
		return ;
	CMSDK_EXTI->IMR |= (1<<NUM);
}

/*设置/清除 事件生成唤醒CPU  0-31 */
void EXTI_EVENT_MASK(uint8_t NUM)
{	
	if(NUM >31)
		return ;
	CMSDK_EXTI->EMR |= (1<<NUM);
}
//GPIO_NUM GPIO_X：配置GPIO
//EXTI_NUM NUM：配置GPIO外部中断
//EXTI_TRIGGER Trigger_level：配置外部中断出发边沿
//IRQn_Type IRQn：配置对应中断号
//FunctionalState ENABLEorNOT：外部中断使能
void EXTI_GPIO_Init(GPIO_NUM GPIO_X, EXTI_NUM NUM,EXTI_TRIGGER Trigger_level , IRQn_Type IRQn,FunctionalState ENABLEorNOT)
{
	//配置GPIO为输出模式
	GPIO_IO_Init(GPIO_X,INPUT,GPIO_OType_PP,GPIO_PD,OUTPUT_FAST,PDRV_4mA,ENABLEorNOT);
	//失能中断
	NVIC_DisableIRQ(IRQn);
	//清除中断标志
	NVIC_ClearPendingIRQ(IRQn);
	if(ENABLEorNOT == ENABLE)
	{
		if(Trigger_level == EXTI_FALL || Trigger_level == EXTI_BOTH)
		{
			//配置下降沿中断
			EXTI_FallingTriggerSelect(NUM,ENABLE);
		}
		if(Trigger_level == EXTI_RISE || Trigger_level == EXTI_BOTH)
		{
			//配置上升沿中断
			EXTI_RisingTriggerSelect(NUM,ENABLE);
		}
		EXTI_INT_MASK(NUM);
		//使能中断
		NVIC_EnableIRQ(IRQn);				
	}
}
//GPIO0-7外部中断回调函数
void GPIO0_7_Handler(void)
{
	if(Read_EXTI_RisingEdgePending(EXTI_GPIO))
	{//上升沿中断触发
		
		GPIO_SetOutput(GPIO_11);
		
		//清除中断标志
		Clear_EXTI_RisingEdgePending(EXTI_GPIO);
	}
	if(Read_EXTI_FallingEdgePending(EXTI_GPIO))
	{//下降沿中断触发
		
		GPIO_ResetOutput(GPIO_11);
		
		//清除中断标志
		Clear_EXTI_FallingEdgePending(EXTI_GPIO);
	}
	
}
//GPIO8-15外部中断回调函数
void GPIO8_15_Handler(void)
{
	
}
//GPIO16-23外部中断回调函数
void GPIO16_23_Handler(void)
{
	
}


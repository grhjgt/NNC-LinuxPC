/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_GPIO.C         
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
#include "my_header.h"
#include "ENS1_GPIO.h"

//IO复用
uint8_t GPIO_AltFunction(GPIO_NUM GPIO_X , GPIO_AltFuncSel GPIO_AltFuncSelx){	
	if(GPIO_X >15)
	{
		CMSDK_GPIO->ALTFH &=~ (0x3 << ((GPIO_X-16)*2));
		CMSDK_GPIO->ALTFH |= (GPIO_AltFuncSelx << ((GPIO_X-16)*2));
	}
	else 
	{
		CMSDK_GPIO->ALTFL &=~ (0x3 << (GPIO_X*2) );
		CMSDK_GPIO->ALTFL |= (GPIO_AltFuncSelx << (GPIO_X*2));
	}
	return 0;
}


/*IO口输入输出设置
GPIO_X：gpio号
I_O_SELECT：输入输出选择
GPIOOType_TypeDef:推挽与开漏输出选择（输入无用）
GPIO_PUPD_TypeDef：上下拉电阻
OUTPUT_SPEED：输出速度（输入无用）
FunctionalState：使能与否
*/
int8_t GPIO_IO_Init(GPIO_NUM GPIO_X, I_O_SELECT INorOUT ,GPIOOType_TypeDef GPIO_OType,GPIO_PUPD_TypeDef GPIO_PUPD,OUTPUT_SPEED_TypeDef OUTPUT_SPEED,OUTPUT_PDRV_TypeDef OUTPUT_PDRV,FunctionalState ENABLEorNOT){
	//选择输入输出模式
	if(INorOUT == INPUT && ENABLEorNOT ==ENABLE)
	{
		CMSDK_GPIO->OE &=~ (1<<GPIO_X);
		CMSDK_GPIO->IE |= (1<<GPIO_X);
	}
	else if(INorOUT == INPUT && ENABLEorNOT ==DISABLE)
	{
		CMSDK_GPIO->OE &=~ (1<<GPIO_X);
		CMSDK_GPIO->IE &=~ (1<<GPIO_X);
	}
	else if(INorOUT == OUTPUT && ENABLEorNOT ==ENABLE)
	{
		CMSDK_GPIO->IE &=~ (1<<GPIO_X);
		CMSDK_GPIO->OE |= (1<<GPIO_X);
		//输出速度配置，快慢
		CMSDK_GPIO->SL = OUTPUT_SPEED;
		//配置推挽或开楼输出
		CMSDK_GPIO->ODEN = GPIO_OType;
	}
	else if(INorOUT == OUTPUT && ENABLEorNOT == DISABLE)
	{
		CMSDK_GPIO->OE &=~ (1<<GPIO_X);
		CMSDK_GPIO->IE &=~ (1<<GPIO_X);
	}

	//选择上下拉电阻
	if(GPIO_PUPD==GPIO_NOPULL)
	{
		CMSDK_GPIO->PU &= ~(0x01 << GPIO_X);		//禁止上拉
		CMSDK_GPIO->PD &= ~(0x01 << GPIO_X);		//禁止下拉
	}
	else if(GPIO_PUPD==GPIO_PU)
	{
		CMSDK_GPIO->PU |=  (0x01 << GPIO_X);		//上拉
		CMSDK_GPIO->PD &= ~(0x01 << GPIO_X);		//禁止下拉
	}
	else if(GPIO_PUPD==GPIO_PD)
	{
		CMSDK_GPIO->PU &= ~(0x01 << GPIO_X);		//禁止上拉
		CMSDK_GPIO->PD |=  (0x01 << GPIO_X);		//下拉
	}
	//输出驱动强度
	CMSDK_GPIO->OPDRV0 &=~( 1 << (GPIO_X));
	CMSDK_GPIO->OPDRV0 |= (OUTPUT_PDRV & 0x01) << (GPIO_X);
	CMSDK_GPIO->OPDRV1 &=~( 1 << (GPIO_X));
	CMSDK_GPIO->OPDRV1 |= ((OUTPUT_PDRV >>1)&0x01) << (GPIO_X);
	//
	return 0;
}

//读取IO口输入状态
uint8_t GPIO_GetInputValue(GPIO_NUM GPIO_X){ 
	return (uint8_t)((CMSDK_GPIO->DATAIN & (1<<GPIO_X))>>GPIO_X);
}

//读取IO口输出状态
uint8_t GPIO_GetOutputValue(GPIO_NUM GPIO_X){ 
	return (uint8_t)((CMSDK_GPIO->DATAOUT & (1<<GPIO_X))>>GPIO_X);
}

//设置输出电平为高或低
void GPIO_Output(GPIO_NUM GPIO_X,LEVELStatus HIGHorLOW){
	if(HIGHorLOW == HIGH_LEVEL)
	{
		CMSDK_GPIO->DATAOUT |= (1<<GPIO_X);
	}
	else 
	{
		CMSDK_GPIO->DATAOUT &=~ (1<<GPIO_X);
	}
}

//设置输出电平为高
void GPIO_SetOutput(GPIO_NUM GPIO_X){
	CMSDK_GPIO->DATAOUT |= (1<<GPIO_X);
}

//设置输出电平为低
void GPIO_ResetOutput(GPIO_NUM GPIO_X){
	CMSDK_GPIO->DATAOUT &=~ (1<<GPIO_X);
}

//IO反转
void  GPIO_Overturn(GPIO_NUM GPIO_X){
	if(GPIO_GetOutputValue(GPIO_X) == HIGH_LEVEL)
	{
		GPIO_ResetOutput(GPIO_X);
	}
	else if(GPIO_GetOutputValue(GPIO_X) == LOW_LEVEL)
	{   
		GPIO_SetOutput(GPIO_X);
	}		

}

//模拟功能使能失能
void GPIO_AnalogChannel_Control(GPIO_NUM GPIO_X ,FunctionalState ENABLEorNOT )
{
	if(ENABLEorNOT == ENABLE)
		CMSDK_GPIO->ANAEN |= (1 << GPIO_X);
	else if(ENABLEorNOT == DISABLE)
		CMSDK_GPIO->ANAEN &=~ (1 << GPIO_X);
	else
		return ;
}





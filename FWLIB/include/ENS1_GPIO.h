/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_GPIO.H          
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description: 
*Function List: 
     1  uint8_t  GPIO_AltFunction(GPIO_NUM GPIO_X , GPIO_AltFuncSel GPIO_AltFuncSelx);
     2  int8_t   GPIO_IO_Select(GPIO_NUM GPIO_X, I_O_SELECT INorOUT ,FunctionalState ENABLEorNOT);
     3  uint8_t  GPIO_GetInputValue(GPIO_NUM GPIO_X);
     4  uint8_t  GPIO_GetOutputValue(GPIO_NUM GPIO_X);
     5  void     GPIO_Output(GPIO_NUM GPIO_X,LEVELStatus HIGHorLOW);
     6  void     GPIO_SetOutput(GPIO_NUM GPIO_X);
     7  void     GPIO_ResetOutput(GPIO_NUM GPIO_X);
     8  void     GPIO_Overturn(GPIO_NUM GPIO_X);
	 9  void     GPIO_AnalogChannel_Control(GPIO_NUM GPIO_X ,FunctionalState ENABLEorNOT );
History: 
1.V1.0
Date:          
Author: 
Modification: 初版
*/
#ifndef ENS1_GPIO_H
#define ENS1_GPIO_H
#include "my_header.h"
#include "CMSDK_CM0.h"

//上下拉配置枚举
typedef enum 
{
	GPIO_PU = 0x00,			//上拉
	GPIO_PD = 0x01,			//下拉
	GPIO_NOPULL = 0x02		//浮空
}GPIO_PUPD_TypeDef;

//输出速度配置枚举
typedef enum 
{
	OUTPUT_FAST = 0x00,		//快速
	OUTPUT_SLOW = 0x01		//慢速
}OUTPUT_SPEED_TypeDef;

//输出模式配置枚举
typedef enum
{
	GPIO_OType_PP = 0x00,	//推挽输出
	GPIO_OType_OD = 0x01	//开漏输出
}GPIOOType_TypeDef;

//输出强度配置枚举
typedef enum 
{
	PDRV_4mA = 0x00,		//输出口输出强度4mA
	PDRV_8mA,				//输出口输出强度8mA
	PDRV_14mA,				//输出口输出强度14mA
	PDRV_16mA 				//输出口输出强度16mA
}OUTPUT_PDRV_TypeDef;		


uint8_t GPIO_AltFunction(GPIO_NUM GPIO_X , GPIO_AltFuncSel GPIO_AltFuncSelx);
int8_t GPIO_IO_Init(GPIO_NUM GPIO_X, I_O_SELECT INorOUT ,GPIOOType_TypeDef GPIO_OType,GPIO_PUPD_TypeDef GPIO_PUPD,OUTPUT_SPEED_TypeDef OUTPUT_SPEED,OUTPUT_PDRV_TypeDef OUTPUT_PDRV,FunctionalState ENABLEorNOT);
uint8_t GPIO_GetInputValue(GPIO_NUM GPIO_X);
uint8_t GPIO_GetOutputValue(GPIO_NUM GPIO_X);
void GPIO_Output(GPIO_NUM GPIO_X,LEVELStatus HIGHorLOW);
void GPIO_SetOutput(GPIO_NUM GPIO_X);
void GPIO_ResetOutput(GPIO_NUM GPIO_X);
void GPIO_Overturn(GPIO_NUM GPIO_X);
void GPIO_AnalogChannel_Control(GPIO_NUM GPIO_X ,FunctionalState ENABLEorNOT );



#endif




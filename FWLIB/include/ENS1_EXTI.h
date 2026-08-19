/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_EXTI.H         
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

#ifndef ENS1_EXTI_H
#define ENS1_EXTI_H
#include "my_header.h"


#define SET_GPIO	GPIO_5
#define EXTI_GPIO	EXTI_GPIO5

//触发电平选择
typedef enum {
	EXTI_FALL,
	EXTI_RISE,
	EXTI_BOTH
}EXTI_TRIGGER;

/*
0-23 : GPIO
24 : LVD output
25 : COMP0 output
26 : COMP1 output
27 : charge_ok
28 : charge_end
29 : over temperature
30 : rtc_alarm
31 : rtc_wut
*/
typedef enum {
	EXTI_GPIO0=0,
	EXTI_GPIO1=1,
	EXTI_GPIO2=2,
	EXTI_GPIO3=3,
	EXTI_GPIO4=4,
	EXTI_GPIO5=5,
	EXTI_GPIO6=6,
	EXTI_GPIO7=7,
	EXTI_GPIO8=8,
	EXTI_GPIO9=9,
	EXTI_GPIO10=10,
	EXTI_GPIO11=11,
	EXTI_GPIO12=12,
	EXTI_GPIO13=13,
	EXTI_GPIO14=14,
	EXTI_GPIO15=15,
	EXTI_GPIO16=16,
	EXTI_GPIO17=17,
	EXTI_GPIO18=18,
	EXTI_GPIO19=19,
	EXTI_GPIO20=20,
	EXTI_GPIO21=21,
	EXTI_GPIO22=22,
	EXTI_GPIO23=23,
	EXTI_LVDOUTPUT=24,
	EXTI_COMP0=25,
	EXTI_COMP1=26,
	EXTI_CHARGE_OK=27,
	EXTI_CHARGE_END=28,
	EXTI_OVER_TEMP=29,
}EXTI_NUM;

/*
寄存器： 
EXTI_RTSR: 上升沿触发选择寄存器
EXTI_FTSR: 下降沿触发选择寄存器
EXTI_SWIER: 软件中断事件
EXTI_RPR:  上升沿挂起寄存器
EXTI_FPR:  下降沿挂起寄存器
EXTI_IMR:  带中断掩码的CPU唤醒寄存器
EXTI_EMR:  带有事件掩码的CPU唤醒寄存器
*/
void EXTI_GPIO_Init(GPIO_NUM GPIO_X, EXTI_NUM NUM,EXTI_TRIGGER Trigger_level , IRQn_Type IRQn,FunctionalState ENABLEorNOT);

#endif


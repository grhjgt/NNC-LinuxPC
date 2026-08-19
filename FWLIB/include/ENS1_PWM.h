/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_PWM.H         
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
#ifndef ENS1_PWM_H
#define ENS1_PWM_H
#include "my_header.h"
#include "ENS1_CLOCK.h"

#define PRESCALE_PWM 1
/*
复用引脚
GPIO16  PWM1
GPIO17  PWM2
GPIO18  PWM3
GPIO19  PWM4
GPIO20  PWM5
GPIO21  PWM6
*/
typedef enum 
{ 
	IO16_PWM1 = 1 ,
    IO17_PWM2 = 2 ,
    IO18_PWM3 = 3 ,
    IO19_PWM4 = 4 ,
	IO20_PWM5 = 5 ,
	IO21_PWM6 = 6
}ENS1_PWM_CHANNEL;


/*
ENS1 PWM 功能特性
1、6个单边沿 和 3个双边沿控制输出 或 1个两种类型的混合
2、脉冲周期和宽度可以是任意定时器计数值
3、双边沿控制PWM输出可以被编程为正极或负极输出
4、匹配寄存器更新与脉冲输出同步以避免产生错误脉冲
5、如不启用PWM功能可作为标准定时器
6、一个有着32位预分频器的32位计时器
*/

/*
PWM使用说明：
1、2个匹配寄存器用于单边沿PWM输出控制，其中PWMMR0控制PWM周期速率，另一个匹配寄存器控制边沿位置（即控制脉宽）
2、3个匹配寄存器可用于双边沿PWM输出控制，其中PWMMR0控制PWM周期速率，另两个匹配控制寄存器控制双边位置
3、使用双边沿PWM的控制可以生成正向和负向脉冲
*/


/*------------------------------------------------函数声明---------------------------------------------*/
/*PWM初始化*/
typedef enum {pwm_single_mode = 0,pwm_double_positive_mode , pwm_double_negtive_mode } PWM_EDGE_MODE;

typedef struct ENS1_PWM_Para
{
	PWM_EDGE_MODE mode;
	uint32_t pwm_freq;
	uint8_t pwm_Duty_cycle ;

	
}PWM_ParaStructrue;


void PWM_init(ENS1_PWM_CHANNEL PWMx , PWM_ParaStructrue* PWM_Para ,  bool pwmEnable) ;
void PWM_SetFreq(ENS1_PWM_CHANNEL PWMx , PWM_ParaStructrue * PWM_Para );
void PWM_SetDutyCycle_SingleMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle);
void PWM_SetDutyCycle_DoublePositiveMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle);
void PWM_SetDutyCycle_DoubleNegtiveMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle);
void PWM_OutputDisable(ENS1_PWM_CHANNEL PWMx);
void PWM_OutputEnable(ENS1_PWM_CHANNEL PWMx);


#endif

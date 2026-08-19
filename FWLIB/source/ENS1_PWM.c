/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_PWM.C         
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

#include "ens1_pwm.h"
//PWMx        选择PWM输出通道
//edge_mode   pwm边沿模式  默认单边
//pulse_duty  占空比  0-100 （有些偏差）
//freq_HZ     频率   单位 HZ 
//pwmEnable   使能输出
void PWM_init(ENS1_PWM_CHANNEL PWMx , PWM_ParaStructrue* PWM_Para ,  bool pwmEnable)  
{	
	if(PWMx > 6 || PWMx < 1)
		PWMx = IO16_PWM1;
	int pwmsel = PWMx-2;
	if(pwmsel<0)
		pwmsel = 0;
	if(PWM_Para->pwm_Duty_cycle>100)
		PWM_Para->pwm_Duty_cycle=100;
    //IO复用
	CMSDK_GPIO->ALTFH |= (0x01 << ((PWMx-1)*2));
	uint32_t pulse_width = (uint32_t)((uint8_t)APB_Clock_Freq / ((PRESCALE_PWM+1) * (PWM_Para->pwm_freq)));
	CMSDK_PWM->MR0 = pulse_width;   	
	/*模式： 1 单边模式 2 双边正脉冲 3 双边负脉冲*/
	if(PWM_Para->mode == pwm_single_mode)
	{
		//pwm通道1-6的某一个通道的计数值填充
		*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)(PWM_Para->pwm_Duty_cycle*CMSDK_PWM->MR0/100);
	}	
	else if(PWM_Para->mode == pwm_double_positive_mode)
	{	
		//pwm通道1-6的某一个通道的计数值填充
		CMSDK_PWM->PCR |= (1<<pwmsel) ;
		if(PWMx < 6 || PWMx > 1)  //仅允许使用2-6
		{
			*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx-1))=(uint32_t)(0);
			*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)(PWM_Para->pwm_Duty_cycle*pulse_width/100);
		}
	}
	else
	{
		CMSDK_PWM->PCR |= (1<<pwmsel) ;
		if(PWMx < 6 || PWMx >1)  //仅允许使用2-6
		{
			*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx-1))=(uint32_t)(pulse_width);
			*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)((100-PWM_Para->pwm_Duty_cycle)*pulse_width/100);
		}
	}
	
	CMSDK_PWM->PR = PRESCALE_PWM;//Prescale  
	CMSDK_PWM->LER |= (1)|(1<<PWMx);      //加载使能
	CMSDK_PWM->TCR |= (1<<1);             //计数使能
	CMSDK_PWM->MCR |= (1<<1)  ;           //中断不使能和复位使能| ((PWMx*3+1)<<1)
	if(pwmEnable == true)
		CMSDK_PWM->PCR |= (0x01 << (PWMx+4));
	else
		CMSDK_PWM->PCR &=~ (0x01 << (PWMx+4));
}

void PWM_SetFreq(ENS1_PWM_CHANNEL PWMx , PWM_ParaStructrue * PWM_Para )
{
	CMSDK_PWM->MR0 = (uint32_t)((uint8_t)APB_Clock_Freq/ ((PRESCALE_PWM+1) * (PWM_Para->pwm_freq))); 
}	
	
void PWM_SetDutyCycle_SingleMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle)  //设置占空比
{
	PWM_OutputDisable(PWMx);
	CMSDK_PWM->LER &=~ (1<<PWMx);
	*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)(DutyCycle* CMSDK_PWM->MR0/100);
	CMSDK_PWM->LER |= (1<<PWMx);
	PWM_OutputEnable(PWMx);
}

void PWM_SetDutyCycle_DoublePositiveMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle)
{	
	if(PWMx < 2)
	{
		return ;
	}
	else 
	{
		PWM_OutputDisable(PWMx);
		CMSDK_PWM->LER &=~ (1<<PWMx);
		*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)(DutyCycle*CMSDK_PWM->MR0/100);
	CMSDK_PWM->LER |= (1<<PWMx);
	PWM_OutputEnable(PWMx);
	}
}

void PWM_SetDutyCycle_DoubleNegtiveMode(ENS1_PWM_CHANNEL PWMx  ,uint8_t DutyCycle)
{
	if(PWMx < 2)
	{
		return ;
	}
	else 
	{
		PWM_OutputDisable(PWMx);
		CMSDK_PWM->LER &=~ (1<<PWMx);
		*(unsigned long*)(CMSDK_PWM_BASE + 0x018 + 0x4*(PWMx))=(uint32_t)((100-DutyCycle)*CMSDK_PWM->MR0/100);
	CMSDK_PWM->LER |= (1<<PWMx);
	PWM_OutputEnable(PWMx);
	}
}

void PWM_OutputDisable(ENS1_PWM_CHANNEL PWMx)
{
	CMSDK_PWM->PCR &=~ (0x01 << (PWMx+4));
}

void PWM_OutputEnable(ENS1_PWM_CHANNEL PWMx)
{
	CMSDK_PWM->PCR |= (0x01 << (PWMx+4));	
}

uint32_t pwm_irq_occurred_mr[7];
void PWM_Handler(void) {
//MR0 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR0_INT_STS_Msk)==CMSDK_PWM_MR0_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR0_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[0]++;
    }
//MR1 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR1_INT_STS_Msk)==CMSDK_PWM_MR1_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR1_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[1]++;
    }
//MR2 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR2_INT_STS_Msk)==CMSDK_PWM_MR2_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR2_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[2]++;
    }
//MR3 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR3_INT_STS_Msk)==CMSDK_PWM_MR3_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR3_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[3]++;
    }
//MR4 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR4_INT_STS_Msk)==CMSDK_PWM_MR4_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR4_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[4]++;
    }
//MR5 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR5_INT_STS_Msk)==CMSDK_PWM_MR5_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR5_INT_STS_Msk;//Clear Interrupt
	pwm_irq_occurred_mr[5]++;
    }
//MR6 status interrupt
    if((CMSDK_PWM->INTSTATUS & CMSDK_PWM_MR6_INT_STS_Msk)==CMSDK_PWM_MR6_INT_STS_Msk) {
    	CMSDK_PWM->INTCLEAR = CMSDK_PWM_MR6_INT_STS_Msk;//Clear Interrupt	
	pwm_irq_occurred_mr[6]++;
    }    
}


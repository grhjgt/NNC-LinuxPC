/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_ANAC.c         
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

#include "ens1_anac.h"
#include "ENS1_CLOCK.h"
#include "ENS1_ADC.h"

/*----------------------------------------------------COMP----------------------------------------------------------------*/

/*比较器的输出设置*/
void CompInitSet(COMP_ConfigStructure* COMPCONFIG)
{
	if(COMPCONFIG->COMPARATOR_NUM == COMP0)
	{
		//IO复用
		GPIO_AltFunction(GPIO_8 ,  ALT_FUNC3);
		GPIO_AltFunction(GPIO_9  , ALT_FUNC3);
		GPIO_AltFunction(GPIO_10 , ALT_FUNC3);
		GPIO_AltFunction(GPIO_11 , ALT_FUNC3);
		GPIO_AltFunction(GPIO_20 , ALT_FUNC2);
		CMSDK_ANAC->COMP0_CTRL |=  ( COMPCONFIG->COMP_VREF_SEL << 4 );
		CMSDK_ANAC->COMP0_CTRL |=  ( COMPCONFIG->COMP_SIGSEL_NEGATIVE_INPUT_SEL << 2 );
		CMSDK_ANAC->COMP0_CTRL |=  ( COMPCONFIG->COMP_SIGSEL_POSITIVE_INPUT_SEL << 1 );
	}
	else if(COMPCONFIG->COMPARATOR_NUM == COMP1)
	{	
		GPIO_AltFunction(GPIO_12 ,  ALT_FUNC3);
		GPIO_AltFunction(GPIO_13  , ALT_FUNC3);
		GPIO_AltFunction(GPIO_14 , ALT_FUNC3);
		GPIO_AltFunction(GPIO_15 , ALT_FUNC3);
		GPIO_AltFunction(GPIO_21 , ALT_FUNC2);
		CMSDK_ANAC->COMP1_CTRL |=  ( COMPCONFIG->COMP_VREF_SEL << 4 );
		CMSDK_ANAC->COMP1_CTRL |=  ( COMPCONFIG->COMP_SIGSEL_NEGATIVE_INPUT_SEL << 2 );
		CMSDK_ANAC->COMP1_CTRL |=  ( COMPCONFIG->COMP_SIGSEL_POSITIVE_INPUT_SEL << 1 );
	}
	else 
		return ;
}

uint8_t Read_Comp_Output(COMP_NUM NUM)
{
	if(NUM == COMP0)
		return (uint8_t)((CMSDK_ANAC->COMP0_CTRL >> 8)&0X1);
	else 
		return (uint8_t)((CMSDK_ANAC->COMP1_CTRL >> 8)&0X1);
}

void CompControl(COMP_NUM NUM ,FunctionalState Newstate)
{
	if(NUM == COMP0)
		(Newstate == ENABLE)?   (CMSDK_ANAC->COMP0_CTRL |= (1) ): ( CMSDK_ANAC->COMP0_CTRL &=~ (1));
	else 
		(Newstate == ENABLE)?   (CMSDK_ANAC->COMP1_CTRL |= (1) ): ( CMSDK_ANAC->COMP1_CTRL &=~ (1));
}

/*--------------------------------------------------------PGA---------------------------------------------------------------*/
//初始化
void PGAInitSet(PGA_ConfigStructure* PGACONFIG)
{
	
	if(PGACONFIG->PGA_NEG_SELx == PGA_NEGATIVE_INPUT_PGA_VIN0)   //PGA0
	{
		GPIO_AltFunction(GPIO_16 ,  ALT_FUNC3);
		GPIO_AltFunction(GPIO_18  , ALT_FUNC3);
		GPIO_AnalogChannel_Control(GPIO_16 ,ENABLE );
		GPIO_AnalogChannel_Control(GPIO_18 ,ENABLE );
	}
	
	else if(PGACONFIG->PGA_NEG_SELx  == PGA_NEGATIVE_INPUT_PGA_VIN1)  //PGA1
	{
		GPIO_AltFunction(GPIO_17 , ALT_FUNC3);
		GPIO_AltFunction(GPIO_19 , ALT_FUNC3);	
		GPIO_AnalogChannel_Control(GPIO_17 ,ENABLE );
		GPIO_AnalogChannel_Control(GPIO_19 ,ENABLE );
	}
	
	else if((PGACONFIG->PGA_NEG_SELx ==PGA_NEGATIVE_INPUT_INTERNAL_VCM) && ((PGACONFIG->PGA_POS_SELx == PGA_POSITIVE_INPUT_PGA_VIP0 )||(PGACONFIG->PGA_POS_SELx == PGA_POSITIVE_INPUT_PGA_VIP1)))  //内部参考电压+0/1
	{
		GPIO_AltFunction((GPIO_NUM)(GPIO_16+PGACONFIG->PGA_POS_SELx),  ALT_FUNC3);
		GPIO_AnalogChannel_Control((GPIO_NUM)(GPIO_16+PGACONFIG->PGA_POS_SELx) ,ENABLE );
	}

	else if((PGACONFIG->PGA_NEG_SELx ==PGA_NEGATIVE_INPUT_EXTERNAL_VCM) && ((PGACONFIG->PGA_POS_SELx == PGA_POSITIVE_INPUT_PGA_VIP0 )||(PGACONFIG->PGA_POS_SELx == PGA_POSITIVE_INPUT_PGA_VIP1)))  //内部参考电压+0/1
	{
		GPIO_AltFunction((GPIO_NUM)(GPIO_16+PGACONFIG->PGA_POS_SELx),  ALT_FUNC3);
		GPIO_AltFunction(GPIO_20,  ALT_FUNC3);
		GPIO_AnalogChannel_Control((GPIO_NUM)(GPIO_16+PGACONFIG->PGA_POS_SELx) ,ENABLE );
		GPIO_AnalogChannel_Control(GPIO_20 ,ENABLE );
	}
	
	else if((PGACONFIG->PGA_POS_SELx ==  PGA_POSITIVE_INPUT_INTERNAL_VCM) && ((PGACONFIG->PGA_NEG_SELx == PGA_NEGATIVE_INPUT_PGA_VIN0)||(PGACONFIG->PGA_NEG_SELx == PGA_NEGATIVE_INPUT_PGA_VIN1)))
	{
		GPIO_AltFunction((GPIO_NUM)(GPIO_18+PGACONFIG->PGA_POS_SELx),  ALT_FUNC3);
		GPIO_AnalogChannel_Control((GPIO_NUM)(GPIO_18+PGACONFIG->PGA_POS_SELx) ,ENABLE );
	}
	else if((PGACONFIG->PGA_POS_SELx ==  PGA_POSITIVE_INPUT_VREF1_2V) && ((PGACONFIG->PGA_NEG_SELx == PGA_NEGATIVE_INPUT_PGA_VIN0)||(PGACONFIG->PGA_NEG_SELx == PGA_NEGATIVE_INPUT_PGA_VIN1)))
	{
		GPIO_AltFunction((GPIO_NUM)(GPIO_18+PGACONFIG->PGA_POS_SELx),  ALT_FUNC3);
		GPIO_AnalogChannel_Control((GPIO_NUM)(GPIO_18+PGACONFIG->PGA_POS_SELx) ,ENABLE );
	}
	else 
	{
		return ;
	}
	
	if(PGACONFIG->PGA_OUT == PGA_TO_GPIO4)
	{
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 4;
		GPIO_AltFunction(GPIO_4,  ALT_FUNC3);   
		GPIO_AnalogChannel_Control(GPIO_4 ,ENABLE );
		ENS1_ADC_STOP(ENS1_PGA_TO_ADC);
	}
	else if(PGACONFIG->PGA_OUT == PGA_TO_ADC)
	{
		ENS1_ADCCLKConfig(ADC_CLK_base32div);
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 0;
		ENS1_ADC_CONFIG(ENS1_PGA_TO_ADC ,
							 CONTINUOUS_ADC_MODE,    
								COV_RCV_EOC ,
									ADC_SampleTime_5ADC_Clk,
									   ENABLE_EOC_INT);
		ENS1_ADC_START(ENS1_PGA_TO_ADC);
	}
	else 
	{
		return ;
	}
	CMSDK_ANAC->PGA_CTRL |= ( PGACONFIG->PGA_GAIN_SEL << 4 );
}

//PGA开关控制
void PGAControl(FunctionalState Newstate)
{
	(Newstate == ENABLE) ? (CMSDK_ANAC->PGA_CTRL |= (0X3) ) : (CMSDK_ANAC->PGA_CTRL &=~ (0X3)) ;
}


/*----------------------------低电压与芯片过温检测--------------------------*/
void PMU_TEMP150C_TRIGControl(FunctionalState Newstate)  //过温检测功能开启与关闭
{
	(Newstate==ENABLE) ? (CMSDK_ANAC->PMU_CTRL |= (1<<5) ) : (CMSDK_ANAC->PMU_CTRL &=~ (1<<5));
}

//返回温度是否超过150度的状态值
int8_t TEMP_150C_TRIG_SIGNAL(void)
{
	if((CMSDK_ANAC->PMU_CTRL >> 5) & 0x1)
		return ((CMSDK_ANAC->PMU_CTRL >> 7) & 0x1);
	else 
	{	
		PMU_TEMP150C_TRIGControl(ENABLE);
		return ((CMSDK_ANAC->PMU_CTRL >> 7) & 0x1);
	}
}

//低电压告警
//设置阈值电压
void LVD_InitSet(uint8_t Threshold_voltage)
{
	CMSDK_ANAC->PMU_CTRL |= (Threshold_voltage << 1);
}
//打开或关闭此功能
void PMU_LVD_Control(FunctionalState Newstate)
{
	(Newstate == ENABLE) ? (CMSDK_ANAC->PMU_CTRL|= (1) ) : (CMSDK_ANAC->PMU_CTRL &=~ (1)) ;
}
//读取低电压状态
uint8_t Read_LVD_Signal(void)
{
	return ((CMSDK_ANAC->PMU_CTRL >> 6) & 0x1);
}
/*------------------------带隙BUFFER启用或关闭-----------------------------------*/
void PMU_BANDGAP_CONTROL(FunctionalState Newstate)
{
	(Newstate == ENABLE) ? (CMSDK_ANAC->PMU_CTRL |= (1<<4)) : (CMSDK_ANAC->PMU_CTRL &=~ (1<<4)) ;
}







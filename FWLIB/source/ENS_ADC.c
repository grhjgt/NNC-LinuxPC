/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_ADC.C          
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

#include "ENS1_ADC.h"
#include "ENS1_GPIO.h"
#include "ENS1_CLOCK.h"
#include "ENS1_ANAC.h"
/* 一、寄存器说明：
ADC配置：转换模式  数据覆盖模式  等待模式
ADC控制：  ADC使能  ， ADC启动
ADC中断使能： EOC中断使能    数据覆盖中断使能
ADC中断状态： EOC中断发生    数据覆盖中断发生
ADC状态 ：  EOC标志    ADC空闲或繁忙状态
ADC时钟分频 ：分频值 2 4 6 8 10 12 16 32
ADC采样时间 ：ADC采样时间时钟数 2 3 4 5 
ADC数据：
ADC通道选择 ：
ADCEOC配置  ：在连续模式下 ， 是否在接收到EOC标志后开启下次转换
*/

/* 二、单次转换模式
 *ADC控制启将 给ADC_START 位置0 当接收到EOC或者EOC_WAIT_COUNT_DONE ， 无论哪一个先到来
 *如果直到EOC_WAIT_COUNT_DONE 都没有接收到EOC ，数据将不会被保存
 *ADC将停止转换，通过用户设置ADC_CTRL_REG 的 ADC_EN 位 为1 
 *如果在采样和转换期间，ADC_EN 位置为0 ，控制器将完成正在进行的转换，然后基于 EOC/EOC_WAIT_COUNT_DONE 停止转换
 *数据会在EOC 到达后锁存
 
ADC单次转换流程：
1、设置单次转换（无等待模式）
2、设置ADC_CONFIG_reg  bit0 = 0  bit2 = 0
3、单次转换启动
4、转换结束， 数据保存至ADC_Data register
5、一个ADC_EOC_IE中断生成，
6、如果使能了 IER寄存器的 EOC_INT_EN 和OVER_RUN_INT_EN 位，则系统进入中断
7、硬件停止ADC
*/

/*三、连续转换模式
1、设置ADC_CONFIG_reg  bit0 = 1  
2、启动位置1   ADC_EN bit and ADC_START bits
3、在每次转换完成之后，基于adc_eoc_config_regsiter[0] 启动下次转换：
    如果Adc_eoc_config_register[0] =1 ， 则接收到EOC后开始下次转换
	数据保存到寄存器中
4、ADC_EOC_IE 中断生成
5、如果使能了 IER寄存器的 EOC_INT_EN 和OVER_RUN_INT_EN 位，则系统进入中断

注意：ADC_eoc_config_reg寄存器仅仅在连续采样-非等待模式下有效  即：0：连续模式不接收EOC即开始启动下一次转换
                                                                   1：连续模式接收到EOC后开始启动下一次转换
在连续采样-等待模式下，ADC_eoc_config_reg应该被设置为0 ！！！
*/

/*四、等待模式
1、设置ADC_CONFIG_reg  bit2 = 1  （使能等待模式）
2、启动测量转换
3、在转换完成后：
   存ADC数据到相应寄存器中
   ADC_EOC_IE 中断生成
   如果使能了 IER寄存器的 EOC_INT_EN 和OVER_RUN_INT_EN 位，则系统进入中断
4、ADC控制器进入到ADC_WAIT 等待状态直到EOC中断被清除 或者 ADC数据被系统读走
5、一旦EOC中断被清除，ADC开始下一次的转换
*/

/*五、数据锁存器
1、当ADC控制启接收到EOC后，数据将被锁存
2、如果 OVERRUN模式被使能，新的转换数据将会被锁存 无论 overrun 错误状态如何

注意： 如果overrun 模式没有被使能， 新的或者旧的转换数据将根据overrun 错误状态选择性锁存
       即： 如果overrun 错误发生，数据将不会被锁存
	        如果没有overrun 错误发生，新数据将被锁存
*/

/*六、中断说明
1、接收到EOC后 EOC_IR 生成
2、OVERRUN错误发生后OVERRUN_IR 生成
3、EOC_IR 和overrun 作为ADC中断被 通道到系统
   在系统通过 data_reg 读取走adc数据后， EOC_IR 被 EOC_IR_CLEAR 清除
*/

/*overrun 错误
   此错误指的是： ADC控制器在系统读取走ADC数据前（在清除到上一个EOC_IR前），接收到新的EOC
*/


volatile uint8_t ADC_READ_STATUS = ADC_READ_DATA_IS_WAITING;
uint8_t ADC_UART_BYTE_LOW = 0;
uint8_t ADC_UART_BYTE_HIGH = 0;

uint8_t ENS1_ADCCLKConfig(uint8_t ADC_CLK_div)
{
	CMSDK_ADC->ADC_CLK_DIV = ((CMSDK_ADC->ADC_CLK_DIV &~ (0x7)) | (ADC_CLK_div));
	return 0;
}
/*
	ENS_ADC_COV_MODE COV_MODE 
	ENS_ADC_OVERRUN_MODE OVERRUN_MODE 
	WAIT_MODE WAITorNOT 
*/
uint8_t ENS1_ADC_CONFIG(ENS_ADC_SEL channelx ,
							uint8_t MODE_SEL, 
								ENS_ADC_COV_INC_EOC EOC_CONFIG ,
									uint8_t SIMLING_TIME,
									   uint8_t INT_MODE_SEL)
{
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_DisableIRQ(ADC_IRQn); 
	if(channelx == ENS1_ADC_CHANNEL1)
	{
		CMSDK_GPIO->ALTFH = (CMSDK_GPIO->ALTFH &~ (0x03<<10))|(0x03<<10); 
		CMSDK_GPIO->ANAEN |= (1 << 21); 
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 1;
	}
	else if(channelx == ENS1_ADC_CHANNEL2)
	{
		CMSDK_GPIO->ALTFH = (CMSDK_GPIO->ALTFH &~ (0x03<<12))|(0x03<<12);
		CMSDK_GPIO->ANAEN |= (1 << 22); 
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 2;
	}	
	else if(channelx == ENS1_ADC_CHANNEL3)
	{	
		CMSDK_GPIO->ALTFH = (CMSDK_GPIO->ALTFH &~ (0x03<<14))|(0x03<<14);
		CMSDK_GPIO->ANAEN |= (1 << 23); 
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 3;
	}
	else
	{
		CMSDK_ADC->ADC_CH_SEL &=~ (0x7);
		CMSDK_ADC->ADC_CH_SEL = 0;
	}
	
		
	CMSDK_ADC->ADC_CONFG =(CMSDK_ADC->ADC_CONFG &~ 0x7 )| MODE_SEL;
	
	CMSDK_ADC->ADC_SAMP_TIME = (CMSDK_ADC->ADC_SAMP_TIME &~ 0x3) | SIMLING_TIME;
	
	//配置ADC_eoc_config_reg寄存器，（仅仅在连续采样-非等待模式下有效）
	if( ((MODE_SEL & 0X1 )== 1 ) && (MODE_SEL & 0X4) == 0)
	{
		CMSDK_ADC->ADC_EOC_CONFG = (CMSDK_ADC->ADC_EOC_CONFG &~ (0x1)) | EOC_CONFIG;  
	}
	
	//中断使能
	CMSDK_ADC->ADC_IER = (CMSDK_ADC->ADC_IER &~ (0x3)) | ( INT_MODE_SEL );
	return CMSDK_ADC->ADC_CONFG;
}

uint8_t ENS1_ADC_START(ENS_ADC_SEL channelx )
{
	CMSDK_ADC->ADC_CTRL |= (1) |(1<<8);
	if(CMSDK_ADC->ADC_IER != 0)
	{
		NVIC_EnableIRQ(ADC_IRQn);
	}
	return 0;
}

uint8_t ENS1_ADC_STOP(ENS_ADC_SEL channelx)
{
	CMSDK_ADC->ADC_CTRL &=~ (1);
	NVIC_DisableIRQ(ADC_IRQn); 
	return 0;
}

//连续读取数据
//ADC在不同模式下有不同的采集方式，根据Adc_config_register的配置不同，共8种模式
uint16_t save_data;
uint8_t ADC_CONFIG_READ;
//此函数未完成，测试使用连续采集+中断模式，此函数暂时用不到

uint16_t ADC_READ_DATA(void)
{
	
	ADC_CONFIG_READ = CMSDK_ADC->ADC_CONFG;
	switch(ADC_CONFIG_READ & 0x7) {
		case  single_mode_without_overrun_without_wait :
			while(ADC_READ_STATUS == ADC_READ_DATA_IS_WAITING);  //等待在中断中EOC的到来
			ADC_READ_STATUS = ADC_READ_DATA_IS_WAITING; //中断发生，已经有ADC的数据了，此时再将状态切换到等待下一次数据
			
		break;
		
		case  continious_mode_without_overrun_without_wait :
			
		break;
		
		case  single_mode_with_overrun_without_wait :
			
		break;
		
		case  continious_mode_with_overrun_without_wait :
			
		break;
		
		case  single_mode_without_overrun_with_wait :
				
		
		
		break;
		
		case  Continious_mode_without_overun_with_wait :
			
		break;
		
		case  single_mode_with_overrun_with_wait :
			
		break;
		
		case  continious_mode_with_overrun_with_wait :
			
		break;
		
	}
	return (uint16_t)save_data;
}

//ADC interrupt handler
void ADC_Handler(void) __irq
{
	if((CMSDK_ADC->ADC_ISR & 0x01) == 0x01)  //接收到EOC
	{
		CMSDK_ADC->ADC_INT_CLR = (0x01<<0);  
		ADC_READ_STATUS = ADC_READ_DATA_IS_READY;
		save_data = (CMSDK_ADC->ADC_DATA & 0x0fff);  //读走数据后，可以进行下一次采集
		printf("%d\n",save_data);
		ADC_UART_BYTE_LOW = save_data&0xff;
		ADC_UART_BYTE_HIGH = (save_data&0x0f00)>>8;
	}
	if(((CMSDK_ADC->ADC_ISR & 0x02)>>1) == 0x01) //overrun error 
	{
		CMSDK_ADC->ADC_INT_CLR = (0x01<<1);
	}
}





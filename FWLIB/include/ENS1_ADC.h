/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_ADC.H          
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

#ifndef ENS1_ADC_H
#define ENS1_ADC_H
#include "my_header.h"

/*
GPIO21  ---  ADC 1
GPIO22  ---  ADC 2
GPIO23  ---  ADC 3

*/
extern uint16_t save_data;
extern uint8_t ADC_UART_BYTE_LOW ;
extern uint8_t ADC_UART_BYTE_HIGH ;
typedef enum { ENS1_PGA_TO_ADC = 0 ,ENS1_ADC_CHANNEL1 = 1,ENS1_ADC_CHANNEL2 = 2,ENS1_ADC_CHANNEL3 = 3}ENS_ADC_SEL;
//typedef enum { SINGLE_ADC_MODE = 0, CONTINUOUS_ADC_MODE = 1}ENS_ADC_COV_MODE;
//typedef enum { NOOVERRUN_ADC_MODE = 0, OVERRUN_ADC_MODE = 1}ENS_ADC_OVERRUN_MODE;
//typedef enum { NOWAIT_ADC_MODE = 0, WAIT_ADC_MODE = 1}WAIT_MODE;
typedef enum { NCOV_WITHOUT_RCV_EOC = 0 , COV_RCV_EOC = 1}ENS_ADC_COV_INC_EOC;

/*模式分类：   MODE_SEL
      单次采集
      连续采集
      OVERRUN
      关闭OVERRUN
      等待模式开启
      等待模式关闭
*/
#define SINGLE_ADC_MODE       0
#define CONTINUOUS_ADC_MODE	  (1)
#define OVERRUN_ADC_MODE      (1<<1)
#define WAIT_ADC_MODE         (1<<2)

/*中断模式选择 INT_MODE_SEL */
#define  DISABLE_INT (0)
#define  ENABLE_EOC_INT   (1)
#define  ENABLE_OVERRUN_INT (1<<1)

/*
ADC读取状态
*/
#define ADC_READ_DATA_IS_WAITING 0x00
#define ADC_READ_DATA_IS_READY   0x01

/*
SIMLING_TIME
*/
#define ADC_SampleTime_2ADC_Clk  ((uint8_t)0x00)
#define ADC_SampleTime_3ADC_Clk	 ((uint8_t)0x01)	
#define ADC_SampleTime_4ADC_Clk  ((uint8_t)0x02)
#define ADC_SampleTime_5ADC_Clk  ((uint8_t)0x03)


/*

*/
#define ADC_CLK_base2div ((uint8_t)0x00)
#define ADC_CLK_base4div ((uint8_t)0x01)
#define ADC_CLK_base6div ((uint8_t)0x02)
#define ADC_CLK_base8div ((uint8_t)0x03)
#define ADC_CLK_base10div ((uint8_t)0x04)
#define ADC_CLK_base12div ((uint8_t)0x05)
#define ADC_CLK_base16div ((uint8_t)0x06)
#define ADC_CLK_base32div ((uint8_t)0x07)


/*

*/
#define single_mode_without_overrun_without_wait    0
#define single_mode_without_overrun_with_wait       4
#define single_mode_with_overrun_without_wait       2
#define single_mode_with_overrun_with_wait          6
#define continious_mode_without_overrun_without_wait 1
#define continious_mode_with_overrun_without_wait    3
#define Continious_mode_without_overun_with_wait     5
#define continious_mode_with_overrun_with_wait       7



/*
声明 ADC相关函数
*/
extern uint8_t ENS1_ADCCLKConfig(uint8_t ADC_CLK_div);
extern uint8_t ENS1_ADC_CONFIG(ENS_ADC_SEL channelx ,
							uint8_t MODE_SEL, 
								ENS_ADC_COV_INC_EOC EOC_CONFIG ,
									uint8_t SIMLING_TIME,
									   uint8_t INT_MODE_SEL);
extern uint8_t ENS1_ADC_START(ENS_ADC_SEL channelx);
extern uint8_t ENS1_ADC_STOP(ENS_ADC_SEL channelx);
extern uint16_t ADC_READ_DATA(void);





#endif


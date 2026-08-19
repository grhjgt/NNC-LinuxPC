/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_CLOCK.H          
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description: 
*Function List: 
    1 uint32_t ClockInitSet(Clock_ConfigStructure* CLOCKCONFIG);
    2 void ClockInit(void);
    3 uint8_t PCLK_Enable(uint8_t APB_CLKEN_POS);
    4 uint8_t PCLK_Disable(uint8_t APB_CLKEN_POS);
History: 
1.V1.0
Date:          
Author: 
Modification: 初版
*/


#ifndef ENS1_CLOCK_H
#define ENS1_CLOCK_H
#include "my_header.h"
#include "CMSDK_CM0.h"

/*
时钟树结构
低速（LSI RC / LSE OSC） --> LFCLK --> RTC / LCD
高速 (HSI RC / HSE    ) --> SYSCLK --> 分频（AHB Prescaler）-----> SYSC / PMU / HCLK /SCLK /DCLK 
                                                             |
                                                             |
                                                             |
															 ---->分频（APB Prescaler） --> PCLK
*/
extern uint32_t APB_Clock_Freq;
//分频选择
//APB
#define ENS1_APB_PCLK_DIV_1 0x0
#define ENS1_APB_PCLK_DIV_2 0x4
#define ENS1_APB_PCLK_DIV_4 0x5
#define ENS1_APB_PCLK_DIV_8 0x6
#define ENS1_APB_PCLK_DIV_16 0x7
//AHB
#define ENS1_AHB_HCLK_DIV_1 0X0
#define ENS1_AHB_HCLK_DIV_2 0X4
#define ENS1_AHB_HCLK_DIV_4 0X5
#define ENS1_AHB_HCLK_DIV_8 0X6
#define ENS1_AHB_HCLK_DIV_16 0X7

typedef enum{HSI_4MHZ =0 , HSI_8MHZ=1 , HS_16MHZ=2 , HSI_32MHZ=3}HSI_FREQ_SEL;
typedef enum{MCO_HSI=0, MCO_HSE ,MCO_LSI,MCO_LSE}MCU_CLOCK_OUTPUT_SEL;     //mcu时钟输出选择
typedef enum{LSI_AS_LFCLK =0, LSE_AS_LFCLK }LFCLK_SWITCH_SEL;              //LFCLK输入源选择
typedef enum{HSI_SYSCLK = 0 ,HSE_SYSCLK , LFCLK_SYSCLK}SYSTEM_CLOCK_SEL;   //选择系统时钟源，在离开stop模式时，系统会强制设置为00

/*--------时钟初始化配置结构体----------*/
typedef struct ClockConfig
{	
	MCU_CLOCK_OUTPUT_SEL MCO_SEL;     //输入 
	HSI_FREQ_SEL HSI_FREQ;            //内部高速RC频率选择
	uint8_t HSE_OSC_FREQ;             //外部高速晶振实际频率（单位MHZ）
	uint16_t LSE_OSC_FREQ;            //外部低速晶振实际频率（单位1）
	SYSTEM_CLOCK_SEL  SYSCLK_SEL; 	  //系统时钟源的选择
	uint8_t ENS1_APB_PCLK_DIV_x;      //PCLK分频选择
	uint8_t ENS1_AHB_PCLK_DIV_x;      //HCLK分频选择
	LFCLK_SWITCH_SEL  LFCLK_SW_SEL;   //选择lfclk的输入源,为RTC的时钟输入

}Clock_ConfigStructure;

extern uint32_t ClockInitSet(Clock_ConfigStructure* CLOCKCONFIG);
extern void ClockInit(void);
//PCLK 的时钟位
#define UART0_PCLK_EN   0
#define UART1_PCLK_EN   1
#define SPI0_PCLK_EN    2
#define SPI1_PCLK_EN    3
#define I2C0_PCLK_EN    4
#define I2C1_PCLK_EN    5
#define WDT_PCLK_EN 	6
#define PWM_PCLK_EN 	7
#define TIMER0_PCLK_EN  8
#define TIMER1_PCLK_EN  9
#define DUAL_TIMER_PCLK_EN  10
#define LCD_PCLK_EN 		11
#define WAVE_GEN_PCLK_EN 	12
#define ADC_PCLK_EN 		13
#define ANALOG_PCLK_EN 		14
#define RTC_PCLK_EN 		15
extern uint8_t PCLK_Enable(uint8_t APB_CLKEN_POS);
extern uint8_t PCLK_Disable(uint8_t APB_CLKEN_POS);
void HSE_ClockInit(uint32_t Clock_Freq);
#endif


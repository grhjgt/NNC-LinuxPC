#ifndef ENS1_TIMER_H
#define ENS1_TIMER_H
#include "CMSDK_CM0.h"
#include "my_header.h"


typedef enum {TOTAL_TIME_MODE = 0 , TRIGGER_TIME_MODE=1}TIME_COUNT_MODE; //选择为总时间或有效运行时间

//双定时器模式选择枚举
typedef enum {
	SETUP_freerunning=0,	//自由运行模式
	SETUP_periodic=1,		//周期运行模式
	SETUP_oneshot=2			//单次运行模式
} DUALTIMER_MODE;

//双定时器分频系数选择枚举
typedef enum {
	PRESCALE_1=0,			//不分频
	PRESCALE_16=1,			//16分频
	PRESCALE_256=2			//256分频
} DUALTIMER_PRESCALE;

//双定时器位数选择枚举
typedef enum {
	SIZE_16=0,				//16位计数寄存器
	SIZE_32=1				//32位计数寄存器
} DUALTIMER_SIZE;





void TIMER0_Init(uint32_t Int_Period);
void TIMER1_Init(uint32_t Int_Period);
void DUALTIMER1_Init(DUALTIMER_MODE SETUP_x,uint32_t Int_Period, DUALTIMER_PRESCALE PRESCALE_x,  FunctionalState irq_en, DUALTIMER_SIZE SIZE_x );
void DUALTIMER2_Init(DUALTIMER_MODE SETUP_x,uint32_t Int_Period, DUALTIMER_PRESCALE PRESCALE_x,  FunctionalState irq_en, DUALTIMER_SIZE SIZE_x );
void CMSDK_timer_EnableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_DisableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_StartTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_StopTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
uint32_t CMSDK_timer_GetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_SetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value);
uint32_t CMSDK_timer_GetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_SetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value);
void CMSDK_timer_ClearIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
uint32_t  CMSDK_timer_StatusIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
void CMSDK_timer_Init(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload, bool irq_en);
void CMSDK_timer_Init_ExtClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,uint32_t irq_en);
void CMSDK_timer_Init_ExtEnable(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,uint32_t irq_en);
void CMSDK_dualtimer_start(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx);
void CMSDK_dualtimer_stop(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx);
void CMSDK_dualtimer_irq_clear(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx);
void CMSDK_dualtimer_setup_freerunning(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
													unsigned int cycle, unsigned int prescale, 
															unsigned int interrupt, unsigned int size);
void CMSDK_dualtimer_setup_periodic(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
												unsigned int cycle, unsigned int prescale, 
														unsigned int interrupt, unsigned int size);
void CMSDK_dualtimer_setup_oneshot(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
													unsigned int cycle, unsigned int prescale,
															unsigned int interrupt, unsigned int size);
void CMSDK_RTC_Init_Calender(uint16_t prescaler, uint8_t data_mode, uint8_t hour_mode, uint32_t init_time, uint32_t init_date);
void CMSDK_RTC_Config_Alarm(uint16_t prescaler, uint8_t data_mode, uint8_t hour_mode, uint32_t init_time, uint32_t init_date, uint32_t alarm_time, uint32_t alarm_date)	;
void CMSDK_RTC_Config_PeriodWake(uint8_t clock_sel, uint16_t prescaler, uint16_t period_time);		
													
#endif


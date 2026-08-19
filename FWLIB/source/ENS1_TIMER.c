#include "ENS1_TIMER.h"
#include "ENS_CURRENT_CALIBRATION.h"
#include "ENS1_CLOCK.h"

void TIMER0_Init(uint32_t Int_Period) //形参，输入中断触发周期 （单位ms） 
{
	PCLK_Enable(TIMER0_PCLK_EN);	
	NVIC_DisableIRQ(TIMER0_IRQn);
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
	CMSDK_timer_Init(CMSDK_TIMER0,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period) , true); //
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER1_Init(uint32_t Int_Period)
{
	PCLK_Enable(TIMER1_PCLK_EN);	
	NVIC_DisableIRQ(TIMER1_IRQn);
	NVIC_ClearPendingIRQ(TIMER1_IRQn);
	CMSDK_timer_Init(CMSDK_TIMER1,(uint32_t)(APB_Clock_Freq / 1000 * Int_Period) , true); // 1ms 
	NVIC_EnableIRQ(TIMER1_IRQn);	
}

//SETUP_x: 双定时器模式，自由运行模式，周期定定时器模式，单词定时器模式
//Int_Period: 定时时间，单位ms
//PRESCALE_x: 分频系数，1分频，16分频，256分频
//irq_en: 中断使能
//SIZE_x: 定时器大小，16位，32位
void DUALTIMER1_Init(DUALTIMER_MODE SETUP_x,uint32_t Int_Period, DUALTIMER_PRESCALE PRESCALE_x,  FunctionalState irq_en, DUALTIMER_SIZE SIZE_x ) //形参，输入中断触发周期 （单位ms） 
{
	PCLK_Enable(DUAL_TIMER_PCLK_EN);	
	NVIC_DisableIRQ(DUALTIMER_IRQn);
	NVIC_ClearPendingIRQ(DUALTIMER_IRQn);
	
	if(SETUP_x == SETUP_freerunning)
	CMSDK_dualtimer_setup_freerunning(CMSDK_DUALTIMER1,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	else if(SETUP_x == SETUP_periodic)
	CMSDK_dualtimer_setup_periodic(CMSDK_DUALTIMER1,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	else if(SETUP_x == SETUP_oneshot)
	CMSDK_dualtimer_setup_oneshot(CMSDK_DUALTIMER1,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	NVIC_EnableIRQ(DUALTIMER_IRQn);
}
void DUALTIMER2_Init(DUALTIMER_MODE SETUP_x,uint32_t Int_Period, DUALTIMER_PRESCALE PRESCALE_x,  FunctionalState irq_en, DUALTIMER_SIZE SIZE_x ) //形参，输入中断触发周期 （单位ms） 
{
	PCLK_Enable(DUAL_TIMER_PCLK_EN);	
	NVIC_DisableIRQ(DUALTIMER_IRQn);
	NVIC_ClearPendingIRQ(DUALTIMER_IRQn);
	
	if(SETUP_x == SETUP_freerunning)
	CMSDK_dualtimer_setup_freerunning(CMSDK_DUALTIMER2,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	else if(SETUP_x == SETUP_periodic)
	CMSDK_dualtimer_setup_periodic(CMSDK_DUALTIMER2,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	else if(SETUP_x == SETUP_oneshot)
	CMSDK_dualtimer_setup_oneshot(CMSDK_DUALTIMER2,(uint32_t)(APB_Clock_Freq / 1000 *Int_Period),PRESCALE_x,irq_en,SIZE_x);
	NVIC_EnableIRQ(DUALTIMER_IRQn);
}


/*使能定时器中断*/
 void CMSDK_timer_EnableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL |= CMSDK_TIMER_CTRL_IRQEN_Msk;
 }

/*关闭定时器中断*/
 void CMSDK_timer_DisableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL &= ~CMSDK_TIMER_CTRL_IRQEN_Msk;
 }

/*定时器启动*/
 void CMSDK_timer_StartTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL |= CMSDK_TIMER_CTRL_EN_Msk;
 }

/*定时器关闭*/
 void CMSDK_timer_StopTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL &= ~CMSDK_TIMER_CTRL_EN_Msk;
 }

/*获取定时器Value*/
 uint32_t CMSDK_timer_GetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->VALUE;
 }

/*设置定时器值*/
 void CMSDK_timer_SetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value)
 {
       CMSDK_TIMER->VALUE = value;
 }

/*获取重载值*/
 uint32_t CMSDK_timer_GetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->RELOAD;
 }

/*设置重载值*/
 void CMSDK_timer_SetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value)
 {
       CMSDK_TIMER->RELOAD = value;
 }


 void CMSDK_timer_ClearIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;
 }

/*返回定时器状态*/
 uint32_t  CMSDK_timer_StatusIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->INTSTATUS;
 }

/*初始化定时器*/
void CMSDK_timer_Init(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload, bool irq_en)
{     
	uint32_t new_ctrl = 0;
    CMSDK_TIMER->VALUE = reload;
    CMSDK_TIMER->RELOAD = reload;
    if (irq_en!=0)   
		new_ctrl |= CMSDK_TIMER_CTRL_IRQEN_Msk;               /* non zero - enable IRQ */
    new_ctrl |= CMSDK_TIMER_CTRL_EN_Msk;                      /* enable timer */
    CMSDK_TIMER->CTRL = new_ctrl;
}

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param reload The value to which the timer is to be set after an underflow has occurred
 * @param irq_en Defines whether the timer IRQ is to be enabled
 * @return none
 * @brief  Initialises the timer to use the external clock and specifies the timer reload value and whether IRQ is enabled or not.
 */

void CMSDK_timer_Init_ExtClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,uint32_t irq_en)
{
       CMSDK_TIMER->CTRL = 0;
       CMSDK_TIMER->VALUE = reload;
       CMSDK_TIMER->RELOAD = reload;
       if (irq_en!=0)                                                                                  /* non zero - enable IRQ */
            CMSDK_TIMER->CTRL = (CMSDK_TIMER_CTRL_IRQEN_Msk |
                                   CMSDK_TIMER_CTRL_SELEXTCLK_Msk |CMSDK_TIMER_CTRL_EN_Msk);
       else  {                                                                                         /* zero - do not enable IRQ */
            CMSDK_TIMER->CTRL = ( CMSDK_TIMER_CTRL_EN_Msk |
                                    CMSDK_TIMER_CTRL_SELEXTCLK_Msk);                                   /* enable timer */
         }
 }

/**
 *
 * @brief  Initialises the timer to use the internal clock but with an external enable. It also specifies the timer reload value and whether IRQ is enabled or not.
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param reload The value to which the timer is to be set after an underflow has occurred
 * @param irq_en Defines whether the timer IRQ is to be enabled
 * @return none
 * Timer 0 only
 *
 */
void CMSDK_timer_Init_ExtEnable(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,uint32_t irq_en)
{
    CMSDK_TIMER->CTRL = 0;
	CMSDK_TIMER->VALUE = reload;
	CMSDK_TIMER->RELOAD = reload;
    if (irq_en!=0)                                                                                  /* non zero - enable IRQ */
		CMSDK_TIMER->CTRL = (CMSDK_TIMER_CTRL_IRQEN_Msk | CMSDK_TIMER_CTRL_SELEXTEN_Msk | CMSDK_TIMER_CTRL_EN_Msk);
    else{                                                                                         /* zero - do not enable IRQ */
		CMSDK_TIMER->CTRL = ( CMSDK_TIMER_CTRL_EN_Msk | CMSDK_TIMER_CTRL_SELEXTEN_Msk);                                    /* enable timer */
    }
}

 /*DUAL Timer driver functions*/
/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *
 * @brief  Start timer in dual timers.
 */
 /* Start Timer */
 void CMSDK_dualtimer_start(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx)
 {
  CMSDK_DUALTIMERx->TimerControl |= CMSDK_DUALTIMER_CTRL_EN_Msk;
 }

/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *
 * @brief  Stop timer in dual timers.
 */

 /* Stop Timer */
 void CMSDK_dualtimer_stop(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx)
 {
  CMSDK_DUALTIMERx->TimerControl &= ~CMSDK_DUALTIMER_CTRL_EN_Msk;
 }

/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *
 * @brief  Clear the interrupt request in dual timers.
 */

 /* Clear the Interrupt */
 void CMSDK_dualtimer_irq_clear(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx)
 {
  CMSDK_DUALTIMERx->TimerIntClr = 0;
 }

/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *定时时间，预分频器0-1，1-16，23-256，中断启用位0-disable,1-enable，定时器位数0-16，1-32
 * @brief  Setup Free running mode in dual timers.
 */

 /* Free running timer mode */
 void CMSDK_dualtimer_setup_freerunning(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
													unsigned int cycle, unsigned int prescale, 
																unsigned int interrupt, unsigned int size)
 {
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
  /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  CMSDK_dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
             (size      & 0x1) << CMSDK_DUALTIMER_CTRL_SIZE_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
 }

/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *
 * @brief  Setup Periodic mode in dual timers.
 */

 /* Periodic timer mode */
 void CMSDK_dualtimer_setup_periodic(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
														unsigned int cycle, unsigned int prescale, 
															       unsigned int interrupt, unsigned int size)
 {
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
  /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  CMSDK_dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
             (size      & 0x1) << CMSDK_DUALTIMER_CTRL_SIZE_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_MODE_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
 }

/**
 *
 * @param *CMSDK_DUALTIMER DUAL Timer Pointer
 * @return none
 *
 * @brief  Setup One shot mode in dual timers.
 */

 /* One shot timer mode */
 void CMSDK_dualtimer_setup_oneshot(CMSDK_DUALTIMER_TypeDef *CMSDK_DUALTIMERx,
											 unsigned int cycle, unsigned int prescale, 
												      unsigned int interrupt, unsigned int size)
 {
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
  /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  CMSDK_dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
             (size      & 0x1) << CMSDK_DUALTIMER_CTRL_SIZE_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_ONESHOT_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
 }


 /*RTC driver functions*/

/**
 *
 * @param *CMSDK_RTC RTC Pointer
 * @return none
 *
 * @brief Initialize RTC Calender
 */
 void CMSDK_RTC_Init_Calender(uint16_t prescaler, uint8_t data_mode, uint8_t hour_mode, 
	                                                         uint32_t init_time, uint32_t init_date)
 {
	uint32_t new_reg_ctrl = 0;
	//wait for rtc prescaler sync ready
	while(!((CMSDK_RTC->SR & CMSDK_RTC_PRES_SYNC_READY_Msk) >> CMSDK_RTC_PRES_SYNC_READY_Pos));
	//set prescaler
	CMSDK_RTC->PR = prescaler;	
	//config data/hour mode
	if(data_mode != 0)      new_reg_ctrl |= CMSDK_RTC_DATA_MODE_Msk;
	if(hour_mode != 0)      new_reg_ctrl |= CMSDK_RTC_HOUR_MODE_Msk;
	CMSDK_RTC->CR = new_reg_ctrl;
	//wait for init sync ready
	while(!(CMSDK_RTC->SR & CMSDK_RTC_INIT_SYNC_READY_Msk));
	//set INIT
	CMSDK_RTC->CR |= CMSDK_RTC_INIT_EN_Msk;	
	//Set Initial Time & Date
	CMSDK_RTC->TR = init_time;
	CMSDK_RTC->DR = init_date;
	//Clear INIT bit
	CMSDK_RTC->CR &= ~CMSDK_RTC_INIT_EN_Msk;
	//wait for init sync ready
	while(!(CMSDK_RTC->SR & CMSDK_RTC_INIT_SYNC_READY_Msk));

	return;                                                                               	
 }
/**
 *
 * @param *CMSDK_RTC RTC Pointer
 * @return none
 *
 * @brief Config RTC Alarm
 */
 void CMSDK_RTC_Config_Alarm(uint16_t prescaler, uint8_t data_mode, uint8_t hour_mode, 
	                                        uint32_t init_time, uint32_t init_date, uint32_t alarm_time, 
                                                        uint32_t alarm_date)
 {
	uint32_t new_reg_ctrl = 0;
	//wait for rtc prescaler sync ready
	while(!((CMSDK_RTC->SR & CMSDK_RTC_PRES_SYNC_READY_Msk) >> CMSDK_RTC_PRES_SYNC_READY_Pos));
	//set prescaler
	CMSDK_RTC->PR = prescaler;		
	//config data/hour mode
	if(data_mode != 0)      new_reg_ctrl |=  CMSDK_RTC_DATA_MODE_Msk;
	if(hour_mode != 0)      new_reg_ctrl |= CMSDK_RTC_HOUR_MODE_Msk;
	CMSDK_RTC->CR = new_reg_ctrl;
	//wait for init sync ready
	while(!(CMSDK_RTC->SR & CMSDK_RTC_INIT_SYNC_READY_Msk));
	//set INIT
	CMSDK_RTC->CR |= CMSDK_RTC_INIT_EN_Msk;	
	//Set Initial Time & Date
	CMSDK_RTC->TR = init_time;
	CMSDK_RTC->DR = init_date;
	//Set Alarm Time & Date
	CMSDK_RTC->TAR = alarm_time;
	CMSDK_RTC->DAR = alarm_date;
	//Alarm enabled
	CMSDK_RTC->CR |= CMSDK_RTC_ALARM_EN_Msk;
	//Clear INIT bit
	CMSDK_RTC->CR &= ~CMSDK_RTC_INIT_EN_Msk;

	return;                                                                               	
 }
/**
 *
 * @param *CMSDK_RTC RTC Pointer
 * @return none
 *
 * @brief Config Period Wakeup
 */
 void CMSDK_RTC_Config_PeriodWake(uint8_t clock_sel, uint16_t prescaler, uint16_t period_time)
 {
    if(clock_sel) {
	CMSDK_RTC->CR |= CMSDK_RTC_WUT_CLK_EN_Msk;
	//wait for rtc prescaler sync ready
	while(!((CMSDK_RTC->SR & CMSDK_RTC_PRES_SYNC_READY_Msk) >> CMSDK_RTC_PRES_SYNC_READY_Pos));
	//set prescaler
	CMSDK_RTC->PR = prescaler;
    }
    else {
	CMSDK_RTC->CR &= ~CMSDK_RTC_WUT_CLK_EN_Msk;
	//wait for wut prescaler sync ready
	while(!((CMSDK_RTC->SR & CMSDK_RTC_WUT_PRES_SYNC_READY_Msk) >> CMSDK_RTC_WUT_PRES_SYNC_READY_Pos));
	//set prescaler
	CMSDK_RTC->WPR = prescaler;
    }	
    //wait for wut value sync ready
    while(!((CMSDK_RTC->SR & CMSDK_RTC_WUT_VAL_SYNC_READY_Msk) >> CMSDK_RTC_WUT_VAL_SYNC_READY_Pos));	
    //set Wakeup time register
    CMSDK_RTC->WTR = period_time;
    //Config CR with Periodic Wakeup timer enabled
    CMSDK_RTC->CR |= CMSDK_RTC_WUT_EN_Msk;	 
    return;                                                                	
 }












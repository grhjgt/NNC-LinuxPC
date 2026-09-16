/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_CURRENT_CALIBRATION.H        
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

#ifndef ENS_CURRENT_CALIBRATION_H
#define ENS_CURRENT_CALIBRATION_H

#include "my_header.h"
#include "ENS1_TIMER.h"
#define PI 3.1415
extern volatile uint8_t statics_config;  //通道状态标志，[3:0] 每一位对应一个通道的状态 0：关闭中 1：开启状态 
extern uint8_t  unit_current[4]; 
extern volatile uint32_t NUM_OF_PULSES_THRESHOLD[4];
extern volatile uint32_t wave_gen_irq_occurred[4];
/* --------------------------电刺激有关的参数以及通道状态--------------------------- */
/*
1、基本参数：
·基础波形类型
·正向脉宽：
·负向脉宽：
·波形频率：
·电流大小：
·延迟输出时间
2、复杂波形参数
·中断点设置（每次可以设置两个点，且第1点需要小于第2点）
·交替波形频率：
·总输出时间设置
·脉冲群中的脉冲个数
·脉冲群与脉冲群的间隔发生时间
*/
typedef enum {SQUARE_WAVE = 0 ,SINE_WAVE = 1, TRIANGULAR_WAVE = 2 }BasicWaveformType;

typedef struct COMPLEX_WAVEFORM_PARA
{
	uint16_t                 AlternatingFreq_HZ;	
	uint32_t                TotalOutputTime_S;
	uint32_t                NumOfPulseGroups;
	uint32_t                TimeOfPulseGroups_MS;
}STRUCT_COMPLEX_WAVEFORM_PARA;

typedef struct WAVEFORM_PARA
{
	BasicWaveformType    			Type;
	uint32_t             			PositivePulseWidth;
	uint32_t             			NegativePulseWidth;
    //uint32_t             		    WaveformFreq;    
	uint32_t                        ClientTime;       
	uint8_t              			DeadTime;
	uint32_t                        DelayOutputTime_US;
	STRUCT_COMPLEX_WAVEFORM_PARA    OtherWaveformPara;
}STRUCT_WAVEFORM_PARA;

extern STRUCT_WAVEFORM_PARA ParaSet_waveform[4];

/* ------------------------------------------------------------------------- */

/*--------------------------------波形设置的函数-----------------------------*/
typedef enum ENS_DRV_CONFIG 
{
	DISABLE_ALL_BIT = -1,
	REST_BIT = 1,
	NEGATIVE_BIT =2,
	SILENT_BIT =4,
	SOURCE_B_BIT = 8,
	ALTERNATING_POSITIVE_BIT =16,
	CONTINUE_REPEATING_BIT = 32,
	MULTI_ELECTRODE_BIT = 64 ,
	ENABLE_ALL_BIT = 0X7F
}DRV_CONFIG;

/*获取到FT测试后的实际测量单元电流值 */
extern uint8_t GET_FT_CURRENT(void);
/*波形参数的设置*/
extern uint32_t	StimulatorInit(CHANNEL_NUM CHANNEL_X);
/*输出电流 / 频率*/
extern uint8_t CURRENT_AMPLITUDE_MODIFY(CHANNEL_NUM CHANNEL_X , float mA);
extern uint32_t CURRENT_FREQ_MODIFY(CHANNEL_NUM CHANNEL_X , uint32_t freq);
/*电刺激时间*/
extern uint32_t StimuTimeCount_S(CHANNEL_NUM CHANNEL_X , TIME_COUNT_MODE MODE);
/*启动停止*/
void StartStimulatorOut(CHANNEL_NUM CHANNEL_X);
void StopStimulatorOut(CHANNEL_NUM CHANNEL_X);


#endif 

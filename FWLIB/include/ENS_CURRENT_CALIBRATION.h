#ifndef ENS_CURRENT_CALIBRATION_H
#define ENS_CURRENT_CALIBRATION_H

#include "my_header.h"
#include "CMSDK_CM0.h"
#include "ENS1_TIMER.h"

#define PI 3.1415

extern volatile uint8_t statics_config;  // ͨ��״̬��־λ[3:0]
extern uint8_t  unit_current[4]; 
extern uint8_t  CUSTOM_UNIT_CURRENT[4];
extern uint16_t unit_t[4];
extern uint32_t isel_t[4][64];

extern volatile uint32_t TOTAL_TIME_THRESHOLD_VALUE[4];
extern volatile uint32_t TRIGGER_TIME_COUNT[4];
extern volatile uint32_t TRIGGER_TIME_THRESHOLD_VALUE[4];
extern volatile uint32_t CHANNEL_TIME_COUNT[4];
extern volatile uint8_t  TRIGGER_TIME_COUNT_FLAG;
extern volatile uint32_t NUM_OF_PULSES_THRESHOLD[4];
extern volatile uint32_t wave_gen_irq_occurred[4];

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
	uint32_t                        ClientTime;       
	uint8_t              			DeadTime;
	uint32_t                        DelayOutputTime_US;
	STRUCT_COMPLEX_WAVEFORM_PARA    OtherWaveformPara;
}STRUCT_WAVEFORM_PARA;

extern STRUCT_WAVEFORM_PARA ParaSet_waveform[4];

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

extern uint32_t	StimulatorInit(CHANNEL_NUM CHANNEL_X);
extern uint8_t CURRENT_AMPLITUDE_MODIFY(CHANNEL_NUM CHANNEL_X , float mA);
extern uint32_t CURRENT_FREQ_MODIFY(CHANNEL_NUM CHANNEL_X , uint32_t freq);
extern uint32_t StimuTimeCount_S(CHANNEL_NUM CHANNEL_X , TIME_COUNT_MODE MODE);
void StartStimulatorOut(CHANNEL_NUM CHANNEL_X);
void StopStimulatorOut(CHANNEL_NUM CHANNEL_X);

#endif

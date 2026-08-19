#ifndef MY_HEADER_H
#define MY_HEADER_H


#include <stdio.h>
#include <math.h>
#include "CMSDK_CM0.h"


#define TRUE 1
#define FALSE 0


typedef enum
{
    true=1, false=0
}bool;
typedef enum {CHANNEL_0 = 0,CHANNEL_1,CHANNEL_2,CHANNEL_3} CHANNEL_NUM;
typedef enum {ALT_FUNC0=0,ALT_FUNC1=1,ALT_FUNC2=2,ALT_FUNC3=3  } GPIO_AltFuncSel;
typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus ,IOStatus;
typedef enum {DISABLE = 0, ENABLE } FunctionalState;
typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;
typedef enum {LOW_LEVEL = 0, HIGH_LEVEL = !LOW_LEVEL} LEVELStatus;
typedef enum {OUTPUT = 0, INPUT = !OUTPUT} I_O_SELECT;
typedef enum {oversamp_16 = 0, oversamp_13=!oversamp_16} OverSampModeSel;
typedef enum {
	GPIO_0=0,
	GPIO_1=1,
	GPIO_2=2,
	GPIO_3=3,
	GPIO_4=4,
	GPIO_5=5,
	GPIO_6=6,
	GPIO_7=7,
	GPIO_8=8,
	GPIO_9=9,
	GPIO_10=10,
	GPIO_11=11,
	GPIO_12=12,
	GPIO_13=13,
	GPIO_14=14,
	GPIO_15=15,
	GPIO_16=16,
	GPIO_17=17,
	GPIO_18=18,
	GPIO_19=19,
	GPIO_20=20,
	GPIO_21=21,
	GPIO_22=22,
	GPIO_23=23,
}GPIO_NUM;



extern uint8_t rev_buf[16];
extern uint8_t rev_len;
extern uint8_t rev_fin ;

#endif

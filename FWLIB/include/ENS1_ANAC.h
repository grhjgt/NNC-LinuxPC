/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_ANAC.H        
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

#ifndef ENS1_ANAC_H
#define ENS1_ANAC_H

#include <stdio.h>
#include "CMSDK_CM0.h"
#include "my_header.h"
#include "ENS1_GPIO.h"
/*-------------------boost------------------------*/
extern uint8_t Boost_Voltage_Sel(uint8_t VOLTAGE_XV);
#define   VOLTAGE_11V  0x0
#define   VOLTAGE_15V  0x1
#define   VOLTAGE_26V  0x2
#define   VOLTAGE_45V  0x3
#define   VOLTAGE_55V  0x4


/*-------------------比较器------------------------*/
/*
比较器IO复用说明

GPIO12 -- COMP1_VIP0
GPIO13 -- COMP1_VIP1
GPIO14 -- COMP1_VIN0
GPIO15 -- COMP1_VIN1
GPIO21 -- COMP1_OUT

GPIO8 -- COMP0_VIP0
GPIO9 -- COMP0_VIP1
GPIO10 -- COMP0_VIN0
GPIO11 -- COMP0_VIN1
GPIO20 -- COMP0_OUT
*/

#define  COMP_VREF_VOLTAGE_0_3v (uint8_t)(0x0) 
#define  COMP_VREF_VOLTAGE_0_6v (uint8_t)(0x1) 
#define  COMP_VREF_VOLTAGE_0_9v (uint8_t)(0x2) 
#define  COMP_VREF_VOLTAGE_1_2v (uint8_t)(0x3) 
#define  COMP_VREF_VOLTAGE_1_5v (uint8_t)(0x4) 
#define  COMP_VREF_VOLTAGE_1_8v (uint8_t)(0x5) 
#define  COMP_VREF_VOLTAGE_2_1v (uint8_t)(0x6) 
#define  COMP_VREF_VOLTAGE_2_4v (uint8_t)(0x7) 

#define  COMP_NEG_INPUT_VIN0  (uint8_t)(0x0) 
#define  COMP_NEG_INPUT_VIN1  (uint8_t)(0x1) 
#define  COMP_NEG_INPUT_VREF  (uint8_t)(0x2) 

#define  COMP_POS_INPUT_VIP0  (uint8_t)(0x1) 
#define  COMP_POS_INPUT_VIP1  (uint8_t)(0x2) 

typedef enum{COMP0=0, COMP1}COMP_NUM;
typedef struct COMPConfig
{	
	COMP_NUM COMPARATOR_NUM;
	uint8_t COMP_VREF_SEL;
	uint8_t COMP_SIGSEL_NEGATIVE_INPUT_SEL;
	uint8_t COMP_SIGSEL_POSITIVE_INPUT_SEL;
}COMP_ConfigStructure;


/*比较器的输出设置*/
extern void CompInitSet(COMP_ConfigStructure* COMPCONFIG);

/*读比较结果*/
extern uint8_t Read_Comp_Output(COMP_NUM NUM);

/*开启/关闭比较器*/
extern void CompControl(COMP_NUM NUM ,FunctionalState Newstate);


/*-----------------------------PGA------------------------------*/

/*
PGA IO复用说明
GPIO16 -- PGA_VIP0   //输入信号直连
GPIO18 -- PGA_VIN0   //需要接入电阻

GPIO17 -- PGA_VIP1   //输入信号直连
GPIO19 -- PGA_VIN1   //需要接入电阻
          
GPIO20 -- PGA_EXVCM

GPIO4 -- PGA_OUT     

*/

//#define PGA_POSITIVE_INPUT_VBAT             (uint8_t)(0x3)
//#define PGA_POSITIVE_INPUT_AVDD1P8          (uint8_t)(0x4)
//#define PGA_POSITIVE_INPUT_VLCD0            (uint8_t)(0x5)
//#define PGA_POSITIVE_INPUT_V_TEMP           (uint8_t)(0x7)   


//PGA放大倍数选择 ，对于反相模式： 1-8 倍， 同相模式2-9倍
#define PGA_GAIN_1X_2X    (uint8_t)(0x0)
#define PGA_GAIN_2X_3X    (uint8_t)(0x1)
#define PGA_GAIN_3X_4X    (uint8_t)(0x2)
#define PGA_GAIN_4X_5X    (uint8_t)(0x3)
#define PGA_GAIN_5X_6X    (uint8_t)(0x4)
#define PGA_GAIN_6X_7X    (uint8_t)(0x5)
#define PGA_GAIN_7X_8X    (uint8_t)(0x6)
#define PGA_GAIN_8X_9X    (uint8_t)(0x7)


typedef enum{PGA_NEGATIVE_INPUT_PGA_VIN0 = 0,PGA_NEGATIVE_INPUT_PGA_VIN1, PGA_NEGATIVE_INPUT_INTERNAL_VCM,PGA_NEGATIVE_INPUT_EXTERNAL_VCM}PGA_NEG_SEL;
typedef enum{PGA_POSITIVE_INPUT_PGA_VIP0 = 0,PGA_POSITIVE_INPUT_PGA_VIP1=1,PGA_POSITIVE_INPUT_INTERNAL_VCM=2,PGA_POSITIVE_INPUT_VREF1_2V=6}PGA_POS_SEL;
typedef enum{PGA_TO_GPIO4=0 , PGA_TO_ADC=1}PGA_OUTPUT_SEL;
typedef struct PGAConfig
{
	PGA_NEG_SEL PGA_NEG_SELx      ; 
	PGA_POS_SEL PGA_POS_SELx      ;
	uint8_t PGA_GAIN_SEL          ; 
	PGA_OUTPUT_SEL PGA_OUT        ;
}PGA_ConfigStructure;

//初始化
extern void PGAInitSet(PGA_ConfigStructure* PGACONFIG );

//PGA开关控制
extern void PGAControl(FunctionalState Newstate);


/*--------------------------低电压与芯片过温检测-------------------------*/
void PMU_TEMP150C_TRIGControl(FunctionalState Newstate);  //过温检测功能开启与关闭

//返回温度是否超过150度的状态值
int8_t TEMP_150C_TRIG_SIGNAL(void);



#define LVD_4P2V  (uint8_t)(0x0)
#define LVD_3P9V  (uint8_t)(0x1)
#define LVD_3P6V  (uint8_t)(0x2)
#define LVD_3P3V  (uint8_t)(0x3)
#define LVD_3P0V  (uint8_t)(0x4)
#define LVD_2P7V  (uint8_t)(0x5)
#define LVD_2P4V  (uint8_t)(0x6)
#define LVD_2P1V  (uint8_t)(0x7)
//低电压告警
//设置阈值电压
void LVD_InitSet(uint8_t Threshold_voltage);
//打开或关闭此功能
void PMU_LVD_Control(FunctionalState Newstate);
//读取低电压状态
uint8_t Read_LVD_Signal(void);

/*-------------------------带隙BUFFER启用或关闭-------------------------*/
void PMU_BANDGAP_CONTROL(FunctionalState Newstate);



#endif

 









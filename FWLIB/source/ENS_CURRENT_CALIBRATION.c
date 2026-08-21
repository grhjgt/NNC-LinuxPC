#include "ENS_CURRENT_CALIBRATION.h"
#include "ENS1_MTP.h"
#include "ENS1_TIMER.h"
#include <math.h>

/* ��ʱ���벨�δ�����������ֵ���� */
volatile uint32_t TOTAL_TIME_THRESHOLD_VALUE[4] = {0, 0, 0, 0};
volatile uint32_t TRIGGER_TIME_COUNT[4] = {0, 0, 0, 0};
volatile uint32_t TRIGGER_TIME_THRESHOLD_VALUE[4] = {0, 0, 0, 0};
volatile uint32_t CHANNEL_TIME_COUNT[4] = {0, 0, 0, 0};
volatile uint8_t  TRIGGER_TIME_COUNT_FLAG = 0;

/* ���β�����ʼ�����ýṹ������ */
STRUCT_WAVEFORM_PARA ParaSet_waveform[4] =
{	
	/*ͨ��1 -- ST0 ST1 */
	{	
		.Type = SQUARE_WAVE ,           			//��������ѡ��
		.PositivePulseWidth = 100  ,      				//������������-΢��
		.DeadTime   = 10,                  				//����ʱ��-΢��
		.NegativePulseWidth = 100 ,       				//������������-΢��
		.ClientTime = 1000 ,              					//��Ĭʱ��-΢��
		.DelayOutputTime_US = 0,          				//�ӳ����ʱ��-΢��
		.OtherWaveformPara.AlternatingFreq_HZ = 0,      //���沨��Ƶ��
		.OtherWaveformPara.TotalOutputTime_S = 0,       //���������ʱ�� ��
		.OtherWaveformPara.NumOfPulseGroups  = 0,       //����Ⱥģʽ�µ�ÿ����������
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   	//����Ⱥģʽ�µ�����ʱ�� ����
	},
	/*ͨ��2 -- ST2 ST3 */
	{
		.Type = TRIANGULAR_WAVE ,          					//��������ѡ��
		.PositivePulseWidth = 0  ,      					//������������-΢��
		.DeadTime   = 0,                  					//����ʱ��-΢��
		.NegativePulseWidth = 0 ,       					//������������-΢��
		.ClientTime = 0 ,              						//��Ĭʱ��-΢��
		.DelayOutputTime_US = 0,          					//�ӳ����ʱ��-΢��
		.OtherWaveformPara.AlternatingFreq_HZ = 0,          //���沨��Ƶ��
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//���������ʱ�� ��
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//����Ⱥģʽ�µ�ÿ����������
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   		//����Ⱥģʽ�µ�����ʱ�� ����
	},
	/*ͨ��3 -- ST4 ST5 */
	{
		.Type = TRIANGULAR_WAVE ,          					//��������ѡ��
		.PositivePulseWidth = 0  ,      					//������������-΢��
		.DeadTime   = 0,                  					//����ʱ��-΢��
		.NegativePulseWidth = 0 ,       					//������������-΢��
		.ClientTime = 0 ,              						//��Ĭʱ��-΢��
		.DelayOutputTime_US = 0,          					//�ӳ����ʱ��-΢��
		.OtherWaveformPara.AlternatingFreq_HZ = 0,        	//���沨��Ƶ��
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//���������ʱ�� ��
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//����Ⱥģʽ�µ�ÿ����������
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,  		//����Ⱥģʽ�µ�����ʱ�� ����
	},
	/*ͨ��4 -- ST6 ST7 */
	{
		.Type = TRIANGULAR_WAVE ,          					//��������ѡ��
		.PositivePulseWidth = 0  ,      					//������������-΢��
		.DeadTime   = 0,                  					//����ʱ��-΢��
		.NegativePulseWidth = 0 ,      					 	//������������-΢��
		.ClientTime = 0 ,              						//��Ĭʱ��-΢��
		.DelayOutputTime_US = 0,          					//�ӳ����ʱ��-΢��
		.OtherWaveformPara.AlternatingFreq_HZ = 0,          //���沨��Ƶ��
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//���������ʱ�� ��
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//����Ⱥģʽ�µ�ÿ����������
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   		//����Ⱥģʽ�µ�����ʱ�� ����
	},
};


/*-------------------------��ʱ��/���δ�����������ֵ����---------------------------*/
/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_CURRENT_CALIBRATION.C         
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description:  
*Function List: 
    
History: 
	1.V1.0
	Date:          
	Author: 
	Modification: 
*/ 
#include "ENS_CURRENT_CALIBRATION.h"
#include "ENS1_MTP.h"
#include "ENS1_UART.h"
#include "ENS1_TIMER.h"

/*-------------------------��ȡ��FT���Ժ��ʵ�ʲ�����Ԫ����ֵ---------------------------*/
static uint16_t FT_CURRENT_SAVE[4] = {0, 0, 0, 0};     // ���ڱ���� MTP ����������
uint8_t CUSTOM_UNIT_CURRENT[4] = {33, 33, 33, 33};     // �Զ���Ĭ�ϵ�Ԫ����ֵ
uint8_t unit_current[4] = {33, 33, 33, 33};            // ����ʵ����Ч�ĵ�Ԫ����ֵ (RAM)

uint8_t GET_FT_CURRENT(void)
{	
	uint16_t latest_ft[4] = {0};
	uint8_t found_valid = 0;
	
	// ɨ�� Sector 6 �� 32 ����λ (0x6F00 ~ 0x6FF8)��Ѱ������һ�����̵�У׼����
	for (int slot = 0; slot < 32; slot++) {
		uint32_t slot_addr = DATA_SAVE_ADDR + slot * 8;
		uint16_t f0 = 0, f1 = 0, f2 = 0, f3 = 0;
		flash_read(slot_addr, &f0);
		flash_read(slot_addr + 2, &f1);
		flash_read(slot_addr + 4, &f2);
		flash_read(slot_addr + 6, &f3);
		
		if (f0 != 0 && f0 != 0xFFFF && f1 != 0 && f1 != 0xFFFF) {
			latest_ft[0] = f0;
			latest_ft[1] = f1;
			latest_ft[2] = f2;
			latest_ft[3] = f3;
			found_valid = 1;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (found_valid && latest_ft[i] != 0 && latest_ft[i] != 0xFFFF)
		{
			FT_CURRENT_SAVE[i] = latest_ft[i];
			unit_current[i] = (uint8_t)(latest_ft[i] / 832); // ����ʵ�ʵ�Ԫ���� uA
		}
		else 
		{
			unit_current[i] = CUSTOM_UNIT_CURRENT[i]; // δУ׼�����̬�����˵�Ĭ�� 33uA
		}
	}
	return 0;
}

/*-------------------------------------��--------------------------------*/
//??
double sin_cal(uint8_t angle)  //0-180
{	
	double radian = angle*PI/180;
	return sin(radian);
}

//????64??��??
uint16_t unit_t[4]; //???0-7  ??
uint32_t isel_t[4][64]; //??��0-255??
BasicWaveformType  wavePara_type[4];
uint8_t cal_output_current_mA(CHANNEL_NUM CHANNEL_X,float mA,BasicWaveformType type, uint16_t* unit , uint32_t (*isel)[64])   // uint16_t read_current_data  
{	
	CMSDK_WAVE_GEN_TypeDef*         WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0)      WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK =             WAVE_GEN_DRVA_BLK3;
	(void)WAVE_GEN_BLK;
	
	if(!mA)
	{	
		for(uint8_t i = 0 ;i<4;i++){
			unit_t[i] = 0;
			for(uint8_t j = 0;j<64;j++)
			{
				isel_t[i][j] = 0;
			}
		}
	}
	else
	{
		if(type == SQUARE_WAVE)   
		{
			//,?????
			*(unit+CHANNEL_X) = (uint16_t)(mA * 1000 / (unit_current[CHANNEL_X] * 255));                    // 0 - 7
			for(int i = 0 ;i<64;i++)
			{
				*(isel[0]+CHANNEL_X*64+i) = (uint16_t)(mA * 1000 / ((*(unit+CHANNEL_X)+1)*unit_current[CHANNEL_X]));      // 0 - 255		
			}
		}
		else if(type== SINE_WAVE)
		{	
			//,?????
			*(unit+CHANNEL_X) = (uint16_t)(mA * 1000 / (unit_current[CHANNEL_X] * 255));                    // 0 - 7
			for(int i = 0 ;i<64;i++)
			{
				*(isel[0]+CHANNEL_X*64+i) = (uint16_t)(mA * 1000 / ((*(unit+CHANNEL_X) + 1)*unit_current[CHANNEL_X])*sin_cal(i*2.85714));      // 0 - 255		
			}
		}	
		else //??(type == TRIANGULAR_WAVE )
		{
			//,?????
				
				*(unit+CHANNEL_X) = (uint16_t)(mA * 1000 / (unit_current[CHANNEL_X] * 255));                    // 0 - 7
				*(isel[0]+CHANNEL_X*64) = 0;
				*(isel[0]+CHANNEL_X*64+63) = 0;
				for(int j = 1 ; j<32 ; j++)
				{
					*(isel[0]+CHANNEL_X*64+j) = (uint16_t)(mA * 1000 / ((*(unit+CHANNEL_X)+1)*unit_current[CHANNEL_X]) * 0.033258*j);      // 0 - 255	
				//	printf("%d\n",*(isel[0]+CHANNEL_X*64+j));
				}
				for(int k = 32 ; k<63 ; k++)
				{
					*(isel[0]+CHANNEL_X*64+k) =*(isel[0]+CHANNEL_X*64+(63-k))  ;
				//	printf("%d\n",*(isel[0]+CHANNEL_X*64+k));					
				}	
		}
	}
	return 0; 	
}

/*?*/
/*CONFIG?*/
uint8_t ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_NUM CHANNEL_X , DRV_CONFIG BIT)
{	
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;
    if(BIT == DISABLE_ALL_BIT)
	{
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG = 0x00;
		return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
	}
	if(BIT == ENABLE_ALL_BIT)
	{
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG = ENABLE_ALL_BIT;
		return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
	}
	if((BIT != DISABLE_ALL_BIT) && (BIT != ENABLE_ALL_BIT))
	{		
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG |= BIT;
	}
	return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
}

uint8_t ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_NUM CHANNEL_X , DRV_CONFIG BIT)
{	
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;
    if(BIT == DISABLE_ALL_BIT)
	{
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG = 0x00;
		return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
	}
	if(BIT == ENABLE_ALL_BIT)
	{
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG = ENABLE_ALL_BIT;
		return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
	}
	if((BIT != DISABLE_ALL_BIT) && (BIT != ENABLE_ALL_BIT))
	{		
		WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG &=~ BIT;
	}
	return (uint8_t)WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG ;
}


/*---------------------------?��?????--------------------------------*/
uint32_t  StimulatorInit(CHANNEL_NUM CHANNEL_X)
{
	//uint32_t WaveformFreq=0;
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;
	//config 
	ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,DISABLE_ALL_BIT);
	//1  ???0��?sine ,?REST 
	if((ParaSet_waveform[CHANNEL_X].DeadTime > 0)){            
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,REST_BIT);
	}
	else
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,REST_BIT);
	}
	//2  0 ? NEGATIVE_BIT  SOURCE_B_BIT
	if(ParaSet_waveform[CHANNEL_X].NegativePulseWidth > 0)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,SOURCE_B_BIT );  //SOURCEB???��=0
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,NEGATIVE_BIT );		
	}
	//3????0SILENT_BIT
	if(ParaSet_waveform[CHANNEL_X].ClientTime>0)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,SILENT_BIT);
	}
	else
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,SILENT_BIT);
	}
	
	//4??ALTERNATING_POSITIVE_BIT
	if(ParaSet_waveform[CHANNEL_X].OtherWaveformPara.AlternatingFreq_HZ > 0 )
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,ALTERNATING_POSITIVE_BIT);
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,SOURCE_B_BIT);
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,NEGATIVE_BIT);
		//??��,????????????2^16
		WAVE_GEN_BLK ->WAVE_GEN_DRV_ALT_LIM_REG =(uint16_t)(ParaSet_waveform[CHANNEL_X].PositivePulseWidth / (uint16_t)ParaSet_waveform[CHANNEL_X].OtherWaveformPara.AlternatingFreq_HZ * (uint16_t)(APB_Clock_Freq/1000000)) ;
		ParaSet_waveform[CHANNEL_X].NegativePulseWidth = 0;
	}
	else 
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,ALTERNATING_POSITIVE_BIT);
	}
	
	ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,CONTINUE_REPEATING_BIT);	
	//??  ��us
	WAVE_GEN_BLK->WAVE_GEN_DRV_DELAY_LIM_REG  = (uint8_t)(APB_Clock_Freq/1000000)*ParaSet_waveform[CHANNEL_X].DelayOutputTime_US;
	//??STRUCT_WAVEFORM_PARA?????

	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG  =  ParaSet_waveform[CHANNEL_X].NegativePulseWidth;
	WAVE_GEN_BLK->WAVE_GEN_DRV_HLF_WAVE_PRD_REG      =  ParaSet_waveform[CHANNEL_X].PositivePulseWidth;
	WAVE_GEN_BLK->WAVE_GEN_DRV_CLK_FREQ_REG          =  (uint32_t)(APB_Clock_Freq/1000000);  //
	WAVE_GEN_BLK->WAVE_GEN_DRV_SILENT_T_REG          =  ParaSet_waveform[CHANNEL_X].ClientTime;
	WAVE_GEN_BLK->WAVE_GEN_DRV_REST_T_REG            =  ParaSet_waveform[CHANNEL_X].DeadTime;
	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_SCALE_REG         =  1;   //???0 ��0
	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_OFFSET_REG        =  0;
	
	//WaveformFreq = (uint32_t)(1000000 / (wavePara.ClientTime + wavePara.DeadTime +wavePara.NegativePulseWidth + wavePara.PositivePulseWidth));
	//�h
	wavePara_type[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].Type;
	
	TOTAL_TIME_THRESHOLD_VALUE[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TotalOutputTime_S * 1000;
	if( (ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TimeOfPulseGroups_MS > 0)  || (ParaSet_waveform[CHANNEL_X].OtherWaveformPara.NumOfPulseGroups >0))
	{
		TRIGGER_TIME_COUNT[CHANNEL_X] = 0;  //?	
		TRIGGER_TIME_THRESHOLD_VALUE[CHANNEL_X]=ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TimeOfPulseGroups_MS ;
		NUM_OF_PULSES_THRESHOLD[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].OtherWaveformPara.NumOfPulseGroups ;
		WAVE_GEN_BLK->WAVE_GEN_DRV_INT_REG               =  0x001f0001;
	}
	uint8_t returnValue = cal_output_current_mA(CHANNEL_X,0, wavePara_type[CHANNEL_X], unit_t , isel_t) ;
	
	return (uint32_t)(WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG);  //?��?
}

/*-------------------------------------------------------------------------------*/
volatile uint8_t statics_config=0;    //??��? 0 / 1 / 2 /4 / 8
void StartStimulatorOut(CHANNEL_NUM CHANNEL_X)
{
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;  statics_config |= (1<<0); }
	else if(CHANNEL_X == CHANNEL_1) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;  statics_config |= (1<<1);}
	else if(CHANNEL_X == CHANNEL_2) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;    statics_config |= (1<<2);}
	else {WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;     statics_config |= (1<<3) ;}
	if(statics_config > 1)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,MULTI_ELECTRODE_BIT);  //��?
	}
	WAVE_GEN_BLK->WAVE_GEN_DRV_CTRL_REG = 1;
}
/*--------------------------------------??----------------------------------------*/
void StopStimulatorOut(CHANNEL_NUM CHANNEL_X)
{
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0)  {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;   statics_config &=~(1<<0);   }
	else if(CHANNEL_X == CHANNEL_1) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;  statics_config &=~(1<<1);  }
	else if(CHANNEL_X == CHANNEL_2) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;   statics_config &=~(1<<2); }
	else {WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;      statics_config &=~(1<<3);  }
	WAVE_GEN_BLK->WAVE_GEN_DRV_CTRL_REG = 0;
	if(statics_config < 2)
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,MULTI_ELECTRODE_BIT);
	}
}

/*-----------------------------?��??-------------------------------*/
/*��?*/
uint8_t CURRENT_AMPLITUDE_MODIFY(CHANNEL_NUM CHANNEL_X , float mA)
{
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;
	
	NVIC_DisableIRQ(WG_DRV_IRQn);
	WAVE_GEN_BLK->WAVE_GEN_DRV_INT_REG =  0;
	StopStimulatorOut(CHANNEL_X);
	cal_output_current_mA(CHANNEL_X,mA, wavePara_type[CHANNEL_X], unit_t , isel_t) ;	
	WAVE_GEN_BLK ->WAVE_GEN_DRV_ISEL_REG = unit_t[CHANNEL_X];  //�� 0x00 - 0x07   ?
	for(int i=0; i<64; i++){
		WAVE_GEN_BLK->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i;  
		WAVE_GEN_BLK->WAVE_GEN_DRV_IN_WAVE_REG = isel_t[CHANNEL_X][i]; //0xff   ��
	}	
	WAVE_GEN_BLK->WAVE_GEN_DRV_INT_REG =  0x001f0001;
	NVIC_EnableIRQ(WG_DRV_IRQn);
	StartStimulatorOut(CHANNEL_X);
	return (uint8_t)(WAVE_GEN_BLK->WAVE_GEN_DRV_CTRL_REG); //??
}

/*????????=(1000000/( +  + ? + ??))*/
uint32_t CURRENT_FREQ_MODIFY(CHANNEL_NUM CHANNEL_X , uint32_t freq)
{
	uint32_t Pulse_Width = 0;
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;	
	if(freq > 50000 )
		freq = 50000;
	else if(freq < 10)
		freq = 10;	
	//??????0 ???????? ???0?/??
	if(ParaSet_waveform[CHANNEL_X].ClientTime > 0)
	{
		Pulse_Width/*??*/ = (uint32_t)((1000000/freq) - ParaSet_waveform[CHANNEL_X].DeadTime - ParaSet_waveform[CHANNEL_X].PositivePulseWidth - ParaSet_waveform[CHANNEL_X].NegativePulseWidth);
		WAVE_GEN_BLK->WAVE_GEN_DRV_SILENT_T_REG = Pulse_Width;
		return Pulse_Width;
	}	
	else if(ParaSet_waveform[CHANNEL_X].ClientTime == 0)
	{
		Pulse_Width/*+*/ = (uint32_t)((1000000/freq) - ParaSet_waveform[CHANNEL_X].DeadTime) ; 
		if(ParaSet_waveform[CHANNEL_X].NegativePulseWidth == 0)
			WAVE_GEN_BLK->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = Pulse_Width;
		else 
		{
			WAVE_GEN_BLK->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = (uint32_t) (Pulse_Width / 2) ;
			WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = (uint32_t) (Pulse_Width / 2) ; 
		}
		return Pulse_Width;
	}	
	else
	{
		return 0;
	}
}


//???

uint32_t StimuTimeCount_S(CHANNEL_NUM CHANNEL_X , TIME_COUNT_MODE MODE)
{
	uint32_t count_time = 0;
	if(MODE == TOTAL_TIME_MODE)
	{
		count_time = *(CHANNEL_TIME_COUNT+CHANNEL_X) ;
	}
	else          //MODE == TRIGGER_TIME_MODE  ?
	{
		count_time = *(TRIGGER_TIME_COUNT+CHANNEL_X);
	}
	return count_time;
}

//??��??��?�ǧ�?
uint32_t waveformOccurreCount(CHANNEL_NUM CHANNEL_X)
{
	return (uint32_t)(wave_gen_irq_occurred[CHANNEL_X]/4);
}

/*-------------------------------?��?-----------------------*/
volatile uint32_t NUM_OF_PULSES_THRESHOLD[4]={0,0,0,0};
volatile uint32_t wave_gen_irq_occurred[4]={0,0,0,0};
void  WG_DRV_Handler(void) 
{ 	
	
	CMSDK_WAVE_GEN_TypeDef *DRVA;
	for(int i =0; i<4;i++)
	{
		if(i==0){	DRVA= WAVE_GEN_DRVA_BLK0;	}
		else if(i==1){  DRVA= WAVE_GEN_DRVA_BLK1;	}
		else if(i==2){  DRVA= WAVE_GEN_DRVA_BLK2;	}
		else if(i==3){  DRVA= WAVE_GEN_DRVA_BLK3;	}

		if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_READ_DRIVER_NUM_Msk) == i) //��??��
		{
			if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_STS_Msk) == CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_STS_Msk)//?��??��
			{
				if(((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_READ_FIRST_ADDR_Msk)>>CMSDK_WAVE_GEN_DRV_INT_READ_FIRST_ADDR_Pos) == 0) 
				{
					if((NUM_OF_PULSES_THRESHOLD[i] > 0) && (wave_gen_irq_occurred[i] > (NUM_OF_PULSES_THRESHOLD[i]*4 - 2)))  
						DRVA->WAVE_GEN_DRV_INT_REG = (63 << CMSDK_WAVE_GEN_DRV_INT_SECOND_ADDR_Pos) | (0 << CMSDK_WAVE_GEN_DRV_INT_FIRST_ADDR_Pos) | CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_CLR_Msk | CMSDK_WAVE_GEN_DRV_INT_EN_Msk;
					else
				        DRVA->WAVE_GEN_DRV_INT_REG = (63 << CMSDK_WAVE_GEN_DRV_INT_SECOND_ADDR_Pos) | (32 << CMSDK_WAVE_GEN_DRV_INT_FIRST_ADDR_Pos) | CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_CLR_Msk | CMSDK_WAVE_GEN_DRV_INT_EN_Msk;
				}
				else if(((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_READ_FIRST_ADDR_Msk)>>CMSDK_WAVE_GEN_DRV_INT_READ_FIRST_ADDR_Pos) == 32) 
				{
						DRVA->WAVE_GEN_DRV_INT_REG = (31 << CMSDK_WAVE_GEN_DRV_INT_SECOND_ADDR_Pos) | (0 << CMSDK_WAVE_GEN_DRV_INT_FIRST_ADDR_Pos) | CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_CLR_Msk | CMSDK_WAVE_GEN_DRV_INT_EN_Msk;
				}
				if((NUM_OF_PULSES_THRESHOLD[i] > 0))  //???
					wave_gen_irq_occurred[i]++;
				
				
			}
			if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_STS_Msk) == CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_STS_Msk) 
			{   
				StopStimulatorOut((CHANNEL_NUM)i);
				DRVA->WAVE_GEN_DRV_INT_REG = CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_CLR_Msk | CMSDK_WAVE_GEN_DRV_INT_EN_Msk;
				NVIC_DisableIRQ(WG_DRV_IRQn);
				DRVA->WAVE_GEN_DRV_INT_REG =  0;
				TRIGGER_TIME_COUNT_FLAG |= (1<<i);  //??1
				wave_gen_irq_occurred[i] = 0;       //
			}
		}	
	}
}













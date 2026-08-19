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
	Modification: 初版
*/ 
#include "ENS_CURRENT_CALIBRATION.h"
#include "ENS1_MTP.h"
#include "ENS1_UART.h"
#include "ENS1_TIMER.h"

/*--------------------------------波形参数的初始化幅值结构体-----------------------------*/

STRUCT_WAVEFORM_PARA ParaSet_waveform[4] =
{	
	/*通道1 -- ST0 ST1 */
	{	
		.Type = SQUARE_WAVE ,           			//波形种类选择
		.PositivePulseWidth = 100  ,      				//正半周期脉宽-微秒
		.DeadTime   = 10,                  				//死区时间-微秒
		.NegativePulseWidth = 100 ,       				//负半周期脉宽-微秒
		.ClientTime = 1000 ,              					//静默时间-微秒
		.DelayOutputTime_US = 0,          				//延迟输出时间-微秒
		.OtherWaveformPara.AlternatingFreq_HZ = 0,      //交替波形频率 //
		.OtherWaveformPara.TotalOutputTime_S = 0,       //设置总输出时间 秒
		.OtherWaveformPara.NumOfPulseGroups  = 0,       //脉冲群模式下的每组脉冲数量
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   	//脉冲群模式下的组间隔时间 毫秒
	},
	/*通道2 -- ST2 ST3 */
	{
		.Type = TRIANGULAR_WAVE ,          					//波形种类选择
		.PositivePulseWidth = 0  ,      					//正半周期脉宽-微秒
		.DeadTime   = 0,                  					//死区时间-微秒
		.NegativePulseWidth = 0 ,       					//负半周期脉宽-微秒
		.ClientTime = 0 ,              						//静默时间-微秒
		.DelayOutputTime_US = 0,          					//延迟输出时间-微秒
		.OtherWaveformPara.AlternatingFreq_HZ = 0,          //交替波形频率 //
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//设置总输出时间 秒
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//脉冲群模式下的每组脉冲数量
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   		//脉冲群模式下的组间隔时间 毫秒
	},
	/*通道3 -- ST4 ST5 */
	{
		.Type = TRIANGULAR_WAVE ,          					//波形种类选择
		.PositivePulseWidth = 0  ,      					//正半周期脉宽-微秒
		.DeadTime   = 0,                  					//死区时间-微秒
		.NegativePulseWidth = 0 ,       					//负半周期脉宽-微秒
		.ClientTime = 0 ,              						//静默时间-微秒
		.DelayOutputTime_US = 0,          					//延迟输出时间-微秒
		.OtherWaveformPara.AlternatingFreq_HZ = 0,        	//交替波形频率 //
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//设置总输出时间 秒
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//脉冲群模式下的每组脉冲数量
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,  		//脉冲群模式下的组间隔时间 毫秒
	},
	/*通道4 -- ST6 ST7 */
	{
		.Type = TRIANGULAR_WAVE ,          					//波形种类选择
		.PositivePulseWidth = 0  ,      					//正半周期脉宽-微秒
		.DeadTime   = 0,                  					//死区时间-微秒
		.NegativePulseWidth = 0 ,      					 	//负半周期脉宽-微秒
		.ClientTime = 0 ,              						//静默时间-微秒
		.DelayOutputTime_US = 0,          					//延迟输出时间-微秒
		.OtherWaveformPara.AlternatingFreq_HZ = 0,          //交替波形频率 //
		.OtherWaveformPara.TotalOutputTime_S = 0,       	//设置总输出时间 秒
		.OtherWaveformPara.NumOfPulseGroups  = 0,        	//脉冲群模式下的每组脉冲数量
		.OtherWaveformPara.TimeOfPulseGroups_MS = 0,   		//脉冲群模式下的组间隔时间 毫秒
	},
};

/*-------------------------------------------------------------------------------------*/

/*-------------------------获取到FT测试后的实际测量单元电流值---------------------------*/
uint16_t FT_CURRENT_SAVE[4]={0,0,0,0};                 //用于保存FT读取出的数据
uint8_t CUSTOM_UNIT_CURRENT[4]={33,33,33,33};          //用于自定义单元电流值
uint8_t  unit_current[4] ={0,0,0,0};                   //保存计算好的实际单元电流值
uint8_t GET_FT_CURRENT(void)                           //电流设置：208*4
{	
	//当需要使用FT后的芯片时，打开注释， 此时自动计算单元电流
	//当手动校准时，修改CUSTOM_UNIT_CURRENT[4] 数组中的数据，并将下面4行注释掉
//	flash_read(DATA_SAVE_ADDR ,&FT_CURRENT_SAVE[0]);//读出保存在MTP中的实际检测电流值用于计算实际单元电流
//	flash_read(DATA_SAVE_ADDR+2 ,&FT_CURRENT_SAVE[1]);//读出保存在MTP中的实际检测电流值用于计算实际单元电流
//	flash_read(DATA_SAVE_ADDR+4 ,&FT_CURRENT_SAVE[2]);//读出保存在MTP中的实际检测电流值用于计算实际单元电流
//	flash_read(DATA_SAVE_ADDR+6 ,&FT_CURRENT_SAVE[3]);//读出保存在MTP中的实际检测电流值用于计算实际单元电流
	for(int i = 0 ;i<4;i++)
	{
		if(FT_CURRENT_SAVE[i] != 0)
			unit_current[i] = (uint8_t)(FT_CURRENT_SAVE[i] /( 208 * 4 )) ;  //计算实际单元电流
		else 
			unit_current[i] = CUSTOM_UNIT_CURRENT[i] ;
	}
	return 0;
}

/*-------------------------------------输出电流大小设置--------------------------------*/
//正弦波计算数据填充值
double sin_cal(uint8_t angle)  //0-180
{	
	double radian = angle*PI/180;
	return sin(radian);
}

//计算不同波形下的的64个点的值，当前有方波，正弦波，三角波
uint16_t unit_t[4]; //保存计算后的单元电流系数（0-7）  共四个通道
uint32_t isel_t[4][64]; //保存计算后得到的电流挡位（0-255）共四个通道
BasicWaveformType  wavePara_type[4];
uint8_t cal_output_current_mA(CHANNEL_NUM CHANNEL_X,float mA,BasicWaveformType type, uint16_t* unit , uint32_t (*isel)[64])   // uint16_t read_current_data  
{	
	CMSDK_WAVE_GEN_TypeDef*         WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0)      WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK =             WAVE_GEN_DRVA_BLK3;
	
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
			//保存计算结果,用于给电流相关的寄存器赋值
			*(unit+CHANNEL_X) = (uint16_t)(mA * 1000 / (unit_current[CHANNEL_X] * 255));                    // 0 - 7
			for(int i = 0 ;i<64;i++)
			{
				*(isel[0]+CHANNEL_X*64+i) = (uint16_t)(mA * 1000 / ((*(unit+CHANNEL_X)+1)*unit_current[CHANNEL_X]));      // 0 - 255		
			}
		}
		else if(type== SINE_WAVE)
		{	
			//保存计算结果,用于给电流相关的寄存器赋值
			*(unit+CHANNEL_X) = (uint16_t)(mA * 1000 / (unit_current[CHANNEL_X] * 255));                    // 0 - 7
			for(int i = 0 ;i<64;i++)
			{
				*(isel[0]+CHANNEL_X*64+i) = (uint16_t)(mA * 1000 / ((*(unit+CHANNEL_X) + 1)*unit_current[CHANNEL_X])*sin_cal(i*2.85714));      // 0 - 255		
			}
		}	
		else //三角波赋值(type == TRIANGULAR_WAVE )
		{
			//保存计算结果,用于给电流相关的寄存器赋值
				
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

/*初始化设置*/
/*CONFIG寄存器设置*/
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


/*---------------------------电刺激有关的寄存器的初始化--------------------------------*/
uint32_t  StimulatorInit(CHANNEL_NUM CHANNEL_X)
{
	//uint32_t WaveformFreq=0;
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;
	else if(CHANNEL_X == CHANNEL_1) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;
	else if(CHANNEL_X == CHANNEL_2) WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;
	else WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;
	//config 设置
	ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,DISABLE_ALL_BIT);
	//1 、 死区时间不为0，波形不为sine波 ,使能REST 
	if((ParaSet_waveform[CHANNEL_X].DeadTime > 0)){            
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,REST_BIT);
	}
	else
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,REST_BIT);
	}
	//2 、 负半周期脉宽大于0 使能 NEGATIVE_BIT 和 SOURCE_B_BIT
	if(ParaSet_waveform[CHANNEL_X].NegativePulseWidth > 0)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,SOURCE_B_BIT );  //SOURCEB不使能则负半周期的数据无效（=0）
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,NEGATIVE_BIT );		
	}
	//3、静默时间不为0，开启SILENT_BIT
	if(ParaSet_waveform[CHANNEL_X].ClientTime>0)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,SILENT_BIT);
	}
	else
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,SILENT_BIT);
	}
	
	//4、交替模式ALTERNATING_POSITIVE_BIT
	if(ParaSet_waveform[CHANNEL_X].OtherWaveformPara.AlternatingFreq_HZ > 0 )
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,ALTERNATING_POSITIVE_BIT);
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,SOURCE_B_BIT);
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,NEGATIVE_BIT);
		//交替时间与主频有关,输入的参数为交替频率，需要转化为时钟个数，时钟个数不能超过2^16
		WAVE_GEN_BLK ->WAVE_GEN_DRV_ALT_LIM_REG =(uint16_t)(ParaSet_waveform[CHANNEL_X].PositivePulseWidth / (uint16_t)ParaSet_waveform[CHANNEL_X].OtherWaveformPara.AlternatingFreq_HZ * (uint16_t)(APB_Clock_Freq/1000000)) ;
		ParaSet_waveform[CHANNEL_X].NegativePulseWidth = 0;
	}
	else 
	{
		ENS_STIMU_CONFIG_BIT_DISABLE(CHANNEL_X,ALTERNATING_POSITIVE_BIT);
	}
	
	ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,CONTINUE_REPEATING_BIT);	
	//延迟输出时间计算  单位us
	WAVE_GEN_BLK->WAVE_GEN_DRV_DELAY_LIM_REG  = (uint8_t)(APB_Clock_Freq/1000000)*ParaSet_waveform[CHANNEL_X].DelayOutputTime_US;
	//根据初始化后的STRUCT_WAVEFORM_PARA结构体给对应的寄存器赋值

	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG  =  ParaSet_waveform[CHANNEL_X].NegativePulseWidth;
	WAVE_GEN_BLK->WAVE_GEN_DRV_HLF_WAVE_PRD_REG      =  ParaSet_waveform[CHANNEL_X].PositivePulseWidth;
	WAVE_GEN_BLK->WAVE_GEN_DRV_CLK_FREQ_REG          =  (uint32_t)(APB_Clock_Freq/1000000);  //
	WAVE_GEN_BLK->WAVE_GEN_DRV_SILENT_T_REG          =  ParaSet_waveform[CHANNEL_X].ClientTime;
	WAVE_GEN_BLK->WAVE_GEN_DRV_REST_T_REG            =  ParaSet_waveform[CHANNEL_X].DeadTime;
	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_SCALE_REG         =  1;   //寄存器默认为0， 必须写大于0的数
	WAVE_GEN_BLK->WAVE_GEN_DRV_NEG_OFFSET_REG        =  0;
	
	//WaveformFreq = (uint32_t)(1000000 / (wavePara.ClientTime + wavePara.DeadTime +wavePara.NegativePulseWidth + wavePara.PositivePulseWidth));
	//保存波形类型数据
	wavePara_type[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].Type;
	
	TOTAL_TIME_THRESHOLD_VALUE[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TotalOutputTime_S * 1000;
	if( (ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TimeOfPulseGroups_MS > 0)  || (ParaSet_waveform[CHANNEL_X].OtherWaveformPara.NumOfPulseGroups >0))
	{
		TRIGGER_TIME_COUNT[CHANNEL_X] = 0;  //间隔时间清零	
		TRIGGER_TIME_THRESHOLD_VALUE[CHANNEL_X]=ParaSet_waveform[CHANNEL_X].OtherWaveformPara.TimeOfPulseGroups_MS ;
		NUM_OF_PULSES_THRESHOLD[CHANNEL_X] = ParaSet_waveform[CHANNEL_X].OtherWaveformPara.NumOfPulseGroups ;
		WAVE_GEN_BLK->WAVE_GEN_DRV_INT_REG               =  0x001f0001;
	}
	uint8_t returnValue = cal_output_current_mA(CHANNEL_X,0, wavePara_type[CHANNEL_X], unit_t , isel_t) ;
	
	return (uint32_t)(WAVE_GEN_BLK->WAVE_GEN_DRV_CONFIG_REG);  //返回波形的频率
}

/*--------------------------------------启动输出-----------------------------------------*/
volatile uint8_t statics_config=0;    //明确当前有几个通道在输出 0 / 1 / 2 /4 / 8
void StartStimulatorOut(CHANNEL_NUM CHANNEL_X)
{
	CMSDK_WAVE_GEN_TypeDef* WAVE_GEN_BLK;
	if(CHANNEL_X == CHANNEL_0) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK0;  statics_config |= (1<<0); }
	else if(CHANNEL_X == CHANNEL_1) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK1;  statics_config |= (1<<1);}
	else if(CHANNEL_X == CHANNEL_2) {WAVE_GEN_BLK=WAVE_GEN_DRVA_BLK2;    statics_config |= (1<<2);}
	else {WAVE_GEN_BLK = WAVE_GEN_DRVA_BLK3;     statics_config |= (1<<3) ;}
	if(statics_config > 1)
	{
		ENS_STIMU_CONFIG_BIT_ENABLE(CHANNEL_X,MULTI_ELECTRODE_BIT);  //多电极使能
	}
	WAVE_GEN_BLK->WAVE_GEN_DRV_CTRL_REG = 1;
}
/*--------------------------------------停止输出----------------------------------------*/
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

/*-----------------------------电刺激过程中的电流、频率设置-------------------------------*/
/*输出电流大小修改*/
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
	WAVE_GEN_BLK ->WAVE_GEN_DRV_ISEL_REG = unit_t[CHANNEL_X];  //范围 0x00 - 0x07   单元电流
	for(int i=0; i<64; i++){
		WAVE_GEN_BLK->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i;  
		WAVE_GEN_BLK->WAVE_GEN_DRV_IN_WAVE_REG = isel_t[CHANNEL_X][i]; //最大0xff   电流挡位
	}	
	WAVE_GEN_BLK->WAVE_GEN_DRV_INT_REG =  0x001f0001;
	NVIC_EnableIRQ(WG_DRV_IRQn);
	StartStimulatorOut(CHANNEL_X);
	return (uint8_t)(WAVE_GEN_BLK->WAVE_GEN_DRV_CTRL_REG); //返回状态
}

/*输出波形频率的实时修改，输出时，波形频率=(1000000/(正半周期脉宽 + 负半周期脉宽 + 死区时间 + 静默时间))*/
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
	//死区数值不改动，如果静默时间不为0 ，则通过修改静默时间修改波形频率 ，如果静默时间为0，则修改正半周期/负半周期脉宽修改频率
	if(ParaSet_waveform[CHANNEL_X].ClientTime > 0)
	{
		Pulse_Width/*静默时间*/ = (uint32_t)((1000000/freq) - ParaSet_waveform[CHANNEL_X].DeadTime - ParaSet_waveform[CHANNEL_X].PositivePulseWidth - ParaSet_waveform[CHANNEL_X].NegativePulseWidth);
		WAVE_GEN_BLK->WAVE_GEN_DRV_SILENT_T_REG = Pulse_Width;
		return Pulse_Width;
	}	
	else if(ParaSet_waveform[CHANNEL_X].ClientTime == 0)
	{
		Pulse_Width/*正+负脉宽*/ = (uint32_t)((1000000/freq) - ParaSet_waveform[CHANNEL_X].DeadTime) ; 
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


//电刺激时间计时

uint32_t StimuTimeCount_S(CHANNEL_NUM CHANNEL_X , TIME_COUNT_MODE MODE)
{
	uint32_t count_time = 0;
	if(MODE == TOTAL_TIME_MODE)
	{
		count_time = *(CHANNEL_TIME_COUNT+CHANNEL_X) ;
	}
	else          //MODE == TRIGGER_TIME_MODE  间隔时间
	{
		count_time = *(TRIGGER_TIME_COUNT+CHANNEL_X);
	}
	return count_time;
}

//返回当前波形发生了多少次（需要开启波形发生器的中断）
uint32_t waveformOccurreCount(CHANNEL_NUM CHANNEL_X)
{
	return (uint32_t)(wave_gen_irq_occurred[CHANNEL_X]/4);
}

/*-------------------------------电刺激驱动器中断处理函数-----------------------*/
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

		if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_READ_DRIVER_NUM_Msk) == i) //判断哪个通道发生了中断
		{
			if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_STS_Msk) == CMSDK_WAVE_GEN_DRV_INT_FIRSTADDR_STS_Msk)//第一中断点发生中断
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
				if((NUM_OF_PULSES_THRESHOLD[i] > 0))  //需要脉冲计数功能才开启计数
					wave_gen_irq_occurred[i]++;
				
				
			}
			if((DRVA->WAVE_GEN_DRV_INT_REG & CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_STS_Msk) == CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_STS_Msk) 
			{   
				StopStimulatorOut((CHANNEL_NUM)i);
				DRVA->WAVE_GEN_DRV_INT_REG = CMSDK_WAVE_GEN_DRV_INT_SECONDADDR_CLR_Msk | CMSDK_WAVE_GEN_DRV_INT_EN_Msk;
				NVIC_DisableIRQ(WG_DRV_IRQn);
				DRVA->WAVE_GEN_DRV_INT_REG =  0;
				TRIGGER_TIME_COUNT_FLAG |= (1<<i);  //脉冲群标志置1
				wave_gen_irq_occurred[i] = 0;       //清零计数
			}
		}	
	}
}














/*
*Copyright ,2024 , NANOCHAP
*File name:   MIAN.C          
*Author:      
*Version:     V1.0 
*Date:        2024-11-                  
*Description:   UART1? / 
*Function List: 
   
History: 
1.V1.0
Date:          
Author: 
Modification: 

????UART0UART1????UART1,GPIO12--RX,GPIO13--TX

?嶨
FIFO?嶨
typedef struct uart_fifo{
	uartRcvTrigger  level;  FIDO?14814bytes
	bool DMA_Enable  ;		DMA??
	bool FIFO_Enable ;		FIFO?
}UART_FifoStructrue;		

???嶨
typedef struct uart_para{		
	 uint32_t UART_BaudRate;				?
	 bool UART_HardwareFlowControl;		    ??
	 UART_FifoStructrue*  FifoSetting ;		FIFO??
	 
	 UART_StopLen stop_len;					??λ
	 UART_WordLen Word_len;					λ
	 UART_Stick_EN Stick_EN;				??У
	 UART_Even_EN Even_EN;					?У?
	 UART_Parity_EN Parity_EN;				??У
}UART_InitStructure;

??λ???λ????1??λ1.5??λ2??λ
??λUART_InitStructure??stop_lenWord_len?
1stop_len?StopLen_1???λ?1Word_len?
2stop_len?StopLen_OtherWord_len?WordLen_5???λ?1.5λ
3stop_len?StopLen_OtherWord_len?WordLen_5???λ?2λ


Уλ?Уλ????У飨None,У飨Odd?У飨Even?1У飨Mark?0У飨Space
УλUART_InitStructure??Stick_EN,Even_ENParity_EN?
1Parity_EN?Parity_RESET??Уλ
2Parity_EN?Parity_SETStick_EN?Stick_RESETEven_EN?Odd_SET?Уλ
3Parity_EN?Parity_SETStick_EN?Stick_RESETEven_EN?Even_SET?Уλ
4Parity_EN?Parity_SETStick_EN?Stick_SETEven_EN?Odd_SETù?1Уλ
5Parity_EN?Parity_SETStick_EN?Stick_SETEven_EN?Even_SETù?0Уλ


???
divisor_value = (uint16_t)((uint32_t)(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1);
CMSDK_UART->DLL = (uint8_t)(divisor_value & 0x00FF);
CMSDK_UART->DLH = (uint8_t)((divisor_value & 0xFF00)>>8);
?·??
1???1?λ?115200?1??115200λ?λ???1/115200

2???1???32MHz?1?32M??????1/32000000

3?ò?????λ?????????λCMSDK_UART->DLH?λCMSDK_UART->DLLС

4overSamp_modeOSM_SEL?0??131??16??е?η????APB_Clock_Freq??uart_paraX->UART_BaudRate?ò

5(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1)УAPB_Clock_Freq / overSamp_mode????е?η?
??????1?APB_Clock_Freq / overSamp_mode????ò??λ??????
(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)???λ???0??1
?(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)???д(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)-1

6????????Щ???15.9??15????ò?
????????????г????

7???????????4M115200?4000000/115200/16-1=1.17 ,???1
???м????С????????????????Ч?????

*/  

#include "my_header.h"
#include "ENS1_MTP.h"
#include "ENS_CURRENT_CALIBRATION.h"
#include "ENS1_CLOCK.h"
#include "ENS1_ANAC.h"
#include "ENS1_UART.h"

uint32_t GetParamNumber=0;//?FIFO?
uint8_t Get_Data=0;//?

uint8_t rev_buf[16];
uint8_t rev_len = 0;
uint8_t rev_fin = 0;

uint8_t send_buf[16];
uint8_t send_len = 0;
uint8_t send_fin = 0;

float g_freq = 10.0;
float g_current = 10.0;
volatile uint8_t g_enable_NNC6121_HSE = 0;

void Delay_268ms( uint16_t time_ms )
{
  uint16_t i,j;

  for( i=0;i<time_ms*268;i++ )
  {
		for( j=0;j<1001;j++ );
  }
}
// CRC8У
static const uint8_t crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31,
    0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9,
    0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
    0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE,
    0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80,
    0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8,
    0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
    0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F,
    0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7,
    0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF,
    0xFA, 0xFD, 0xF4, 0xF3
};


uint8_t crc8_calc(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;  
    
    while (len--) {
        crc = crc8_table[crc ^ *data++];
    }
    
    return crc;  
}

// ?
int sine_data[64] =  {0x0c, 0x18, 0x25, 0x31, 0x3d, 0x4a, 0x55,
											0x61, 0x6d, 0x78, 0x83, 0x8d, 0x97, 0xa1, 
											0xab, 0xb4, 0xbc, 0xc5, 0xcc, 0xd4, 0xda, 0xe0, 
											0xe6, 0xeb, 0xf0, 0xf4, 0xf7, 0xfa, 0xfc, 0xfd, 0xfe, 
											0xff, 0xfe, 0xfd, 0xfc, 0xfa, 0xf7, 0xf4, 0xf0, 0xeb,
											0xe6, 0xe0, 0xda, 0xd4, 0xcc, 0xc5, 0xbc, 0xb4, 0xab,
											0xa1, 0x97, 0x8d, 0x83, 0x78, 0x6d, 0x61, 0x55, 0x4a,
											0x3d, 0x31, 0x25, 0x18, 0x0c, 0x00};

/* --------------------------------------------------------------- */
/*  WAVE GEN Driver_A SINE test                                    */
/* --------------------------------------------------------------- */

int wavegen_driverA_sine_test(CMSDK_WAVE_GEN_TypeDef *CMSDK_WAVEGEN_DRVA, int units){
    int return_val=0;
    int err_code=0; 
    int Current_units=0;
	  float freq = 0;
    //puts("\nDRIVER A SINE TEST\n");

    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_REST_T_REG = 0x00000000;   //?
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_SILENT_T_REG = 0;  //??
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CLK_FREQ_REG = 0x00000020;     //32MHZ ==PCLK
	  freq = (500.00/g_freq)*1000.00;
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = freq;   ; //200us  //
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = freq;  ; //500us  //
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CONFIG_REG = 0x0000004A; //bit 0:rest enable   ??м ?? 
															  //bit1:negative enable   ??????и?
	                                                          //bit2: silent enable    ????
                                                              //bit3: source B enable 
															  //bit4: alternating the positive side ,棬
                                                              //bit5: continue mode   
	                                                          //bit6: multi-electrode 
	Current_units=4*(units+1)-1;
	Current_units = 1;
	if (CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK0){
		WAVE_GEN_DRVC_BLK5->WAVE_GEN_DRV_ISEL_REG=Current_units&0x7;	// BLK5 driver C is for pair 0,low 3 bit for unit of current
		}
	else if(CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK1){
		WAVE_GEN_DRVC_BLK6->WAVE_GEN_DRV_ISEL_REG=Current_units&0x7; // BLK6 driver C is for pair 1, low 3 bit for unit of current
	}
	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ISEL_REG=(Current_units&(0x7<<3))>>3; // high 3 bit for units of current

    // 【修复 BUG】: 启动波形时，必须把当前 RAM 中已校准的 unit_current 比例叠加进去
    float unit_val = (unit_current[0] > 0) ? (float)unit_current[0] : 33.0f;
    float scale_factor = 0.6455f * (g_current / 20.0f) * (unit_val / 33.0f);

    for(int i=0; i<64; i++){
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i; 
       	int val = (int)( (float)sine_data[i] * scale_factor );
        if (val > 255) val = 255;
        if (val < 0) val = 0;
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = val;
    }
   
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_LIM_REG = 0x00000000;//  ? 
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_SILENT_LIM_REG = 0x00000000; // ???b
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_DELAY_LIM_REG = 0x00000000;  // ??
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_SCALE_REG = 0x00000001;  //??糬2550?
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_OFFSET_REG = 0x00000000; //???糬2550?
                                                                  //???250??10????5 
		
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_INT_REG = 0x0;//  interrupt register set to 0
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CTRL_REG = 0x00000001; //? driver

    /* Generate return value */
    if (err_code != 0) {
       //puts("\nERROR : Driver A Test Failed\n");
       return_val=1;
       err_code = 0;
    }

    return(return_val);
}
void wavegen_Stop(CMSDK_WAVE_GEN_TypeDef *CMSDK_WAVEGEN_DRVA)
{
	if (CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK0){
		WAVE_GEN_DRVC_BLK5->WAVE_GEN_DRV_ISEL_REG=0;	// BLK5 driver C is for pair 0,low 3 bit for unit of current
		}
	else if(CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK1){
		WAVE_GEN_DRVC_BLK6->WAVE_GEN_DRV_ISEL_REG=0; // BLK6 driver C is for pair 1, low 3 bit for unit of current
	}
	CMSDK_WAVEGEN_DRVA ->WAVE_GEN_DRV_ISEL_REG = 0;  //Χ 0x00 - 0x07
	
	for(int i=0; i<64; i++){
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i; 
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = 0; //0xff
    }


}
 void Delay_ms( uint16_t time_ms )
{
  uint16_t i,j;
	if(time_ms<5000) //??С?????
	{
		time_ms=5000;
	}
	
  for( i=0;i<time_ms;i++ )
  {
		for( j=0;j<1001;j++ );
  }
}

//??
 void SysHseSet(void)
{
		
		CMSDK_GPIO->IE = (CMSDK_GPIO->IE & ~(0x01ul << 0)) | (0x01 << 0); //GPIO0??
		CMSDK_GPIO->ALTFL = (CMSDK_GPIO->ALTFL & ~(0x03ul << 0)) | (0x02 << 0); //GPIO0?HSE_CLK
		CMSDK_SYSCON->CLK_CFG  = (CMSDK_SYSCON->CLK_CFG & ~CMSDK_SYSCON_SYSCLK_SEL_Msk) | (0x1 << CMSDK_SYSCON_SYSCLK_SEL_Pos); // ?HSE??
		while (((CMSDK_SYSCON->CLK_CFG & CMSDK_SYSCON_SYSCLK_SWSTS_Msk) >> CMSDK_SYSCON_SYSCLK_SWSTS_Pos) != 0x01) { }       //?HSE????GPIO0
		CMSDK_SYSCON->HSI_CTRL = (CMSDK_SYSCON->HSI_CTRL & ~CMSDK_SYSCON_HSI_EN_Msk); //HSI OSC OFF
}

//е????
 void SwitchHseFun(uint8_t aSec)
{

		if(aSec <5) aSec=5;
		if(30<aSec) aSec=30;
		uint32_t aDelayTime = aSec*1000;
		Delay_ms(aDelayTime);		//?5sGPIO?HSE	
	
		SysHseSet();
}

/* 0x04 电流校准处理 (多次 RAM 逼近，不写入 MTP)
   只处理校准计算, 不回发响应帧 (与 start/pause 只发不收风格一致,
   消除响应帧 0x00 字节信号畸变及 HK32 端轮询/中断竞争问题) */
static void handle_calibrate_current(void) {
	uint8_t target_mA = rev_buf[3];
	uint16_t measured_x100 = ((uint16_t)rev_buf[4] << 8) | rev_buf[5];

	// 特殊指令：target_mA == 0xFF 为软复位，恢复 RAM 中的 unit_current[0] 为默认 33uA
	if (target_mA == 0xFF) {
		unit_current[0] = CUSTOM_UNIT_CURRENT[0]; // 33uA
		float mA = (g_current > 0) ? ((float)g_current / 10.0f) : 2.0f;
		cal_output_current_mA(CHANNEL_0, mA, SINE_WAVE, unit_t, isel_t);
		return;
	}

	// 输入有效性检查 (target_mA 以 0.1mA 为单位，范围 0.1mA~20.0mA 即 1~200)
	if (target_mA < 1 || target_mA > 200 || measured_x100 == 0) {
		return; // 参数非法, 不回发
	}

	// target_mA 单位为 0.1mA，x10 统一换算为以 0.01mA 为单位 (例 20 -> 200 即 2.00mA)
	uint32_t I_exp = (uint32_t)target_mA * 10;
	// measured_x100 单位本身即为 0.01mA (例 215 即 2.15mA)
	uint32_t I_act = (uint32_t)measured_x100;
	uint32_t old_unit = (uint32_t)unit_current[0];
	// 四舍五入逼近计算新的 unit_current (uA): 目标 I_exp / 实测 I_act 比例进行正向缩放
	uint32_t new_unit = (old_unit * I_exp + I_act / 2) / I_act;

	if (new_unit < 3 || new_unit > 250) {
		return; // 超出合理范围, 不回发
	}

	unit_current[0] = (uint8_t)new_unit; // 仅更新 RAM 变量！不写入 MTP

	// 动态刷新 WaveGen 硬件 RAM 描点 (保持 DEMO 官方基准缩放)
	float target_ratio = (g_current > 0) ? ((float)g_current / 20.0f) : 1.0f;
	float unit_val = (unit_current[0] > 0) ? (float)unit_current[0] : 33.0f;
	float scale_factor = 0.6455f * target_ratio * (unit_val / 33.0f);
	for(int i = 0; i < 64; i++){
		WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i;
		int val = (int)((float)sine_data[i] * scale_factor);
		if (val > 255) val = 255;
		if (val < 0) val = 0;
		WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_IN_WAVE_REG = val;
	}
}

/* 0x05 一键落盘 MTP 存储处理 */


int main(){
	int count;
	Delay_268ms(1);
	MTP_init();
	ClockInit();
	/*uart*/
	
///****************FIFOж??*************************/
//    UART_Init(CMSDK_UART1, &UART1_Init);
//    UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
//	char* a = "uartputchar_test";
//	while(1)
//	{  	
//		printf("%s \n" , a);
//		for(uint32_t i = 0;i<2000000 ;i++){};	
//		UartPutc(CMSDK_UART1,0x5E);	
//	}
///****************************************************/
	
	
/****************FIFO???*************************/
//    UART_Init(CMSDK_UART1, &UART1_Init);
//	//?ж?FIFO????
//	//  UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
//	while(1)
//	{  	
//		//λ?
//		if((CMSDK_UART1->LSR & 0x01)== 0x01)
//		{
//			//?FIFO
//			GetParamNumber=(CMSDK_UART1->FSR>>16) & 0x1f;
//			for(uint32_t i=0;i<GetParamNumber;i++)
//			{
//				Get_Data=CMSDK_UART1->RBR;//?
//				UartPutc(CMSDK_UART1,Get_Data);//
//			}
//		}
//	}
/****************************************************/


	// uint8_t  data[8] = {1,2,3,4,5,6,7,8};
	
	UART_Init(CMSDK_UART1, &UART1_Init);
	
    UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
	
	
	CMSDK_SYSCON->APB_CLKEN = 0x4000|0x1000|CMSDK_SYSCON->APB_CLKEN;
	//???1-20%=80% ,256? ,10.5A ,26V,??,?,   no_?mos	
		//??64?  10.5A   26V  ??   
		CMSDK_ANAC->BOOST_CTRL =0x77213;
		CMSDK_ANAC->PMU_CTRL = 0x10;
	
	while(1){
		//Uart_Send(CMSDK_UART1,data,7);
		while(rev_fin && rev_len >0){
			  //check crc
			  
		#if 1
			   if(rev_buf[0] != 0xA5 || rev_buf[rev_buf[2]+3+1] != 0x5A || rev_buf[rev_buf[2]+3] != crc8_calc(rev_buf,rev_buf[2]+3) )
				 {
				 		rev_len = 0;
						rev_fin = 0;
					    break;
				 } 
		#endif
			  if(rev_buf[1] == 0x00 && rev_buf[2] == 0x01 && rev_buf[3] == 0x01 ){//start
				  send_buf[0]=0xA5;
				  send_buf[1]=0x09 ;
					send_buf[2]=0x02 ;
					send_buf[3]=0x00 ;
					send_buf[4]=0x00 ;
					send_buf[5]=crc8_calc(send_buf,5) ;
					send_buf[6]=0x5A ;
					send_len = 7;
					
          for(count = 0 ; count < 200; count++){;}
						
					Uart_Send(CMSDK_UART1,send_buf,send_len);
					wavegen_driverA_sine_test(WAVE_GEN_DRVA_BLK0,3);
				}else if(rev_buf[1] == 0x00 && rev_buf[2] == 0x01 && rev_buf[3] == 0x02 ){//pause
				  send_buf[0]=0xA5;
				  send_buf[1]=0x09 ;
					send_buf[2]=0x02 ;
					send_buf[3]=0x00 ;
					send_buf[4]=0x00 ;
					send_buf[5]=crc8_calc(send_buf,5) ;
					send_buf[6]=0x5A ;
					send_len = 7;
					
					for(count = 0 ; count < 200; count++){;}
						
					Uart_Send(CMSDK_UART1,send_buf,send_len);		
          wavegen_Stop(WAVE_GEN_DRVA_BLK0);					
				}else if(rev_buf[1] == 0x00 && rev_buf[2] == 0x01 && rev_buf[3] == 0x00 ){//stop
				  send_buf[0]=0xA5;
				  send_buf[1]=0x09 ;
					send_buf[2]=0x02 ;
					send_buf[3]=0x00 ;
					send_buf[4]=0x00 ;
					send_buf[5]=crc8_calc(send_buf,5) ;
					send_buf[6]=0x5A ;
					send_len = 7;
					
					for(count = 0 ; count < 200; count++){;}
						
					Uart_Send(CMSDK_UART1,send_buf,send_len);		
          wavegen_Stop(WAVE_GEN_DRVA_BLK0);					
				}else if(rev_buf[1] == 0x02 && rev_buf[2] == 0x03){//set param
					g_freq = rev_buf[4];
					g_current = rev_buf[3];
					send_buf[0]=0xA5;
				  send_buf[1]=0x09 ;
					send_buf[2]=0x02 ;
					send_buf[3]=0x02 ;
					send_buf[4]=0x00 ;
					send_buf[5]=crc8_calc(send_buf,5) ;
					send_buf[6]=0x5A ;
					send_len = 7;
					
					for(count = 0 ; count < 200; count++){;}
						
					Uart_Send(CMSDK_UART1,send_buf,send_len);	
					// if(rev_buf[5] != 0 && g_enable_NNC6121_HSE != 1){
					// 	g_enable_NNC6121_HSE = 1;
						
					// 	//SwitchHseFun(5);
						
					// 	Delay_268ms(1);SysHseSet();
					// }
					
				}else if(rev_buf[1] == 0x03 && rev_buf[2] == 0x01){
					send_buf[0]=0xA5;
				  send_buf[1]=0x0A ;
					send_buf[2]=0x02 ;
					send_buf[3]=0x60|0x07 ;
					send_buf[4]=15 ;
					send_buf[5]=crc8_calc(send_buf,5) ;
					send_buf[6]=0x5A ;
					send_len = 7;
					
					for(count = 0 ; count < 200; count++){;}
						
					Uart_Send(CMSDK_UART1,send_buf,send_len);	
				}else if(rev_buf[1] == 0x04 && rev_buf[2] == 0x03){ // 0x04 RAM 电流校准 (不写 MTP)
					handle_calibrate_current();
				}
		    
			  rev_len = 0;
			  rev_fin = 0;
		}
		
	}



}


/*
*Copyright ,2024 , NANOCHAP
*File name:   MIAN.C          
*Author:      
*Version:     V1.0 
*Date:        2024-11-                  
*Description:   UART1??? / ????????
*Function List: 
   
History: 
1.V1.0
Date:          
Author: 
Modification: ????

???????????????UART0??UART1??????????????????????UART1,GPIO12--RX,GPIO13--TX

????G??????
????FIFO????G??
typedef struct uart_fifo{
	uartRcvTrigger  level;  ????FIDO???????????????????1??4??8??14bytes
	bool DMA_Enable  ;		DMA????
	bool FIFO_Enable ;		FIFO???
}UART_FifoStructrue;		

????????????G??
typedef struct uart_para{		
	 uint32_t UART_BaudRate;				?????????
	 bool UART_HardwareFlowControl;		    ????????????
	 UART_FifoStructrue*  FifoSetting ;		????FIFO???????
	 
	 UART_StopLen stop_len;					????
	 UART_WordLen Word_len;					??????
	 UART_Stick_EN Stick_EN;				??????????????
	 UART_Even_EN Even_EN;					????????????
	 UART_Parity_EN Parity_EN;				??????????
}UART_InitStructure;

?????????????????????????1????????1.5????????2??????
??????UART_InitStructure???????stop_len??Word_len???????
1????stop_len?StopLen_1????????1????Word_len???
2????stop_len?StopLen_Other??Word_len?WordLen_5????????1.5??
3????stop_len?StopLen_Other??Word_len???WordLen_5????????2??


??????????????????????????????????????None??,???????Odd??????????Even???????1?????Mark???????0?????Space??
????????UART_InitStructure???????Stick_EN,Even_EN??Parity_EN???????????????
1????Parity_EN?Parity_RESET????????????????
2????Parity_EN?Parity_SET??Stick_EN?Stick_RESET??Even_EN?Odd_SET???????????????
3????Parity_EN?Parity_SET??Stick_EN?Stick_RESET??Even_EN?Even_SET?????????????
4????Parity_EN?Parity_SET??Stick_EN?Stick_SET??Even_EN?Odd_SET?????????1??????
5????Parity_EN?Parity_SET??Stick_EN?Stick_SET??Even_EN?Even_SET?????????0??????


???????????????????
divisor_value = (uint16_t)((uint32_t)(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1);
CMSDK_UART->DLL = (uint8_t)(divisor_value & 0x00FF);
CMSDK_UART->DLH = (uint8_t)((divisor_value & 0xFF00)>>8);
????????????????
1?????????????1??????????????????115200???????????1??????????115200????????????????????1/115200??

2???????????????1???????????????32MHz???????????1???????32M????????????????????????1/32000000??

3???????????????????????????????????????????????????????????????????????????????????CMSDK_UART->DLH????????????CMSDK_UART->DLL????

4??overSamp_mode????OSM_SEL???????????0??13??1??16???????????????????????????????????????APB_Clock_Freq??????????uart_paraX->UART_BaudRate???????????

5??????(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1)????APB_Clock_Freq / overSamp_mode??????????????????????????????????
???????????????????????????1??APB_Clock_Freq / overSamp_mode?????????????????????????????????????????????????????????????????????????????????
??(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)????????????????????0????1????
?????(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)???????????????????(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)-1??

6??????????????????????????????????????????????????????????????????????????????????15.9??????????15??????????????????????????????????????
?????????????????????????????????????????????????????????????????????????????????????

7??????????????????????????????????????????????????????????????4M????????????115200????????????4000000/115200/16-1=1.17 ,???????1??
????????????????????????????????????????????????????????????????????????????????????????????????????

*/  

#include "my_header.h"
#include "ENS1_MTP.h"
#include "ENS1_CLOCK.h"
#include "ENS1_ANAC.h"
#include "ENS1_UART.h"
#include "ENS1_GPIO.h"
#include "ENS1_EXTI.h"

uint32_t GetParamNumber=0;//???FIFO???????????
uint8_t Get_Data=0;//???????

uint8_t rev_buf[16];
uint8_t rev_len = 0;
uint8_t rev_fin = 0;

uint8_t send_buf[16];
uint8_t send_len = 0;
uint8_t send_fin = 0;

float g_freq = 10.0;
float g_current = 10.0;
volatile uint8_t g_enable_NNC6121_HSE = 0;

/* SCH-260515: U1/U2 TLV217 collectors -> GPIO19 / GPIO22.
   Firmware only drives WAVE_GEN_DRVA_BLK0 (STIMU0/STIMU1). */
#define LOD_GPIO_CH0          GPIO_19
#define LOD_GPIO_CH2          GPIO_22
#define LOD_WIN_MIN_MS        200u
/* 2mA 闭路方波低电平约 0V，高电平只有 1V 多，GPIO 高门限够不着，全程读成 0。
   这是贴合。开路被上拉拉高，只有不到 1ms 的毛刺。连续低电平达到 LOD_LOW_MIN_MS 算贴合。 */
#define LOD_SAMPLE_HZ         10000u
#define LOD_TICKS_PER_MS      (LOD_SAMPLE_HZ / 1000u)
#define LOD_LOW_MIN_MS        3u

volatile uint32_t g_tick_ms = 0;
static volatile uint8_t  g_stim_running = 0;
/* 0=刺激中贴合, 1=脱落或电平卡死, 0xFF=未在刺激 */
static uint8_t  g_lead_off_ch0 = 0xFF;
static uint8_t  g_lead_off_ch2 = 0;
static volatile uint32_t s_sample_cnt = 0;
static volatile uint32_t s_low_run = 0;
static volatile uint32_t s_max_low_run = 0;
static uint8_t s_sub_ms = 0;
static uint32_t s_duty_tick = 0;
static uint32_t s_lod_arm_tick = 0;
static uint8_t s_open_latched = 0;

void Delay_268ms( uint16_t time_ms )
{
  uint16_t i,j;

  for( i=0;i<time_ms*268;i++ )
  {
		for( j=0;j<1001;j++ );
  }
}
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
// ?????????
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

    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_REST_T_REG = 0x00000000;   //???????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_SILENT_T_REG = 0;  //??????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CLK_FREQ_REG = 0x00000020;     //32MHZ ==PCLK
	  freq = (500.00/g_freq)*1000.00;
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = freq;   ; //200us  //??????????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = freq;  ; //500us  //????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CONFIG_REG = 0x0000004A; //bit 0:rest enable  ?? ????????????????? ?????? 
															  //bit1:negative enable  ?? ???????????????????????????????????????
	                                                          //bit2: silent enable   ?? ???????????????????
                                                              //bit3: source B enable 
															  //bit4: alternating the positive side ,?????????
                                                              //bit5: continue mode   ??
	                                                          //bit6: multi-electrode ??
	Current_units=4*(units+1)-1;
	Current_units = 1;
	if (CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK0){
		WAVE_GEN_DRVC_BLK5->WAVE_GEN_DRV_ISEL_REG=Current_units&0x7;	// BLK5 driver C is for pair 0,low 3 bit for unit of current
		}
	else if(CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK1){
		WAVE_GEN_DRVC_BLK6->WAVE_GEN_DRV_ISEL_REG=Current_units&0x7; // BLK6 driver C is for pair 1, low 3 bit for unit of current
	}
	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ISEL_REG=(Current_units&(0x7<<3))>>3; // high 3 bit for units of current

    for(int i=0; i<64; i++){
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i; 
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = (int)( (float)sine_data[i]*0.6587 *(g_current/20) + 0.5f ) ;
    }
   
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_LIM_REG = 0x00000000;//  ????? 
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_SILENT_LIM_REG = 0x00000000; // ?????????????????????????????????b????????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_DELAY_LIM_REG = 0x00000000;  // ????????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_SCALE_REG = 0x00000001;  //??????????????????^??255?????0???????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_OFFSET_REG = 0x00000000; //?????????????????^??255?????0?????????
                                                                  //?????????????250??????????10????????????5 ??????
		
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_INT_REG = 0x0;//  interrupt register set to 0
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CTRL_REG = 0x00000001; //??? driver

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
	/* 先停驱动。波形和 ISEL 写 0 时电流仍是 (0+1)*(0+1)*33µA，电极还贴着会灌进脑电 */
	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CTRL_REG = 0;

	if (CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK0){
		WAVE_GEN_DRVC_BLK5->WAVE_GEN_DRV_ISEL_REG=0;	// BLK5 driver C is for pair 0,low 3 bit for unit of current
		}
	else if(CMSDK_WAVEGEN_DRVA==WAVE_GEN_DRVA_BLK1){
		WAVE_GEN_DRVC_BLK6->WAVE_GEN_DRV_ISEL_REG=0; // BLK6 driver C is for pair 1, low 3 bit for unit of current
	}
	CMSDK_WAVEGEN_DRVA ->WAVE_GEN_DRV_ISEL_REG = 0;  //???? 0x00 - 0x07
	
	for(int i=0; i<64; i++){
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i; 
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = 0; //???0xff
    }


}
 void Delay_ms( uint16_t time_ms )
{
  uint16_t i,j;
	if(time_ms<5000) //?????????????????????????????????????????????
	{
		time_ms=5000;
	}
	
  for( i=0;i<time_ms;i++ )
  {
		for( j=0;j<1001;j++ );
  }
}

//?????????
 void SysHseSet(void)
{
		
		CMSDK_GPIO->IE = (CMSDK_GPIO->IE & ~(0x01ul << 0)) | (0x01 << 0); //GPIO0??????
		CMSDK_GPIO->ALTFL = (CMSDK_GPIO->ALTFL & ~(0x03ul << 0)) | (0x02 << 0); //GPIO0?????HSE_CLK
		CMSDK_SYSCON->CLK_CFG  = (CMSDK_SYSCON->CLK_CFG & ~CMSDK_SYSCON_SYSCLK_SEL_Msk) | (0x1 << CMSDK_SYSCON_SYSCLK_SEL_Pos); // ???HSE??????
		while (((CMSDK_SYSCON->CLK_CFG & CMSDK_SYSCON_SYSCLK_SWSTS_Msk) >> CMSDK_SYSCON_SYSCLK_SWSTS_Pos) != 0x01) { }       //???HSE??????????????????????????GPIO0
		CMSDK_SYSCON->HSI_CTRL = (CMSDK_SYSCON->HSI_CTRL & ~CMSDK_SYSCON_HSI_EN_Msk); //HSI OSC OFF
}

//???????????????
 void SwitchHseFun(uint8_t aSec)
{

		if(aSec <5) aSec=5;
		if(30<aSec) aSec=30;
		uint32_t aDelayTime = aSec*1000;
		Delay_ms(aDelayTime);		//???5s???GPIO?????HSE????	
	
		SysHseSet();
}

/* now、last 都取自 g_tick_ms。last 比 now 新（两次读取跨过 1ms，或计数回绕）时返回 0，
   调用方继续等满窗，不会把倒退当成超长间隔去判脱落。 */
static uint32_t lod_age_ms(uint32_t now, uint32_t last)
{
    if ((int32_t)(now - last) < 0) {
        return 0;
    }
    return now - last;
}

/* 每 100us 读一次 GPIO19，只记最长连续低电平。
   闭路时脚被拉在高门限以下，这段低电平贯穿整个周期；开路毛刺撑不满 3ms。
   g_tick_ms 仍是毫秒，给统计窗用。 */
void SysTick_Handler(void)
{
    uint8_t level;

    if (++s_sub_ms >= LOD_TICKS_PER_MS) {
        s_sub_ms = 0;
        g_tick_ms++;
    }
    if (!g_stim_running) {
        return;
    }
    level = GPIO_GetInputValue(LOD_GPIO_CH0);
    s_sample_cnt++;
    if (level == 0) {
        s_low_run++;
        if (s_low_run > s_max_low_run) {
            s_max_low_run = s_low_run;
        }
    } else {
        s_low_run = 0;
    }
}

/* GPIO16–23 共用这个入口，当前只开了 GPIO19 的上升沿和下降沿。
   两沿挂起不在同一个寄存器，进一次中断可能两沿都置上，都要清掉，否则会反复进来。
   2mA 开路也有窄毛刺，沿本身不代表贴合，脉宽在 SysTick_Handler 里量。
   不走 EXTI_GPIO_Init：那个函数会打开内部下拉，和板上的上拉对着干。 */
void GPIO16_23_Handler(void)
{
    uint32_t rise = CMSDK_EXTI->RISE_PEND_STS;
    uint32_t fall = CMSDK_EXTI->FALL_PEND_STS;

    /* 先读再清。写 1 只清对应位，别的 GPIO 挂起不动 */
    if (fall & (1u << EXTI_GPIO19)) {
        CMSDK_EXTI->FALL_PEND_CLR = (1u << EXTI_GPIO19);
    }
    if (rise & (1u << EXTI_GPIO19)) {
        CMSDK_EXTI->RISE_PEND_CLR = (1u << EXTI_GPIO19);
    }
}

/* 统计窗：两个刺激周期，且不少于 LOD_WIN_MIN_MS。频率低于 1Hz 按 1Hz，避免除零。 */
static uint32_t lod_window_ms(uint8_t freq_hz)
{
    uint32_t win;
    if (freq_hz < 1) {
        freq_hz = 1;
    }
    win = 2000u / freq_hz;
    if (win < LOD_WIN_MIN_MS) {
        win = LOD_WIN_MIN_MS;
    }
    return win;
}

/* 主动上报给主控，不再等小程序来查。0xFF=开机待机，1=刺激中因脱落进入待机。
   主控收到后若蓝牙已连接，把这一帧转给小程序。 */
static void lod_uplink(uint8_t state)
{
    send_buf[0] = 0xA5;
    send_buf[1] = 0x0A;
    send_buf[2] = 0x02;
    send_buf[3] = state;
    send_buf[4] = 0;
    send_buf[5] = crc8_calc(send_buf, 5);
    send_buf[6] = 0x5A;
    Uart_Send(CMSDK_UART1, send_buf, 7);
}

/* 脱落时停输出并进入待机，同时上发一次。已经停过就直接返回。 */
static void lod_cut_output(void)
{
    if (s_open_latched) {
        return;
    }
    s_open_latched = 1;
    wavegen_Stop(WAVE_GEN_DRVA_BLK0);
    g_stim_running = 0;
    g_lead_off_ch0 = 0xFF;
    g_lead_off_ch2 = 0;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    lod_uplink(1);
}

/* 刺激开始：先报贴合，计数清零，并空出整整一个统计窗再允许判定。
   刚启动时波形还没起来，立刻统计会把空窗当成开路。 */
static void lod_mark_stim_start(void)
{
    uint32_t now = g_tick_ms;

    g_stim_running = 1;
    g_lead_off_ch0 = 0;
    g_lead_off_ch2 = 0;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    s_duty_tick = now;
    s_lod_arm_tick = now + lod_window_ms((uint8_t)g_freq);
}

/* 上位机停刺激。没有光耦电流，不能再报贴合或脱落，状态回到 0xFF，并放开脱落锁存。 */
static void lod_mark_stim_stop(void)
{
    g_stim_running = 0;
    g_lead_off_ch0 = 0xFF;
    g_lead_off_ch2 = 0;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    s_open_latched = 0;
}

/* GPIO19/GPIO22 配成无上下拉输入。只打开 GPIO19 双边沿；GPIO22 本版不参与判定。 */
void LOD_Init(void)
{
    GPIO_AnalogChannel_Control(LOD_GPIO_CH0, DISABLE);
    GPIO_AnalogChannel_Control(LOD_GPIO_CH2, DISABLE);
    GPIO_AltFunction(LOD_GPIO_CH0, ALT_FUNC0);
    GPIO_AltFunction(LOD_GPIO_CH2, ALT_FUNC0);
    GPIO_IO_Init(LOD_GPIO_CH0, INPUT, GPIO_OType_PP, GPIO_NOPULL,
                 OUTPUT_FAST, PDRV_4mA, ENABLE);
    GPIO_IO_Init(LOD_GPIO_CH2, INPUT, GPIO_OType_PP, GPIO_NOPULL,
                 OUTPUT_FAST, PDRV_4mA, ENABLE);
    /* 先关中断并清掉上电残留的沿挂起，再开下降沿、上升沿和中断线，最后才进 NVIC */
    NVIC_DisableIRQ(GPIO16_23_IRQn);
    NVIC_ClearPendingIRQ(GPIO16_23_IRQn);
    CMSDK_EXTI->FALL_PEND_CLR = (1u << EXTI_GPIO19);
    CMSDK_EXTI->RISE_PEND_CLR = (1u << EXTI_GPIO19);
    CMSDK_EXTI->FTSR |= (1u << EXTI_GPIO19);
    CMSDK_EXTI->RTSR |= (1u << EXTI_GPIO19);
    CMSDK_EXTI->IMR |= (1u << EXTI_GPIO19);
    NVIC_EnableIRQ(GPIO16_23_IRQn);
    /* 关掉上次比较器试验留下的脱落开关，避免它把输出钳在片内 */
    CMSDK_ANAC->COMP0_CTRL &= ~(1u << 0);
    WAVE_GEN_DRVC_BLK5->WAVE_GEN_DRV_CTRL_REG &= ~1u;
    WAVE_GEN_DRVC_BLK7->WAVE_GEN_DRV_CTRL_REG &= ~1u;
    SysTick_Config(APB_Clock_Freq / LOD_SAMPLE_HZ);
}

/* 主循环调用。未刺激报 0xFF；已锁存脱落则保持停输出。
   满窗之后：最长连续低电平达到 LOD_LOW_MIN_MS 判贴合，否则判脱落并切断。 */
void LOD_Proc(void)
{
    uint32_t now = g_tick_ms;
    uint32_t win;
    uint32_t n;
    uint32_t max_run;

    /* 没在输出时没有光耦电流，不能报贴合 */
    if (!g_stim_running) {
        g_lead_off_ch0 = 0xFF;
        g_lead_off_ch2 = 0;
        return;
    }

    if (s_open_latched) {
        g_lead_off_ch0 = 1;
        return;
    }

    if ((int32_t)(now - s_lod_arm_tick) < 0) {
        g_lead_off_ch0 = 0;
        g_lead_off_ch2 = 0;
        return;
    }

    win = lod_window_ms((uint8_t)g_freq);
    if (lod_age_ms(now, s_duty_tick) < win) {
        return;
    }

    __disable_irq();
    n = s_sample_cnt;
    max_run = s_max_low_run;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    __enable_irq();
    s_duty_tick = now;

    /* 样本不够不判，避免半窗被当成开路 */
    if (n < (win * LOD_TICKS_PER_MS / 2u)) {
        return;
    }

    g_lead_off_ch0 = (max_run < (LOD_LOW_MIN_MS * LOD_TICKS_PER_MS)) ? 1 : 0;
    g_lead_off_ch2 = 0;
    if (g_lead_off_ch0 == 1) {
        lod_cut_output();
    }
}

int main(){
	int count;
	Delay_268ms(1);
	MTP_init();
	ClockInit();
	/*uart*/
	
///****************FIFO??????*************************/
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
	
	
/****************FIFO?????*************************/
//    UART_Init(CMSDK_UART1, &UART1_Init);
//	//??????????FIFO????????
//	//  UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
//	while(1)
//	{  	
//		//?????????????
//		if((CMSDK_UART1->LSR & 0x01)== 0x01)
//		{
//			//???FIFO????????????
//			GetParamNumber=(CMSDK_UART1->FSR>>16) & 0x1f;
//			for(uint32_t i=0;i<GetParamNumber;i++)
//			{
//				Get_Data=CMSDK_UART1->RBR;//???????
//				UartPutc(CMSDK_UART1,Get_Data);//????????
//			}
//		}
//	}
/****************************************************/


	uint8_t  data[8] = {1,2,3,4,5,6,7,8};
	
	UART_Init(CMSDK_UART1, &UART1_Init);
	
    UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
	
	
	CMSDK_SYSCON->APB_CLKEN = 0x4000|0x1000|CMSDK_SYSCON->APB_CLKEN;
	//?????1-20%??=80% ,256??? ,10.5A ,26V,??????,???,????????????   no_??mos	
		//????64???  10.5A   26V  ??????   
		CMSDK_ANAC->BOOST_CTRL =0x77213;
		CMSDK_ANAC->PMU_CTRL = 0x10;
	LOD_Init();
	lod_uplink(0xFF);

	while(1){
		LOD_Proc();
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
					lod_mark_stim_start();
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
					lod_mark_stim_stop();
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
					lod_mark_stim_stop();
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
					if(rev_buf[5] != 0 && g_enable_NNC6121_HSE != 1){
						g_enable_NNC6121_HSE = 1;
						
						//SwitchHseFun(5);
						
						Delay_268ms(1);SysHseSet();
					}
					
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
				}
		    
			  rev_len = 0;
			  rev_fin = 0;
		}
		
	}



}


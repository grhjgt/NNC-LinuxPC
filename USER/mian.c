/*
*Copyright ,2024 , NANOCHAP
*File name:   MIAN.C          
*Author:      
*Version:     V1.0 
*Date:        2024-11-                  
*Description:   UART1打印 / 接收数据
*Function List: 
   
History: 
1.V1.0
Date:          
Author: 
Modification: 初版

只有全功能开发板具备UART0和UART1，双通道和四通道开发板只有UART1,GPIO12--RX,GPIO13--TX

结构体定义详情
串口FIFO结构体定义
typedef struct uart_fifo{
	uartRcvTrigger  level;  接收FIDO触发电平的数据数量，1，4，8，14bytes
	bool DMA_Enable  ;		DMA使能？
	bool FIFO_Enable ;		FIFO使能
}UART_FifoStructrue;		

串口初始化结构体定义
typedef struct uart_para{		
	 uint32_t UART_BaudRate;				串口波特率
	 bool UART_HardwareFlowControl;		    硬件流控是否开启
	 UART_FifoStructrue*  FifoSetting ;		串口FIFO结构体指针
	 
	 UART_StopLen stop_len;					停止位
	 UART_WordLen Word_len;					数据位
	 UART_Stick_EN Stick_EN;				固定奇偶校验启用
	 UART_Even_EN Even_EN;					择奇偶校验选择
	 UART_Parity_EN Parity_EN;				使能奇偶校验
}UART_InitStructure;

停止位配置：停止位有三种选择，分别为1个停止位，1.5个停止位，2个停止位
停止位由UART_InitStructure结构体下的stop_len和Word_len共同控制
1、当stop_len为StopLen_1时，停止位为1，与Word_len无关
2、当stop_len为StopLen_Other，Word_len为WordLen_5时，停止位为1.5位
3、当stop_len为StopLen_Other，Word_len不为WordLen_5时，停止位为2位


校验位配置：校验位共有五种选择。分别为无校验（None）,奇校验（Odd），偶校验（Even），固定1校验（Mark），固定0校验（Space）
校验位由UART_InitStructure结构体下的Stick_EN,Even_EN和Parity_EN三个参数共同控制
1、当Parity_EN为Parity_RESET时，禁用奇偶校验位
2、当Parity_EN为Parity_SET，Stick_EN为Stick_RESET，Even_EN为Odd_SET时，启用奇校验位
3、当Parity_EN为Parity_SET，Stick_EN为Stick_RESET，Even_EN为Even_SET，启用偶校验位
4、当Parity_EN为Parity_SET，Stick_EN为Stick_SET，Even_EN为Odd_SET，启用固定1校验位
5、当Parity_EN为Parity_SET，Stick_EN为Stick_SET，Even_EN为Even_SET，启用固定0校验位


关于波特率配置使用到的
divisor_value = (uint16_t)((uint32_t)(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1);
CMSDK_UART->DLL = (uint8_t)(divisor_value & 0x00FF);
CMSDK_UART->DLH = (uint8_t)((divisor_value & 0xFF00)>>8);
可通过以下方式理解：
1、波特率的含义为1秒内传输位的个数，如115200波特率，代表1秒内可以传输115200个位，每个位的传输时间为1/115200秒

2、外设主频的含义为1秒内的时钟个数，如32MHz的主频，代表1秒内发生32M个时钟周期，每个时钟周期时间为1/32000000秒

3、对串口设置波特率即为设置对应波特率每个位传输时间占外设主频的多少个时钟周期，并把这个值高八位存放于CMSDK_UART->DLH，低八位存放于CMSDK_UART->DLL中。

4、overSamp_mode，由OSM_SEL寄存器决定，0时为13，1时为16，可理解为对外设主频进行第一次分频的分频系数，APB_Clock_Freq为外设主频，uart_paraX->UART_BaudRate为配置波特率

5、所以(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1)中（APB_Clock_Freq / overSamp_mode）可以看作首先对外设时钟进行第一次分频，
结果为串口实际主频，此时串口时钟为1秒（APB_Clock_Freq / overSamp_mode）个时钟周期，把它除以波特率，即可算出该波特率配置时一个位传输时间需要多少个串口主频时钟周期，
即(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)，且寄存器为档位调节，值为0时为第1档，
所以第(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)档，需要在寄存器中写入(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode)-1。

6、问题：由于计算出来的时钟数结果很难是整数，且强制转换类型取整的原因，会造成些许误差，如计算出来的结果为15.9，取整后会得到15，造成误差，使串口实际波特率与设置波特率误差较大，
造成错误，所以当发现串口波特率误差较大时，请在公式计算结束后尝试加一或减一，再进行尝试，挑选一个更加优秀的结果。

7、局限，当外设主频较低，且波特率较高时，波特率误差会很大，且无法调整，例：主频4M，波特率设置115200，则计算结果为：4000000/115200/16-1=1.17 ,取整后值为1，
此时无法对值进行加一或减一操作（值越小，调整值对实际波特率影响越大），所以当使用较低主频时，使用低波特率可以有效提高串口通信可靠性。

*/  

#include "my_header.h"
#include "ENS1_MTP.h"
#include "ENS1_CLOCK.h"
#include "ENS1_ANAC.h"
#include "ENS1_UART.h"

uint32_t GetParamNumber=0;//获取FIFO数据缓存数量
uint8_t Get_Data=0;//获取参数

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
// 正弦波数组
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

    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_REST_T_REG = 0x00000000;   //死区时间
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_SILENT_T_REG = 0;  //静默时间
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CLK_FREQ_REG = 0x00000020;     //32MHZ ==PCLK
	  freq = (500.00/g_freq)*1000.00;
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = freq;   ; //200us  //正半轴脉宽
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = freq;  ; //500us  //负半
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CONFIG_REG = 0x0000004A; //bit 0:rest enable  ， 正半周和负半轴中间的 休息时间 
															  //bit1:negative enable  ， 负半周期发生使能，如果不使能，则没有负半轴的波形
	                                                          //bit2: silent enable   ， 负半周期结束后的静默时间
                                                              //bit3: source B enable 
															  //bit4: alternating the positive side ,正极交替，
                                                              //bit5: continue mode   ，
	                                                          //bit6: multi-electrode ，
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
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = (int)( (float)sine_data[i]*0.6455 *(g_current/20) ) ;  //描点数组，根据数组中的点描绘正半周期的波形
    }
   
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_LIM_REG = 0x00000000;//  无交替 
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_SILENT_LIM_REG = 0x00000000; // 交替后静默的时钟数。在这种情况下，驱动器b连续交替
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_DELAY_LIM_REG = 0x00000000;  // 延迟时钟数
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_SCALE_REG = 0x00000001;  //倍乘正半周期幅值，如超过255，则从0开始增长
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_OFFSET_REG = 0x00000000; //偏移正半周期幅值，如超过255，则从0开始增长，
                                                                  //如正半周期幅值为250，此处设置为10，则负半周期幅值为5 ！！！
		
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_INT_REG = 0x0;//  interrupt register set to 0
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CTRL_REG = 0x00000001; //使能 driver

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
	CMSDK_WAVEGEN_DRVA ->WAVE_GEN_DRV_ISEL_REG = 0;  //范围 0x00 - 0x07
	
	for(int i=0; i<64; i++){
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i; 
       	CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = 0; //最大0xff
    }


}
 void Delay_ms( uint16_t time_ms )
{
  uint16_t i,j;
	if(time_ms<5000) //防呆处理，防止调用者传入更小的时间导致无法及时下载程序。
	{
		time_ms=5000;
	}
	
  for( i=0;i<time_ms;i++ )
  {
		for( j=0;j<1001;j++ );
  }
}

//外部时钟设置
 void SysHseSet(void)
{
		
		CMSDK_GPIO->IE = (CMSDK_GPIO->IE & ~(0x01ul << 0)) | (0x01 << 0); //GPIO0输入模式
		CMSDK_GPIO->ALTFL = (CMSDK_GPIO->ALTFL & ~(0x03ul << 0)) | (0x02 << 0); //GPIO0复用为HSE_CLK
		CMSDK_SYSCON->CLK_CFG  = (CMSDK_SYSCON->CLK_CFG & ~CMSDK_SYSCON_SYSCLK_SEL_Msk) | (0x1 << CMSDK_SYSCON_SYSCLK_SEL_Pos); // 选中HSE作为时钟
		while (((CMSDK_SYSCON->CLK_CFG & CMSDK_SYSCON_SYSCLK_SWSTS_Msk) >> CMSDK_SYSCON_SYSCLK_SWSTS_Pos) != 0x01) { }       //等待HSE起振完毕，这会等待外部晶振引脚接入GPIO0
		CMSDK_SYSCON->HSI_CTRL = (CMSDK_SYSCON->HSI_CTRL & ~CMSDK_SYSCON_HSI_EN_Msk); //HSI OSC OFF
}

//切到外部时钟功能函数
 void SwitchHseFun(uint8_t aSec)
{

		if(aSec <5) aSec=5;
		if(30<aSec) aSec=30;
		uint32_t aDelayTime = aSec*1000;
		Delay_ms(aDelayTime);		//等待5s后把GPIO复用为HSE引脚	
	
		SysHseSet();
}
int main(){
	int count;
	Delay_268ms(1);
	MTP_init();
	ClockInit();
	/*uart*/
	
///****************FIFO中断模式*************************/
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
	
	
/****************FIFO轮询模式*************************/
//    UART_Init(CMSDK_UART1, &UART1_Init);
//	//不使能中断，FIFO默认轮询模式
//	//  UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
//	while(1)
//	{  	
//		//数据位准备就绪
//		if((CMSDK_UART1->LSR & 0x01)== 0x01)
//		{
//			//获取FIFO缓存数据数量
//			GetParamNumber=(CMSDK_UART1->FSR>>16) & 0x1f;
//			for(uint32_t i=0;i<GetParamNumber;i++)
//			{
//				Get_Data=CMSDK_UART1->RBR;//获取数据
//				UartPutc(CMSDK_UART1,Get_Data);//返回数据
//			}
//		}
//	}
/****************************************************/


	uint8_t  data[8] = {1,2,3,4,5,6,7,8};
	
	UART_Init(CMSDK_UART1, &UART1_Init);
	
    UART_ITConfig(CMSDK_UART1, &UART1_ITSet);
	
	
	CMSDK_SYSCON->APB_CLKEN = 0x4000|0x1000|CMSDK_SYSCON->APB_CLKEN;
	//占空比（1-20%）=80% ,256分频 ,10.5A ,26V,关闭过温,过压,开启过流保护   no_外部mos	
		//占空比64分频  10.5A   26V  关闭过温   
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
				}
		    
			  rev_len = 0;
			  rev_fin = 0;
		}
		
	}



}


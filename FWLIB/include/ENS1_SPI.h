/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_SPI.H         
*Author:      
*Version:     V1.0 
*Date:        2023-11-                  
*Description:  
*Function List: 
    1  uint8_t 			READ_SPI_MODE(CMSDK_SPI_TypeDef* SPIx)
    2  uint8_t 			CLR_TX_FIFO(CMSDK_SPI_TypeDef* SPIx)
    3  uint8_t 			CLR_RX_FIFO(CMSDK_SPI_TypeDef* SPIx) 
    4  uint8_t 			RX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx)        
    5  uint8_t 			TX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx)        
    6  SPI_BUSY_STATE 	BUSY_STATE(CMSDK_SPI_TypeDef* SPIx)  
    7  uint8_t  		RX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx)    
    8  uint8_t 			RX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx)  
    9  uint8_t 			TX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx)  
    10 uint8_t 			TX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx)   
    11 uint8_t 			SPI_FIFO_STATE(CMSDK_SPI_TypeDef* SPIx)
    12 uint8_t 			SPI_FIFO_ENABLE(CMSDK_SPI_TypeDef* SPIx )  
    13 uint8_t 			SPI_FIFO_DISABLE(CMSDK_SPI_TypeDef* SPIx ) 
    14 uint8_t 			SPI_FIFODMA_SET(CMSDK_SPI_TypeDef* SPIx ,bool TXDMA_SET ,bool RXDMA_SET)  
    15 uint8_t 			SPI_NSS_CHANNEL(CMSDK_SPI_TypeDef* SPIx ,NSS_CHANNEL_SEL NSSx ,FunctionalState ENorDIS) 
    16 uint8_t 			SPI_Config_init(CMSDK_SPI_TypeDef* SPIx , struct SPI_ModeConfig_Struct SPI_Config ,struct SPI_FIFO_Struct FIFO_Struct)  
    17 uint8_t 			SPI_START(CMSDK_SPI_TypeDef* SPIx) 
    18 uint8_t 			SPI_STOP(CMSDK_SPI_TypeDef* SPIx)
    19 uint16_t 		READ_SPI_RCVBuff(CMSDK_SPI_TypeDef* SPIx )
    20 void 			WRITE_SPI_THRBuff(CMSDK_SPI_TypeDef* SPIx,uint8_t data)	
    21 uint8_t 			SPI_INT_SET(IRQn_Type IRQn, bool SPI_INT_ENABLE, uint8_t SPI_INT_BIT_SET)
History: 
	1.V1.0
	Date:          
	Author: 
	Modification: 初版
*/ 

#ifndef ENS1_SPI_H
#define ENS1_SPI_H
#include "my_header.h"

/*
一、引脚关系：
ALT Function2 
SPI1_SCK    ---    GPIO16
SPI1_MOSI   ---    GPIO17
SPI1_MISO   ---    GPIO18
SPI1_NSS0   ---    GPIO19
SPI1_NSS1   ---    GPIO2
SPI1_NSS2   ---	   GPIO3
SPI1_NSS3   ---    GPIO4

ALT Function2
SPI0_SCK    ---    GPIO8
SPI0_MOSI   ---    GPIO9
SPI0_MISO   ---    GPIO10
SPI0_NSS0   ---    GPIO11
SPI0_NSS1   ---    GPIO13
SPI0_NSS2   ---	   GPIO14
SPI0_NSS3   ---    GPIO15

二、SPI功能列表说明：
三条线路上的全双工同步传输 
双线半双工同步传输
两线单工同步传输（带单向数据线）
16*16bits FIFO 收发缓冲区
4-16位数据大小选择
主模式波特率发生器高达Fpclk/2
从模式波特率发生器高达Fpclk/4
软件或硬件管理 NSS
可编程时钟极性和相位
可编程的数据顺序与MSB或LSB移位
DMA事件支持
中断支持

*/



typedef enum  {MASTER = 1 ,SLAVE= 0}MASTER_SLAVE_SEL; 
typedef enum  {NSS0= 8 ,NSS1 ,NSS2 ,NSS3}NSS_CHANNEL_SEL ;
typedef enum  {NOTBUSY = 0, BUSY}SPI_BUSY_STATE ;
typedef enum  {EMPTY=0,FULL}FIFO_FULL_EMPTY_STATE;

struct SPI_ModeConfig_Struct
{
	uint8_t BAUD_FPCLKdivx ;    //波特率分频系数
	uint8_t SPI_MODE ;          //SPI工作模式
	uint8_t SPI_TRANS_MODE;     //传输模式选择
	MASTER_SLAVE_SEL  MS_SEL;   //主从模式选择
	uint16_t CHAR_LEN ;         //设置传输长度 （4 - 16 bit）
	NSS_CHANNEL_SEL NSSx ;
	uint8_t SAMP_PHASE ;
	
};
struct SPI_FIFO_Struct    //设置FIFO及DMA传输
{
	uint8_t TX_FIFO_TH;   //  0 - 16  char
	uint8_t RX_FIFO_TH;   //  0 - 16  char
	bool FIFO_ENABLE_SET;
	bool TXDMA_SET;       //选择是否启动DMA(fifo开启的前提下)
	bool RXDMA_SET;	
};

/* 中断类型使能结构体
1、发送部分有 下溢 中断（发送数据没有啦）
2、接收部分有 溢出 中断（接满啦）
3、收发完成中断？
4、发送缓冲区空 中断
5、接收缓冲区非空 中断
*/
//中断使能
#define  UNDERRUN_INT_EN   (uint8_t)0x10
#define  OVERRUN_INT_EN    (uint8_t)0x8
#define  CMPL_INT_EN       (uint8_t)0x4
#define  TXE_INT_EN        (uint8_t)0x2
#define  RXNE_INT_EN       (uint8_t)0x1

//判断是否检测到对应的中断
#define UNDERRUN_INT   (uint8_t)0x10
#define OVERRUN_INT    (uint8_t)0x8
#define CMPL_INT       (uint8_t)0x4
#define TXE_INT        (uint8_t)0x2
#define RXNE_INT       (uint8_t)0x1


/*模式：                   |  接线方式： 主机             从机  
全双工                     |            MISO/MOSI         MISO/MOSI
半双工                     |            MOSI              MISO
主机仅发送，从机仅接收模式  |            MOSI              MOSI
主机仅接收，从机仅发送模式  |            MISO              MISO

  SPI_TRANS_MODE 传输模式选择：
 2线单向                       /      1线双向
 收+发 / 仅发 /仅收/                     单向仅发送 / 仅接收
*/
#define L2_UniDirect_TandR  (uint8_t)0x0 //BIT[15:12] 0 0 00  
#define L2_UniDirect_T      (uint8_t)0x1 //BIT[15:12] 0 0 01
#define L2_UniDirect_R      (uint8_t)0x2 //BIT[15:12] 0 0 10
#define L1_BiDirect_T  (uint8_t)0x8 //BIT[15:12] 1 0 00
#define L1_BiDirect_R  (uint8_t)0xc//BIT[15:12] 1 1 00

/*NSS相关设置*/
#define NSS_PULSE      1    //有nss
#define NO_NSS_PULSE   0    //没有nss
#define	NSS_ASSERTED   0    //
#define NSS_DEASSERYED 1    //软件发出nss信号
#define NSS_CTRL_HW    0    //设置为硬件生成NSS
#define NSS_CTRL_SW    1    //设置为软件生成NSS

/*波特率设置*/
#define BAUD_FPCLKdiv2    (uint8_t)0x0
#define BAUD_FPCLKdiv4    (uint8_t)0x1
#define BAUD_FPCLKdiv8    (uint8_t)0x2
#define BAUD_FPCLKdiv16   (uint8_t)0x3
#define BAUD_FPCLKdiv32   (uint8_t)0x4
#define BAUD_FPCLKdiv64   (uint8_t)0x5
#define BAUD_FPCLKdiv128  (uint8_t)0x6
#define BAUD_FPCLKdiv256  (uint8_t)0x7

/*SPI_MODE 工作模式选择*/
#define SPI_MODE0       (uint8_t)0x0   //bit[3:2]  00
#define SPI_MODE1       (uint8_t)0x1   //  01
#define SPI_MODE2       (uint8_t)0x2   //  10
#define SPI_MODE3       (uint8_t)0x3   //  11


/*SAMP_PHASE 选项*/
#define PRE_1_PCLK_PERIOD  (uint8_t)0X0
#define SAMP_PHASE_NORMAL  (uint8_t)0X1
#define DELAY_1_PCLK_PERIOD  (uint8_t)0X2
#define DELAY_2_PCLK_PERIOD  (uint8_t)0X3

/*****************************声明 和 定义***************************/
#define SPI0_CS_SET         GPIO_SetOutput(GPIO_11)
#define SPI0_CS_RESET       GPIO_ResetOutput(GPIO_11)
#define SPI1_CS_SET         GPIO_SetOutput(GPIO_19)
#define SPI1_CS_RESET       GPIO_ResetOutput(GPIO_19)


//读SPI当前设置的模式
extern uint8_t READ_SPI_MODE(CMSDK_SPI_TypeDef* SPIx);
/*清除FIFO和计数清0*/
extern uint8_t CLR_TX_FIFO(CMSDK_SPI_TypeDef* SPIx);
extern uint8_t CLR_RX_FIFO(CMSDK_SPI_TypeDef* SPIx) ; 
/*FIFO 状态读取*/
extern uint8_t RX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx);          //读取当前接收FIFO数据长度
extern uint8_t TX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx)      ;    //读取当前发送FIFO数据长度
extern SPI_BUSY_STATE BUSY_STATE(CMSDK_SPI_TypeDef* SPIx) ;          //读取当前SPI是否繁忙
extern uint8_t  RX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx) ;    //当前读取FIFO是否为满？
extern uint8_t RX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx) ;  //当前读取FIFO是否为空？
extern uint8_t TX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx)   ;//当前发送FIFO是否为满？
extern uint8_t TX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx)   ; //当前发送FIFO是否为空？
extern uint8_t SPI_FIFO_STATE(CMSDK_SPI_TypeDef* SPIx);
extern uint8_t SPI_FIFO_ENABLE(CMSDK_SPI_TypeDef* SPIx );  //FIFO功能使能
extern uint8_t SPI_FIFO_DISABLE(CMSDK_SPI_TypeDef* SPIx ); //FIFO功能失能
extern uint8_t SPI_FIFODMA_SET(CMSDK_SPI_TypeDef* SPIx ,bool TXDMA_SET ,bool RXDMA_SET);  //DMA设置
extern uint8_t SPI_NSS_CHANNEL(CMSDK_SPI_TypeDef* SPIx ,NSS_CHANNEL_SEL NSSx ,FunctionalState ENorDIS); //nss通道选择
extern uint8_t SPI_Config_init(CMSDK_SPI_TypeDef* SPIx , struct SPI_ModeConfig_Struct SPI_Config ,struct SPI_FIFO_Struct FIFO_Struct);  //spi的初始配置 
extern uint8_t SPI_START(CMSDK_SPI_TypeDef* SPIx); 
extern uint8_t SPI_STOP(CMSDK_SPI_TypeDef* SPIx);
//读被接收的数据 最多16bits
extern uint16_t READ_SPI_RCVBuff(CMSDK_SPI_TypeDef* SPIx );
//写数据
extern void WRITE_SPI_THRBuff(CMSDK_SPI_TypeDef* SPIx,uint8_t data);	
extern uint8_t SPI_INT_SET(IRQn_Type IRQn, bool SPI_INT_ENABLE, uint8_t SPI_INT_BIT_SET);

#endif

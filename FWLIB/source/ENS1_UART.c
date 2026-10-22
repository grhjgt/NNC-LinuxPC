/*
*Copyright (C),2023 , NANOCHAP
*File name:   ENS1_UART.C          
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

/*
ENS1 uart特性说明
1 符合AMBA APB规范
2 最高115200bps波特率(可以有更高的设置)
3 发送 接收 分离FIFO （16字节）
4 支持自动流控
5 标准异步通信位(开始、停止、奇偶校验)
6 DMA
7 支持回环测试
8 中断
9 全可编程串行接口特点:
  数据位可设置： 5，6，7，8
  偶、奇或非奇偶位的生成和检测
  可产生1、1.5或2位停止位

*/
#include "my_header.h"
#include "ENS1_UART.h"
#include "ENS1_GPIO.h"
UART_FifoStructrue UART1_Fifo = {
	.level = bytes_8 ,
	.DMA_Enable = false ,
	.FIFO_Enable = true ,
};
UART_InitStructure UART1_Init = {
	.UART_BaudRate = 9600 ,        //计算出来的DLL DLH不为整数， 1、需要校准RC精确度，2、需要调整此处的波特率
	.UART_HardwareFlowControl = false,
	.FifoSetting = 	&UART1_Fifo ,	
	.stop_len = StopLen_1,		//固定停止位1位
	.Word_len = WordLen_8,		//数据位8位
	.Stick_EN = Stick_RESET,	//禁用固定奇偶校验
	.Even_EN = Even_SET,		//奇偶校验选择
	.Parity_EN = Parity_RESET,	//禁用奇偶检验
};

UART_ITStructure UART1_ITSet = {
	.UartIntModel = RLSI_EN | RDAI_EN ,
};

/*判断是否有中断挂起*/
uint8_t UART_INT_PEND(CMSDK_UART_TypeDef* UARTx)  //为0时有UART的中断挂起 
{
	return (uint8_t)(UARTx->IIR & 0x1);
}
/*中断类型判断*/
uint8_t UART_INT_TYPE(CMSDK_UART_TypeDef* UARTx)
{
	return (uint8_t)((UARTx->IIR >> 1) & 0x7 );
}
/*fifo 使用指示器*/
uint8_t UART_FIFO_USE(CMSDK_UART_TypeDef* UARTx)  //0:非fifo模式  1：fifo 使能
{
	return (uint8_t)((UARTx->IIR >> 6) & 0x3);
}

/*清除传输FIFO*/
void UART_TXCLR(CMSDK_UART_TypeDef* UARTx)
{
	UARTx->FCR |= (1<<2) ;
}
/*清除接收FIFO*/
void UART_RXCLR(CMSDK_UART_TypeDef* UARTx)
{
	UARTx->FCR |= (1<<1);
}
/*FIFO使能*/
void UART_FIFOEnable(CMSDK_UART_TypeDef* UARTx)
{
	UARTx->FCR |= (1);
}
/*FIFO关闭*/
void UART_FIFODisable(CMSDK_UART_TypeDef* UARTx)
{
	UARTx->FCR &=~ 1;
}

/*接收数据*/
uint8_t READ_UART_RCVBuff(CMSDK_UART_TypeDef* UARTx )
{
	return (uint8_t)(UARTx->RBR & 0xff);
}

/*发送数据*/
void WRITE_UART_THRBuff(CMSDK_UART_TypeDef* UARTx ,uint8_t data)
{
	UARTx->THR = data;
}

/*FIFO 状态读取*/
uint8_t UART_RX_FIFO_LEN(CMSDK_UART_TypeDef* UARTx)  
{
	return (uint8_t)((UARTx->FSR & 0x001f0000)>>16);        //读取当前接收FIFO数据长度
}

uint8_t UART_TX_FIFO_LEN(CMSDK_UART_TypeDef* UARTx)    
{
	return (uint8_t)((UARTx->FSR & 0x00001f00)>>8);	       //读取当前发送FIFO数据长度
}	

uint8_t  UART_RX_FIFO_FULL(CMSDK_UART_TypeDef* UARTx) 
{
	return 	(uint8_t)((((UARTx->FSR & 0x8)>> 3 )==1) ?  (1) : (0)) ;          //当前读取FIFO是否为满？
}	

uint8_t UART_RX_FIFO_EMPTY(CMSDK_UART_TypeDef* UARTx) 
{	
	return 	(uint8_t)((((UARTx->FSR & 0x4) >> 2)==1) ?    (1) : (0)  ) ;          //当前读取FIFO是否为空？
}	

uint8_t UART_TX_FIFO_FULL(CMSDK_UART_TypeDef* UARTx)  
{
	return 	(uint8_t)((((UARTx->FSR & 0x2) >> 1)==1) ?  (1) : (0)) ;         //当前发送FIFO是否为满？
}	

uint8_t UART_TX_FIFO_EMPTY(CMSDK_UART_TypeDef* UARTx)  
{
	return (uint8_t)(((UARTx->FSR & 0x1)==1) ? (1) : (0)) ;          //当前发送FIFO是否为空？
}


/*收发线状态获取*/
uint8_t UARTLine_RCVError(CMSDK_UART_TypeDef* UARTx)
{
	return (uint8_t)((UARTx->LSR>>7) & 0x1);      //返回0 没有错误， 返回1 在fifo和非fifo状态下都有一个校验/帧/或接收缓存或fifo 的中断指示（默认不使能break）
}

/*判断发送缓存是否为空*/
uint8_t UARTLine_TRANSEmpty(CMSDK_UART_TypeDef* UARTx)
{
	return (uint8_t)((UARTx->LSR>>6) & 0x1) ;
}

/*判断THR是否为空*/
uint8_t UARTLine_THREmpty(CMSDK_UART_TypeDef* UARTx)
{
	return (uint8_t)((UARTx->LSR>>5) & 0x1) ;
}

/*
初始化UART需要以下步骤:
1. 执行必要的设备引脚多路复用设置。
2. 通过将适当的时钟除数值写入除数锁存寄存器(DLL和DLH)来设置所需的波特率。
3. 如果要使用FIFO，请选择所需的触发等级，并通过将适当的值写入(FCR)来启用FIFO。
   在FCR寄存器的 FIFOEN 位需首先配置。
4. 通过向行控制寄存器(LCR)写入适当的值来选择所需的协议设置。
5. 如果需要自动流量控制，则将适当的值写入调制解调器控制寄存器(MCR)。
   请注意，并非所有uart都支持自动流量控制。
6. 对挂起事件的选择所需响应通过配置FREE位，
   通过在(PMU)寄存器中设置TXRST和RXRST位来释放位并使能UART
*/


//uart初始化，参数1：uart0/1，参数2：uart参数设置结构体
void UART_Init(CMSDK_UART_TypeDef *CMSDK_UART, UART_InitStructure* uart_paraX){
	uint16_t divisor_value;
	uint8_t overSamp_mode;
	/* 1 GPIO - alt_function*/
	if(CMSDK_UART == CMSDK_UART0){
		PCLK_Enable(UART0_PCLK_EN);	
		GPIO_AltFunction(UART0_RX , ALT_FUNC1);   //rx
		GPIO_AltFunction(UART0_TX , ALT_FUNC1);   //tx
	}
	else if(CMSDK_UART == CMSDK_UART1)
	{
		PCLK_Enable(UART1_PCLK_EN);	
		GPIO_AltFunction(UART1_RX , ALT_FUNC1);  //RX
		GPIO_AltFunction(UART1_TX , ALT_FUNC1);  //TX
	}
	/* 2 波特率设置，过采样模式选择，无需自行配置，使用默认即可*/
	if(((CMSDK_UART->MDR)&0x00000001) == 0)
	{
		overSamp_mode = 16;  //默认0  ，期望波特率=输入时钟频率/(({DLH，DLL}+1) × 16)
	}
	else
	{
		overSamp_mode = 13;		//期望波特率=输入时钟频率/(({DLH，DLL}+1) × 13)
	}
	
	//波特率校准，反推处DLL和DLH，（外设时钟主频/设置波特率/过采样模式）-1，
	divisor_value = (uint16_t)((uint32_t)(APB_Clock_Freq / uart_paraX->UART_BaudRate / overSamp_mode) - 1);
	CMSDK_UART->DLL = (uint8_t)(divisor_value & 0x00FF);
	CMSDK_UART->DLH = (uint8_t)((divisor_value & 0xFF00)>>8);
	
	/*FCR配置 FIFO control*/
	CMSDK_UART->FCR |= (uart_paraX->FifoSetting->level <<6);//接收多少字节后触发中断
	if(uart_paraX->FifoSetting->FIFO_Enable == true)
	{
		CMSDK_UART->FCR |= (1);
	}
	//配置停止位
	CMSDK_UART->LCR &= ~(0x01 << 2);
	CMSDK_UART->LCR |= uart_paraX->stop_len<<2;
	
	//配置数据位
	CMSDK_UART->LCR &= ~(0x03 << 0);
	CMSDK_UART->LCR |= uart_paraX->Word_len<<0;
	
	//配置是否启用校验位
	CMSDK_UART->LCR &= ~(0x01 << 3);
	CMSDK_UART->LCR |= uart_paraX->Parity_EN<<3;
	
	//启用校验位
	if(uart_paraX->Parity_EN==Parity_SET)
	{
		CMSDK_UART->LCR &= ~(0x03 << 4);
		CMSDK_UART->LCR |= uart_paraX->Even_EN<<4;
		CMSDK_UART->LCR |= uart_paraX->Stick_EN<<5;
	}
	
	/*MCR自动流控配置*/
	if(uart_paraX->UART_HardwareFlowControl == true)
	{
		CMSDK_UART->MCR |= (1<<5);
	}

	/*电源管理寄存器PMU 的TXRST和RXRST设置,不需要设置*/
	
	
}

/*UART 的中断设置*/
void UART_ITConfig(CMSDK_UART_TypeDef *CMSDK_UART, UART_ITStructure* uart_paraX)
{
	if(CMSDK_UART == CMSDK_UART1){
		NVIC_DisableIRQ(UART1_IRQn);//Disable NVIC interrupt
		NVIC_ClearPendingIRQ(UART1_IRQn);//Clear Pending NVINVIC_EnableIRQ(UART1_IRQn);//Enable NVIC interrupt	
	}
	else if(CMSDK_UART == CMSDK_UART0){
		NVIC_DisableIRQ(UART0_IRQn);//Disable NVIC interrupt
		NVIC_ClearPendingIRQ(UART0_IRQn);//Clear Pending NVIC interrupt
	}
	/*IER配置，中断使能寄存器*/
	CMSDK_UART->IER |= (uart_paraX->UartIntModel);
	
	if(CMSDK_UART == CMSDK_UART1){
		NVIC_EnableIRQ(UART1_IRQn);
	}
	else if(CMSDK_UART == CMSDK_UART0){
		NVIC_EnableIRQ(UART0_IRQn);
	}
}


unsigned char UartPutc(CMSDK_UART_TypeDef *CMSDK_UART ,unsigned char my_ch)
{
	while (UARTLine_THREmpty(CMSDK_UART) == 0x0); 
	WRITE_UART_THRBuff(CMSDK_UART,my_ch);
	return (my_ch);
}
//串口发送一定字节的数据
void Uart_Send(CMSDK_UART_TypeDef *CMSDK_UART ,uint8_t *data, uint8_t len)
{
	uint8_t t;
	for(t=0;t<len;t++)		//循环发送数据
	{
		while(UARTLine_THREmpty(CMSDK_UART) == 0x0);//等待发送结束		  
		WRITE_UART_THRBuff(CMSDK_UART,data[t]);
	}	 
	while(UARTLine_THREmpty(CMSDK_UART) == 0x0);//等待发送结束
	
}


/*
中断与中断状态清除
1   接收线状态：INT_RCV_LINE_STATUS
    溢出、校验、帧错误或检测到break
    中断清除方式：对于溢出错误，读取LSR 可清除中断，对于校验、帧错误或break，中断需要等待所有的错误数据被读取后才被清除
2   接收数据就绪： INT_RCV_DATA_AVAILABLE
    非FIFO模式下，接收数据准备就绪， fifo模式下，达到触发阈值，如果四个字节时间内没有访问FIFO ,就再次触发
    中断清除方式：非FIFO模式下，RBR被读取 ，  FIFO模式下，FIFO低于触发阈值被清除
3   接收超时：INT_CHAR_TIMEOUT_INDICATION 
    仅仅FIFO模式有效 ， 在最后四个字节的时间内，没有字符从接收器FIFO 中移除或者输入，并且在此期间接收器FIFO中至少有一个字符
    中断清除方式：（1）1个字节从接收FIFO中被读出  （2）一个新的字节到达接收FIFO   （3）PMU寄存器中的URRST 位置1
4   传输保持寄存器空（THR）  INT_THR_EMPTY
    非FIFO模式下： THR空。 FIFO模式下，传输器FIFO空 
    中断清除方式：一个字节被写到 THR
*/

void UART0_Handler(void) {
	uint8_t rev_data = 0;
	NVIC_ClearPendingIRQ(UART0_IRQn);
	//接收线中断 有错误或者break
    if(UART_INT_TYPE(CMSDK_UART0) == INT_RCV_LINE_STATUS) {
    	CMSDK_UART0->IER &= ~CMSDK_UART_IER_RLSI_EN_Msk;
    }
	
	//数据就绪中断
	//接收超时中断
    if((UART_INT_TYPE(CMSDK_UART0) == INT_RCV_DATA_AVAILABLE) || (UART_INT_TYPE(CMSDK_UART0) == INT_CHAR_TIMEOUT_INDICATION)) 
	{
    	CMSDK_UART0->IER &= ~CMSDK_UART_IER_RDAI_EN_Msk;
		rev_data = CMSDK_UART0->RBR; 
		UartPutc(CMSDK_UART0,rev_data);
		CMSDK_UART0->IER |= CMSDK_UART_IER_RDAI_EN_Msk;
    }
    return;
}

void UART1_Handler(void) {
	uint8_t rev_data = 0;
	uint32_t ParamNumber = 0;
	NVIC_ClearPendingIRQ(UART1_IRQn);
	//接收线中断 有错误或者break
    if(UART_INT_TYPE(CMSDK_UART1) == INT_RCV_LINE_STATUS) {
    	CMSDK_UART1->IER &= ~CMSDK_UART_IER_RLSI_EN_Msk;//禁用接收线路状态中断
    }
	
	
	//数据就绪中断													//接收超时中断
    if((UART_INT_TYPE(CMSDK_UART1) == INT_RCV_DATA_AVAILABLE) || (UART_INT_TYPE(CMSDK_UART1) == INT_CHAR_TIMEOUT_INDICATION)) {
		CMSDK_UART1->IER &= ~CMSDK_UART_IER_RDAI_EN_Msk;	
		ParamNumber=(CMSDK_UART1->FSR>>16) & 0x1f;
		for(uint32_t i=0;i<ParamNumber;i++)
		{
					rev_data = CMSDK_UART1->RBR;
					/* 上一帧主循环还没取走，不能覆写。启动帧只有 6 字节，FIFO 要 8 字节才触发，
					   半帧就把 rev_fin 置上会让主循环用旧 CRC 把启动命令丢掉。 */
					if (rev_fin) {
						continue;
					}
					if (rev_data == 0xA5 && rev_len == 0) {
							rev_buf[0] = 0xA5;
							rev_len = 1;
					} else if (rev_len == 1) {
							rev_buf[1] = rev_data;
							rev_len = 2;
					} else if (rev_len == 2) {
						if (rev_data > 8) {
							rev_len = 0;
						} else {
							rev_buf[2] = rev_data;
							rev_len = 3;
						}
					} else if (rev_len >= 3 && rev_len < (3 + rev_buf[2] + 2) && rev_len < 16) {
							rev_buf[rev_len] = rev_data;
							rev_len++;
							if (rev_len == (3 + rev_buf[2] + 2)) {
								rev_fin = 1;
							}
					} else {
							rev_len = 0;
					}
		}
	}
	CMSDK_UART1->IER |= CMSDK_UART_IER_RDAI_EN_Msk;//重新启用中断
    return;
}


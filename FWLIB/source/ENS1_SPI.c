/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_SPI.C         
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

#include "ENS1_SPI.h"
#include "ENS1_GPIO.h"
/*---------------------------------------------------fifo相关的函数头--------------------------------------------------*/
/*清除FIFO和计数清0*/
uint8_t CLR_TX_FIFO( CMSDK_SPI_TypeDef* SPIx) 
{
	return (uint8_t)(SPIx->FCR>>8 & 0x1);
}

uint8_t CLR_RX_FIFO(CMSDK_SPI_TypeDef* SPIx)   
{
	return (uint8_t)(SPIx->FCR>>1 & 0x1);
}
/*FIFO 状态读取*/
uint8_t RX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx)  
{
	return (uint8_t)(((SPIx->FCR & 0x1) == 0x1) ? ((uint8_t)((SPIx->FSR & 0x001f0000)>>16)):0);        //读取当前接收FIFO数据长度
}

uint8_t TX_FIFO_LEN(CMSDK_SPI_TypeDef* SPIx)    
{
	return (uint8_t)(((SPIx->FCR & 0x1) == 0x1) ? ((uint8_t)((SPIx->FSR & 0x00001f00)>>8)):0);	       //读取当前发送FIFO数据长度
}	

SPI_BUSY_STATE BUSY_STATE(CMSDK_SPI_TypeDef* SPIx) 
{
	return (SPI_BUSY_STATE)((((SPIx->FSR & 0x10) >> 4)==1) ? ( BUSY ) : ( NOTBUSY ));           //读取当前SPI是否繁忙
}	

uint8_t  RX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx) 
{
	return 	(uint8_t)((((SPIx->FSR & 0x8)>> 3 )==1) ?  (1) : (0)) ;          //当前读取FIFO是否为满？
}	

uint8_t RX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx) 
{	
	return 	(uint8_t)((((SPIx->FSR & 0x4) >> 2)==1) ?    (1) : (0)  ) ;          //当前读取FIFO是否为空？
}	

uint8_t TX_FIFO_FULL(CMSDK_SPI_TypeDef* SPIx)  
{
	return 	(uint8_t)((((SPIx->FSR & 0x2) >> 1)==1) ?  (1) : (0)) ;         //当前发送FIFO是否为满？
}	

uint8_t TX_FIFO_EMPTY(CMSDK_SPI_TypeDef* SPIx)  
{
	return (uint8_t)(((SPIx->FSR & 0x1)==1) ? (1) : (0)) ;          //当前发送FIFO是否为空？
}




/*FIFO使能/DMA使能*/ 
uint8_t SPI_FIFO_STATE(CMSDK_SPI_TypeDef* SPIx)  //读FIFO设置
{
	return (uint8_t)(SPIx->FCR & 0x1) ;
}
uint8_t SPI_FIFO_ENABLE(CMSDK_SPI_TypeDef* SPIx )
{	
	SPIx->FCR |= 0x1;
	return   0;//(uint8_t)(SPIx->FCR & 0x1) ;
}

uint8_t SPI_FIFO_DISABLE(CMSDK_SPI_TypeDef* SPIx )
{
	SPIx->FCR &=~ 0x1;
	return (uint8_t)(SPIx->FCR & 0x1) ;
}

uint8_t SPI_FIFODMA_SET(CMSDK_SPI_TypeDef* SPIx ,bool TXDMA_SET ,bool RXDMA_SET)
{
	if(TXDMA_SET == true)
		SPIx->CTRL2 |= (1<<5);
	else if(TXDMA_SET == false)
		SPIx->CTRL2 &=~ (1<<5);
	
	if(TXDMA_SET == true)
		SPIx->CTRL2 |= (1<<4);
	else if(TXDMA_SET == false)
		SPIx->CTRL2 &=~ (1<<4);
	return (uint8_t)((SPIx->CTRL2>>4)&0x3);
}

/*------------------------------------------------------fifo相关的函数尾--------------------------------------------------*/

/*----------------------------------------------------------SPI配置-------------------------------------------------------*/
//读取当前的SPI模式
uint8_t READ_SPI_MODE(CMSDK_SPI_TypeDef* SPIx)
{
	return ((SPIx->CTRL1 & 0x7000) >> 12); 
}

//NSS通道选择
uint8_t SPI_NSS_CHANNEL(CMSDK_SPI_TypeDef* SPIx ,NSS_CHANNEL_SEL NSSx ,FunctionalState ENorDIS)
{
	if(ENorDIS == ENABLE)
		SPIx->CTRL2 |= ( 1 << NSSx );
	else
		SPIx->CTRL2 &=~ ( 1 << NSSx);
	
	return (uint8_t)((SPIx->CTRL2 & 0x0f00) >> 8);
}

//spi的参数配置
uint8_t SPI_Config_init(CMSDK_SPI_TypeDef* SPIx ,
							struct SPI_ModeConfig_Struct SPI_Config,
								struct SPI_FIFO_Struct FIFO_Struct) 
{
/*1、GPIO ALTER*/
	if(SPIx == CMSDK_SPI1)
	{
		
		
		GPIO_AltFunction(GPIO_16 , ALT_FUNC2);
		GPIO_AltFunction(GPIO_17 , ALT_FUNC2);
		GPIO_AltFunction(GPIO_18 , ALT_FUNC2);
		GPIO_AltFunction(GPIO_19 , ALT_FUNC2);
	
	}
	else if(SPIx == CMSDK_SPI0)
	{   
		
		
		GPIO_AltFunction(GPIO_8, ALT_FUNC1);
		GPIO_AltFunction(GPIO_9, ALT_FUNC1);
		GPIO_AltFunction(GPIO_10,ALT_FUNC1);
		GPIO_AltFunction(GPIO_11,ALT_FUNC1);
	}

/*
  2、写SPI_CTRL1寄存器
	（1）配置时钟波特率BAUD_RATE[2:0]
	（2）配置时钟极性和相位 CPOL and CPHA bits
	（3）选择传输模式 BIDI_EN, BIDI_MODE, UNIDI_MODE bits
	（4）配置LSB_SEL位来定义帧格式
	（5）通过配置NSS_TOGGLE、NSS_MST_CTRL、NSS_MST_SW bits来选择NSS控制方式  bit11 9 8
	（6）通过配置MST_SLV_SEL位选择主模式或从模式
*/
	 SPIx->CTRL1 = (SPIx->CTRL1&~ 0xffff) | ( SPI_Config.BAUD_FPCLKdivx << 4);
	 SPIx->CTRL1 |= (SPI_Config.SPI_MODE << 2);
	 SPIx->CTRL1 |= (SPI_Config.SPI_TRANS_MODE << 12 );
	 SPIx->CTRL1 &=~ (0x1 << 7);  //帧格式默认大端模式
	 SPIx->CTRL1 &=~ (0x1 << 8);  //帧格式默认大端模式
	 SPIx->CTRL1 &=~ (1 << 11);
	 SPIx->CTRL1 |= (1 << 11) ;  // 默认硬件生成NSS
	
	 if(SPI_Config.MS_SEL == MASTER)
	 {
		 SPIx->CTRL1 |= (1<<1);
	 }
	 else 
	 {
		SPIx->CTRL1 &=~ (1<<1);
	 }
/*
  3、写SPI_CTRL2寄存器
	（1）配置CHAR_LEN[3:0]位来选择传输的数据长度
	（2）选择“NSS端口” NSS0_EN, NSS1_EN, NSS2_EN
	（3）通过配置主机选择合适的RX数据采集阶段 SAMP_PHASE(1:0)位
	（4）通过配置C2T_DELAY和T2C_DELAY位，可以根据从设备的需求选择合适的C2T/T2C延迟
	（5）通过配置TXDMA_EN和RXDMA_EN位使能或使能FIFO模式的TX/RX DMA。
*/
	SPIx->CTRL2 =  (SPIx->CTRL2&~ 0xffff);
	if(SPI_Config.CHAR_LEN < 4)
	{
 
	}
	else if((SPI_Config.CHAR_LEN >= 4) && (SPI_Config.CHAR_LEN  < 17))
	{
		SPIx->CTRL2 |= (SPI_Config.CHAR_LEN -1);	 
	}
	else 
	{
		
	}
	SPI_NSS_CHANNEL(SPIx ,SPI_Config.NSSx ,ENABLE);
	//rx读采集相位， 仅仅在主模式下有效，默认选normal
	//C2T_DELAY ,仅在主模式下有效
	//T2C_DELAY, 仅在主模式下有效
	if(SPI_Config.MS_SEL == MASTER)
	{
		SPIx->CTRL2 |= (SPI_Config.SAMP_PHASE << 6);
		//T2C 是Transmit-end-to-chip-inactive 的延迟时间，默认为1T SCK
		//C2T 是Chip-select-active-to-transmit-start  的时间 默认1T SCK ,这里对这两参数没做设置
	}
	
	
/*
	4、写FIFO 寄存器
	（1）配置TX_FIFO_TH或RX_FIFO_TH来定义触发级别阈值
	（2）通过配置TX_FIFO_CLR和RX_FIFO_CLR位清除TX/RX FIFO
	（3）通过配置fif_en位使能或禁用FIFO模式
*/	
	SPIx->FCR |= (FIFO_Struct.TX_FIFO_TH << 9 );
	SPIx->FCR |= (FIFO_Struct.RX_FIFO_TH << 2 );
	CLR_TX_FIFO(SPIx) ;
    CLR_RX_FIFO(SPIx) ;
	if(FIFO_Struct.FIFO_ENABLE_SET == true)
		SPI_FIFO_ENABLE(SPIx);
	//SPI_FIFODMA_SET(SPIx ,FIFO_Struct.TXDMA_SET ,FIFO_Struct.RXDMA_SET);	
	return 0;
}

/*---------------------------------------------SPI启动和停止-------------------------------------------------*/
uint8_t SPI_START(CMSDK_SPI_TypeDef* SPIx )
{
	SPIx->CTRL1 |= (1);
	return (uint8_t)(SPIx->CTRL1 & 0x1);
}

uint8_t  SPI_STOP(CMSDK_SPI_TypeDef* SPIx)
{	
	if((READ_SPI_MODE(SPIx)==(L2_UniDirect_R & 0x7)) || (READ_SPI_MODE(SPIx)==(L1_BiDirect_R & 0x7)))
	{
		if(SPI_FIFO_STATE(SPIx) == 1)
		{	
			//读 RX的FIFO 直到 FIFO长度为 0
			while((RX_FIFO_LEN(SPIx) != 0)  || (BUSY_STATE(SPIx) == BUSY))
			{
				//save_data = READ_SPI_RCVBuff(SPIx);
			}
			SPIx->CTRL1 &=~ (1);
		}	
		else if(SPI_FIFO_STATE(SPIx) == 0)
		{
			while(BUSY_STATE(SPIx) == BUSY);
			SPIx->CTRL1 &=~ (1);
		}
	}
	else 
	{	
		if(SPI_FIFO_STATE(SPIx) == 1)
		{
			while((TX_FIFO_LEN(SPIx)!= 0) || (BUSY_STATE(SPIx) == BUSY));
			SPIx->CTRL1 &=~ (1);
			//读 RX的FIFO 直到 FIFO长度为 0
			while(RX_FIFO_LEN(SPIx) != 0)
			{
				uint16_t save_data = READ_SPI_RCVBuff(SPIx);
			}
		}	
		else if(SPI_FIFO_STATE(SPIx) == 0)
		{
			while(BUSY_STATE(SPIx) == BUSY);
			SPIx->CTRL1 &=~ (1);
		}
	}
	return (uint8_t)(SPIx->CTRL1*0x1);
}

/*-----------------------------------------读 / 写 SPI BUFFER的数据-------------------------------------------------*/
//读被接收的数据 最多16bits
uint16_t READ_SPI_RCVBuff(CMSDK_SPI_TypeDef* SPIx)
{	
	return (uint16_t)(SPIx->RBR & 0xffff);
}

//写数据
void WRITE_SPI_THRBuff(CMSDK_SPI_TypeDef* SPIx,uint8_t data)
{	
	SPIx->THR = data;
	while(BUSY_STATE(SPIx) == BUSY);
}

/*------------------------------------------------------SPI中断------------------------------------------------------*/
uint8_t SPI_INT_SET(IRQn_Type IRQn, bool SPI_INT_ENABLE, uint8_t SPI_INT_BIT_SET)
{
	NVIC_DisableIRQ(IRQn);
	NVIC_ClearPendingIRQ(IRQn);	
	if(IRQn == SPI0_IRQn)
	{
		CMSDK_SPI0->IER = ((CMSDK_SPI0->IER &~ (0xff)) | SPI_INT_BIT_SET);
	}
	else if(IRQn == SPI1_IRQn)
	{
		CMSDK_SPI1->IER = ((CMSDK_SPI0->IER &~ (0xff)) | SPI_INT_BIT_SET);
	}
	
	if(SPI_INT_ENABLE == true)
	{
		NVIC_EnableIRQ(IRQn); 
	}
	else 
	{
		NVIC_DisableIRQ(IRQn); 
	}	
	return 0;
}

//中断处理函数
/*
中断有如下类型：
1、发送部分有 下溢 中断（发送数据没有啦）
2、接收部分有 溢出 中断（接满啦）
3、收发完成中断？
4、发送缓冲区空 中断
5、接收缓冲区非空 中断
*/
void SPI1_Handler(void)
{
	NVIC_ClearPendingIRQ(SPI1_IRQn);
	uint8_t read_fifo=0;
	if((CMSDK_SPI1->INTSTATUS & 0x10 )== UNDERRUN_INT)  //下溢-发送时发送数据已为空时触发
	{
		CMSDK_SPI1->INTCLEAR |= (1<<4);        //清除中断
	}
	if((CMSDK_SPI1->INTSTATUS & 0x8) ==OVERRUN_INT)  
	{
		CMSDK_SPI1->INTCLEAR |= (1<<3); 
	}	
	if((CMSDK_SPI1->INTSTATUS& 0x4) ==CMPL_INT)
	{
		CMSDK_SPI1->INTCLEAR |= (1<<2); 
	}	
	if((CMSDK_SPI1->INTSTATUS &0x2) ==TXE_INT)  
	{
		
	}	
	if((CMSDK_SPI1->INTSTATUS & 1)== RXNE_INT)  
	{
		
		while(!RX_FIFO_EMPTY(CMSDK_SPI1))   
		{		
			read_fifo = READ_SPI_RCVBuff(CMSDK_SPI1);
			printf("masterrcv:%d\n",read_fifo);
		}
	}
	
	
}

void SPI0_Handler(void)
{
	NVIC_ClearPendingIRQ(SPI0_IRQn);	
	uint8_t read_fifo=0;
	if((CMSDK_SPI0->INTSTATUS & 0x10 )== UNDERRUN_INT)  //下溢-发送时发送数据已为空时触发
	{
		CMSDK_SPI0->INTCLEAR |= (1<<4);        //清除中断
	}
	if((CMSDK_SPI0->INTSTATUS & 0x8) ==OVERRUN_INT)  
	{
		CMSDK_SPI0->INTCLEAR |= (1<<3); 
	}	
	if((CMSDK_SPI0->INTSTATUS& 0x4) ==CMPL_INT)
	{
		CMSDK_SPI0->INTCLEAR |= (1<<2); 
	}	
	if((CMSDK_SPI0->INTSTATUS &0x2) ==TXE_INT)  
	{
		
	}	
	if((CMSDK_SPI0->INTSTATUS & 1)== RXNE_INT)  
	{
		
		while(!RX_FIFO_EMPTY(CMSDK_SPI0))    
		{		
			read_fifo = READ_SPI_RCVBuff(CMSDK_SPI0);
			printf("masterrcv:%d\n",read_fifo);
		}
	}
}




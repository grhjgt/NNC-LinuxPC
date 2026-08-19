/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_I2C.C         
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

#include "ENS1_IIC.h"

IIC_ConfigStructure IIC0_Config = 
{
		.MODE = IIC_SLAVE  ,
		.OWN_ADDRESS = 0x78  ,
		.ACK_EN =    true ,
		.IIC_SPEED =  IIC_STARDARD_MODE_10K  ,
};

IIC_ConfigStructure IIC1_Config = 
{
		.MODE = IIC_SLAVE  ,
		.OWN_ADDRESS = 0x78  ,
		.ACK_EN =    true ,
		.IIC_SPEED =  IIC_STARDARD_MODE_10K  ,
};


//bus error 判断（ITERREN使能）
bool IIC_BUS_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 8)&0x1) ? true :false);}
//应答错误判断
bool IIC_ACK_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 10)&0x1) ? true :false);}
//仲裁
bool IIC_ARBITRAT_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 9)&0x1) ? true :false);}
//溢出/下溢错误
bool IIC_OVERRUN_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 11)&0x1) ? true :false);}

//ITEVFEN使能后有如下事件
//起始状态，起始位发送（主机模式）
bool IIC_SB_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS)&0x1) ? true :false);}
//地址发送（主机模式） / 地址匹配（从机事件）
bool IIC_ADDR_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 1)&0x1) ? true :false);}
//主机模式下，主机已发送10bit地址数据的第一个字节！！！
bool IIC_ADD10_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 3)&0x1) ? true :false);}
//从机模式下，停止条件被接收到
bool IIC_STOPF_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 4)&0x1) ? true :false);}
//数据字节传输成功
bool IIC_BTF_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 2)&0x1) ? true :false);}

//ITEVFEN 且 ITBUFEN 使能后，有如下事件
//数据寄存器非空（接受器读到数据）
bool IIC_RxNE_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 5)&0x1) ? true :false);}
//数据寄存器空（传输完数据）
bool IIC_TxE_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 6)&0x1) ? true :false);}

//总线状态：繁忙与否
uint8_t IIC_Bus_BUSY(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (uint8_t)(((CMSDK_I2Cx->I2C_STS >> 13)&0x1) ? 1 :0);};

//模式确认
bool IIC_MASTER_MODE(CMSDK_I2C_TypeDef* CMSDK_I2Cx){return (bool)(((CMSDK_I2Cx->I2C_STS >> 12)&0x1) ? true :false);}

uint8_t IIC_Config_init(CMSDK_I2C_TypeDef* CMSDK_I2Cx  ,IIC_ConfigStructure*  IIC_Para )
{	
	if(CMSDK_I2Cx == CMSDK_I2C0)
	{
		 GPIO_AltFunction(GPIO_6 , ALT_FUNC1);
		 GPIO_AltFunction(GPIO_7 , ALT_FUNC1);
	}
	else if(CMSDK_I2Cx == CMSDK_I2C1)
	{
		GPIO_AltFunction(GPIO_8 , ALT_FUNC2);
		GPIO_AltFunction(GPIO_9 , ALT_FUNC2);
	}
	if(IIC_Para->MODE == IIC_SLAVE)
	{
	//	CMSDK_I2Cx->I2C_CR2|=(1<<7);//|(1<<8);//中断使能--bit8：buffer中断 / bit7：事件中断 / bit6：错误中断
		CMSDK_I2Cx->I2C_CR1|=(1);
		CMSDK_I2Cx->I2C_OAR = IIC_Para->OWN_ADDRESS ;
		if(IIC_Para->ACK_EN == true)
			CMSDK_I2Cx->I2C_CR1 |= (1<<5);
	}
	else 
	{
		/*
		IIC master transmitter:
		IIC_CR2:配置中断时钟频率
		IIC_CR1:使能IIC接口
		IIC_CR1:配置START 位 IIC主机
		*/
		CMSDK_I2Cx->I2C_CR2 |=  IIC_Para->IIC_SPEED ;    
		CMSDK_I2Cx->I2C_CR1 |= (1) ;   
		CMSDK_I2Cx->I2C_OAR = IIC_Para->OWN_ADDRESS ;
		if(IIC_Para->ACK_EN == true)
			CMSDK_I2Cx->I2C_CR1 |= (1<<5);	
	}	
	return 0;
}

void IIC_ITConfig(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,uint16_t IIC_IT_SEL ,FunctionalState NewState)
{
	if(CMSDK_I2Cx == CMSDK_I2C1){
		CMSDK_I2C1->I2C_CR2 |= IIC_IT_SEL;
		NVIC_DisableIRQ(I2C1_Event_IRQn);
		NVIC_ClearPendingIRQ(I2C1_Event_IRQn);
		NVIC_DisableIRQ(I2C1_Error_IRQn);
		NVIC_ClearPendingIRQ(I2C1_Error_IRQn);
		if(NewState == ENABLE)
			NVIC_EnableIRQ(I2C1_Event_IRQn);
	}
	else if(CMSDK_I2Cx == CMSDK_I2C0){
		CMSDK_I2C0->I2C_CR2 |= IIC_IT_SEL;
		NVIC_DisableIRQ(I2C0_Event_IRQn);
		NVIC_ClearPendingIRQ(I2C0_Event_IRQn);
		NVIC_DisableIRQ(I2C0_Error_IRQn);
		NVIC_ClearPendingIRQ(I2C0_Error_IRQn);
		if(NewState == ENABLE)
			NVIC_EnableIRQ(I2C0_Event_IRQn);
	}
}

void IIC_Cmd(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,FunctionalState NewState)   //使能IIC外设
{	
	if(CMSDK_I2Cx ==CMSDK_I2C0)
	{
		PCLK_Enable(I2C0_PCLK_EN);
	}
	else if(CMSDK_I2Cx ==CMSDK_I2C1)
	{
		PCLK_Enable(I2C1_PCLK_EN);
	}
	else
	{
	
	}
	
	if(NewState == ENABLE)
		CMSDK_I2Cx->I2C_CR1 |= (1);
	else 
		CMSDK_I2Cx->I2C_CR1 &=~ (1);
}

void IIC_GenerateSTART(CMSDK_I2C_TypeDef* CMSDK_I2Cx)
{	
	while(IIC_Bus_BUSY(CMSDK_I2Cx));
	CMSDK_I2Cx->I2C_CR1 |= (1<<3);
}

void IIC_GenerateSTOP(CMSDK_I2C_TypeDef* CMSDK_I2Cx)
{
	CMSDK_I2Cx->I2C_CR1 |= (1<<4);
}

//general call 用于向总线所有设备发送消息，用于初始化从设备与设备同步
//主设备通过向总线发送0x00地址来触发GENERAL CALL
void IIC_GeneralCallCmd(CMSDK_I2C_TypeDef* CMSDK_I2Cx)  
{
	
}

void IIC_Send7bitAddress(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,uint8_t Address)
{
	CMSDK_I2Cx->I2C_DR = Address ;
}

void IIC_SendData(CMSDK_I2C_TypeDef* CMSDK_I2Cx , uint8_t data)
{	
	CMSDK_I2Cx->I2C_DR = data;
	while(IIC_BTF_EVENT(CMSDK_I2Cx) == false); //字节传输完成
}

uint8_t  IIC_ReadData(CMSDK_I2C_TypeDef* CMSDK_I2Cx)
{	
	return CMSDK_I2Cx->I2C_DR;
}



void I2C0_Event_Handler(void)
{	
	NVIC_ClearPendingIRQ(I2C0_Event_IRQn);
	if(IIC_MASTER_MODE(CMSDK_I2C0) == TRUE)
	{
		if(IIC_SB_EVENT(CMSDK_I2C0) == true )
		{
			return ;
		}
		
		if(IIC_ADDR_EVENT(CMSDK_I2C0) == true )
		{
			return ;
		}
		
		if(IIC_ADD10_EVENT(CMSDK_I2C0) == true)
		{
			return;
		}		
	}
	
	else
	{
		if(IIC_ADDR_EVENT(CMSDK_I2C0) == true)
		{
			return ;
		}
		if(IIC_STOPF_EVENT(CMSDK_I2C0) == true)
		{
			//如果不再接收，则关闭IIC!!!
			IIC_Cmd(CMSDK_I2C0 ,DISABLE) ;
			CMSDK_I2C0->I2C_CR2|=(1<<7);  //中断使能--bit8：buffer中断 / bit7：事件中断 / bit6：错误中断
			CMSDK_I2C0->I2C_CR1|=(1);
			CMSDK_I2C0->I2C_OAR = 0X78 ;
			CMSDK_I2C0->I2C_CR1 |= (1<<5);
			IIC_Cmd(CMSDK_I2C0 ,ENABLE) ;
			return ;
		}
	}
	
	if(IIC_BTF_EVENT(CMSDK_I2C0) == true)
	{
		
	}
	
	if(IIC_RxNE_EVENT(CMSDK_I2C0) == true)
	{
		uint8_t read_data = (uint8_t)(CMSDK_I2C0->I2C_DR & 0xff);
		printf("%d\n", read_data);
	}
	
	if(IIC_TxE_EVENT(CMSDK_I2C0) == true)
	{
		
	}
	return ;
}

void I2C0_Error_Handler(void)
{
	NVIC_ClearPendingIRQ(I2C0_Error_IRQn);
	
	if(IIC_BUS_ERROR(CMSDK_I2C0) == true)
	{
		printf("bus error\n");
		return ;
	}
	
	if(IIC_ACK_ERROR(CMSDK_I2C0) == true)
	{
		printf("IIC_ACK_ERROR\n");
		return ;
	}
	
	if(IIC_ARBITRAT_ERROR(CMSDK_I2C0) == true)
	{
		printf("IIC_ARBITRAT_ERROR\n");
		return ;
	}
	
	if(IIC_OVERRUN_ERROR(CMSDK_I2C0) == true)
	{
		printf("IIC_OVERRUN_ERROR\n");
		return ;
	}
	
}


void I2C1_Event_Handler(void)
{	
	NVIC_ClearPendingIRQ(I2C1_Event_IRQn);
	if(IIC_MASTER_MODE(CMSDK_I2C1) == TRUE)
	{
		if(IIC_SB_EVENT(CMSDK_I2C1) == true )
		{
			return ;
		}
		
		if(IIC_ADDR_EVENT(CMSDK_I2C1) == true ) 
		{
			return;
		}
		
		if(IIC_ADD10_EVENT(CMSDK_I2C1) == true)
		{
			return;
		}		
	}
	else
	{
		if(IIC_ADDR_EVENT(CMSDK_I2C1) == true) //作为从机，进入此处则地址匹配！
		{
			return ;
		}
		
		if(IIC_STOPF_EVENT(CMSDK_I2C1) == true) //作为从机，接收到STOP信号
		{
			printf("get stop\n");
			//如果不再接收，则关闭IIC!!!
			IIC_Cmd(CMSDK_I2C1 ,DISABLE) ;
			CMSDK_I2C1->I2C_CR2|=(1<<7);  //中断使能--bit8：buffer中断 / bit7：事件中断 / bit6：错误中断
			CMSDK_I2C1->I2C_CR1|=(1);
			CMSDK_I2C1->I2C_OAR = 0X78 ;
			CMSDK_I2C1->I2C_CR1 |= (1<<5);
			IIC_Cmd(CMSDK_I2C1 ,ENABLE) ;
			IIC_ITConfig(CMSDK_I2C0,0 ,DISABLE);
			IIC_ITConfig(CMSDK_I2C0,0 ,ENABLE);
			return ;
		}
	}
	if(IIC_BTF_EVENT(CMSDK_I2C1) == true)
	{
	 
	}
	if((IIC_RxNE_EVENT(CMSDK_I2C1) == true)) //作为从机，buffer非空，读IIC_DR中的数据
	{
		uint8_t read_data = (uint8_t)(CMSDK_I2C1->I2C_DR & 0xff);
		printf("%d\n", read_data);
	}
	if(IIC_TxE_EVENT(CMSDK_I2C1) == true)
	{

	}
	return ;	
}

void I2C1_Error_Handler(void)
{
	NVIC_ClearPendingIRQ(I2C1_Error_IRQn);
	
	if(IIC_BUS_ERROR(CMSDK_I2C1) == true)
	{
		printf("bus error\n");
		return ;
	}
	
	if(IIC_ACK_ERROR(CMSDK_I2C1) == true)
	{
		printf("IIC_ACK_ERROR\n");
		return ;
	}
	
	if(IIC_ARBITRAT_ERROR(CMSDK_I2C1) == true)
	{
		printf("IIC_ARBITRAT_ERROR\n");
		return ;
	}
	
	if(IIC_OVERRUN_ERROR(CMSDK_I2C1) == true)
	{
		printf("IIC_OVERRUN_ERROR\n");
		return ;
	}
	
}


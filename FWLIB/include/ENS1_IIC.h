/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_I2C.H         
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

#ifndef ENS1_IIC_H
#define ENS1_IIC_H
#include "my_header.h"
#include "ens1_uart.h"
#include "ENS1_GPIO.h"
/*
一、引脚关系：
ALT Function1 
IIC0 SCL    ---    GPIO6
IIC0 SDA    ---    GPIO7

ALT Function2 
IIC1 SCL    ---    GPIO8
IIC1 SDA    ---    GPIO9

ENS1 IIC ：
1、标准模式100khz  快速模式 400khz
2、DMA
3、主机模式下通过软件发出启停信号
4、从机模式下，接口可以识别自己的地址和一般呼叫地址，一般呼叫地址监测可以通过软件使能或失能
5、数据和地址的传输以8位，大端模式，开始后的字节包含地址信息，（7bit地址模式是一字节，10bit地址模式是2字节）
   地址总时在主机模式下传输。
6、在一个字节传输的8个时钟周期后的第九个时钟，接收端必须发送一个应答位给传输端
   应答可以被软件使能或者失能，可以通过软件选择地址是7位还是10位的。

特性说明：
1、可做主、从端
2、半双工通讯（仅发送或接收）
3、7位 / 10位的寻址或检测
4、支持不同传输速率： 100k  400k
5、模拟噪声滤波器
6、状态标志：
  （1）传输/接收模式标志
  （2）字节结束传输标志
  （3）IIC 繁忙标志
7、数据总是大端模式
8、错误标志：
（1）主模式下:仲裁丢失状态
（2）地址 / 数据传输后应答失败
（3）错位启动或停止 状态的检测
9、1字节缓冲带有DMA功能
*/

/*
错误状态（会造成通讯失败）
1 总线错误 BUS ERROR
2 应答错误 AD
3 仲裁
4 溢出/下溢 错误
*/

/*IIC 速度选择*/
#define IIC_STARDARD_MODE_10K    0x0
#define IIC_STARDARD_MODE_20K    0x1
#define IIC_STARDARD_MODE_100K   0x9

#define IIC_FAST_MODE_110K    0xA
#define IIC_FAST_MODE_120K    0xB
#define IIC_FAST_MODE_400K    0x27

#define IIC_HIGH_MODE_500K    0x28
#define IIC_HIGH_MODE_600K    0x29
#define IIC_HIGH_MODE_1500K    0x2E

/*中断使能*/
#define IIC_ITERREN  (1<<6)    //error 使能
#define IIC_ITEVTEN  (1<<7)    //事件中断使能
#define IIC_ITBUFEN  (1<<8)    //缓冲区中断使能
#define IIC_DMAEN    (1<<9)    //DMA使能




typedef enum{IIC_MASTER=0, IIC_SLAVE=1}IIC_MODE ;

typedef struct IIC_Config_Struct
{
	IIC_MODE MODE;
	uint32_t OWN_ADDRESS;
	bool ACK_EN; 
	uint8_t IIC_SPEED;
}IIC_ConfigStructure;

extern IIC_ConfigStructure IIC0_Config;
extern IIC_ConfigStructure IIC1_Config;

//bus error 判断（ITERREN使能）
extern bool IIC_BUS_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//应答错误判断
extern bool IIC_ACK_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//仲裁
extern bool IIC_ARBITRAT_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//溢出/下溢错误
extern bool IIC_OVERRUN_ERROR(CMSDK_I2C_TypeDef* CMSDK_I2Cx);

//ITEVFEN使能后有如下事件
//起始状态，起始位发送（主机模式）
extern bool IIC_SB_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//地址发送（主机模式） / 地址匹配（从机事件）
extern bool IIC_ADDR_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//主机模式下，主机已发送10bit地址数据的第一个字节！！！
extern bool IIC_ADD10_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//从机模式下，停止条件被接收到
extern bool IIC_STOPF_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//数据字节传输成功
extern bool IIC_BTF_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);

//ITEVFEN 且 ITBUFEN 使能后，有如下事件
//数据寄存器非空（接受器读到数据）
extern bool IIC_RxNE_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//数据寄存器空（传输完数据）
extern bool IIC_TxE_EVENT(CMSDK_I2C_TypeDef* CMSDK_I2Cx);

//总线状态：繁忙与否
extern uint8_t IIC_Bus_BUSY(CMSDK_I2C_TypeDef* CMSDK_I2Cx);

//模式确认
extern bool IIC_MASTER_MODE(CMSDK_I2C_TypeDef* CMSDK_I2Cx);



//IIC配置初始化
extern uint8_t IIC_Config_init(CMSDK_I2C_TypeDef* CMSDK_I2Cx  ,IIC_ConfigStructure*  IIC_Para );
//使能与失能IIC
extern void IIC_Cmd(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,FunctionalState NewState) ;
//应答选择
extern void IIC_Ackconfig(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,FunctionalState NewState );
//生成START 信号
extern void IIC_GenerateSTART(CMSDK_I2C_TypeDef* CMSDK_I2Cx);
//生成STOP信号
extern void IIC_GenerateSTOP(CMSDK_I2C_TypeDef* CMSDK_I2Cx);

//IIC发送数据
extern void IIC_SendData(CMSDK_I2C_TypeDef* CMSDK_I2Cx , uint8_t data);

extern void IIC_ITConfig(CMSDK_I2C_TypeDef* CMSDK_I2Cx ,uint16_t IIC_IT_SEL ,FunctionalState NewState);
#endif 

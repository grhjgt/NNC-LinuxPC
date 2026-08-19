/*
*Copyright ,2023 , NANOCHAP
*File name:   ENS1_WATCHDOG.h         
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

#ifndef ENS1_WATCHDOG_H
#define ENS1_WATCHDOG_H
#include "CMSDK_CM0.h"
#include "ENS1_CLOCK.h"
#include "my_header.h"
/*
1、watchdog寄存器有写锁，在写入watchdog的寄存器前需要对 WDOGLOCK 寄存器写入 0x1ACCE551
2、watchdog的技术加载值： WDOGLOAD = interval(us) * pclk(MHZ)
3、计数重加载位置： WDOGVALUE
4、写1到 WDOGCONTROL 的 INTEN 位 ，使能看门狗和预警中断，当WDT计数到0，则触发一次中断
   中断状态被知识通过设置 WDOGMIS位，如果 WDOGINTCLR 寄存器写入任意值（比如按照惯例，写1）
   此时WDOGMIS 位被清0 ，如果RESEN位是 0 ，watchdog定时器将重载。
5、以上步骤设置完成后，写一个任意数据（除了0x1ACCE551）到WDOGLOCK寄存器
6、如果WDT当前计数值没有重载， 计数到达0时 WDT 将触发一个早期预警中断
   因此，在中断触发前需要重载 WDOGLOAD寄存器
*/
typedef enum{UNLOCK = 0,LOCK=1}WdogLockState;
//看门狗喂狗
void WatchDogRefresh(void);
extern void WatchDogLoad(uint32_t loadvalue_us);
extern uint32_t Read_WDOGVALUE(void);
extern void WatchDog_Control(FunctionalState newstate);
extern void WatchDog_IntClear(void);
extern uint8_t WatchDog_LOCK_RegSet(WdogLockState newstate);
extern void WatchDog_Init(uint32_t loadvalue_us);
#endif




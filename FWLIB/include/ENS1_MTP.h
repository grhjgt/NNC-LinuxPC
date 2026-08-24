/*
* Copyright (C), 2023, NANOCHAP
* File name:   ENS1_MTP.H
* Description: MTP Flash 驱动头文件 (仅保留读模式切换与初始化接口)
*/

#ifndef ENS1_MTP_H
#define ENS1_MTP_H

#include "CMSDK_CM0.h"
#include "ENS001_CONFIG.h"
#include <stdlib.h>
#include <string.h>
#include "my_header.h"

extern uint8_t MTP_init(void);
extern void MTP_enter_read_mode(void);

#endif

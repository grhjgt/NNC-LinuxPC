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

/* SCH-260515: U1 光耦集电极 -> GPIO19。只驱动 WAVE_GEN_DRVA_BLK0。
   硬件须同时满足，否则 0.1mA/100Hz 开路和贴合的低电平宽度分不开：
   R166（集电极上拉）= 220kΩ，R172（LED 并联）= 100kΩ，拆掉 C3（22nF）。
   C3 串在 LED 旁路上，脱落时的电压跳变会经它点亮 LED。 */
#define LOD_GPIO_CH0          GPIO_19
#define LOD_GPIO_CH2          GPIO_22
#define LOD_WIN_MIN_MS        200u
/* 连续低电平达到 lod_low_min_ticks() 算贴合。 */
#define LOD_SAMPLE_HZ         10000u
#define LOD_TICKS_PER_MS      (LOD_SAMPLE_HZ / 1000u)
/* R172=100k 后 0.1mA/10Hz 闭路每周期低电平约 24ms（占 24%），电流越大越宽。
   门限取周期的 12%，10Hz 即 12ms；高频不低于 2ms。0.1mA/100Hz 开路约 1.6ms，贴合约 2.6ms。 */
#define LOD_LOW_MIN_PCT       12u
#define LOD_LOW_FLOOR_TICKS   20u
/* 慢沿过门限时会抖出 0.1~0.2ms 的高电平，不打断这段低电平 */
#define LOD_HIGH_GAP_TICKS    2u
/* 连续两个窗都不达标才切输出，单窗干扰不误停 */
#define LOD_OPEN_WINDOWS      2u
/* 冷机升压起来前 GPIO 没有低脉冲，这段时间不准切输出 */
#define LOD_BOOST_ARM_MS      2000u
/* 与主控 NNC_LOD_ENABLE=0 对齐：0=开路不主动切断本片输出，便于跨路/空载台架测试；1=恢复开路切输出 */
#define LOD_CUT_ENABLE        0

volatile uint32_t g_tick_ms = 0;
volatile uint32_t g_tick_100us = 0;
volatile uint32_t g_rx_fin_us = 0;
static volatile uint8_t  g_stim_running = 0;
/* 0=刺激中贴合, 1=脱落或电平卡死, 0xFF=未在刺激 */
static uint8_t  g_lead_off_ch0 = 0xFF;
static uint8_t  g_lead_off_ch2 = 0;
static volatile uint32_t s_sample_cnt = 0;
static volatile uint32_t s_low_run = 0;
static volatile uint32_t s_max_low_run = 0;
static volatile uint8_t  s_high_run = 0;
static uint8_t s_open_cnt = 0;
static uint8_t s_sub_ms = 0;
static uint32_t s_duty_tick = 0;
static uint32_t s_lod_arm_tick = 0;
static uint8_t s_open_latched = 0;
static uint8_t s_contact_ok = 0;
/* 1–4Hz 合成正弦：半周寄存器拉不到 1Hz，改 CLK 会截断。
   每周期 128 点，按 sin 写电流，正半周只开正、负半周只开负。
   1Hz 约 8ms 一点，4Hz 约 2ms 一点。5–100Hz 仍是硬件连续正弦。 */
#define SLOW_HZ_MAX     4u
#define SLOW_STEPS      128u
#define SLOW_HOLD_US    8000u
#define SLOW_MIN_US     2u
static uint8_t s_slow_on = 0;
static uint8_t s_slow_hz = 1;
static uint8_t s_slow_idx = 0xFF;
static uint8_t s_slow_neg = 0xFF;
static uint32_t s_slow_t0_us = 0;

/* --------------------------------------------------------------- */
/*  串口 1s 晶振同步与锁相环 (UART-DPLL: CMD=0x05)                   */
/*  用主从控 STM32 的 8MHz 石英晶振 1s 信标校准本片 HSI RC 频偏，    */
/*  并在四板统一对齐时刻将正弦相位锁定到 0° 过零点，消除跨板拍频。   */
/* --------------------------------------------------------------- */
static uint32_t s_hsi_us_per_sec = 1000000u;
static uint8_t  s_hsi_calibrated = 0u;
static uint8_t  s_sync_has_prev = 0u;
static uint8_t  s_sync_last_seq = 0u;
static uint32_t s_sync_last_target_us = 0u;
static uint8_t  s_sync_pending = 0u;
static uint8_t  s_sync_pending_seq = 0u;
static uint32_t s_sync_target_us = 0u;
static uint8_t  s_sync_outlier_cnt = 0u;
/* STM32 刺激输出期间每 1s 下发 0x05 同步信标; 若 RX 断开导致收不到 STOP 也收不到信标,
   超时 3.5s 后 ENS001 自主切断输出, 不再依赖单向串口命令。 */
#define STIM_SYNC_WATCHDOG_MS 3500u
#define STIM_SYNC_STOP_REP_MS 1000u
#define STIM_SYNC_STOP_REP_MAX 15u
static uint32_t s_sync_wdog_tick = 0u;
static uint8_t  s_sync_stopped = 0u;
static uint32_t s_sync_stop_rep_tick = 0u;
static uint8_t  s_sync_stop_rep_cnt = 0u;
static uint8_t  s_stim_start_seq = 0u;

/* 返回片内 HSI/32 基准下的微秒计数（与 WAVE_GEN_DRV_CLK_FREQ_REG=0x20 同源同频） */
uint32_t get_hsi_us(void)
{
    uint32_t t1, t2, val;
    uint32_t load = SysTick->LOAD;
    do {
        t1 = g_tick_100us;
        val = SysTick->VAL;
        t2 = g_tick_100us;
    } while (t1 != t2);
    return t1 * 100u + ((load - val) >> 5);
}

static void sync_reset_phase_state(void)
{
    s_sync_has_prev = 0u;
    s_sync_pending = 0u;
    s_sync_outlier_cnt = 0u;
}

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

/* 1–4Hz 专用半正弦，从 0 起落到 0，避免起始台阶打成尖峰。电流仍按 g_current 缩放。 */
static const int sine_slow_data[64] = {
    0, 13, 25, 38, 51, 63, 75, 87, 99, 111, 122, 133, 144, 154, 164, 173,
    183, 191, 199, 207, 214, 221, 227, 232, 237, 242, 245, 249, 251, 253, 254, 255,
    255, 254, 253, 251, 249, 245, 242, 237, 232, 227, 221, 214, 207, 199, 191, 183,
    173, 164, 154, 144, 133, 122, 111, 99, 87, 75, 63, 51, 38, 25, 13, 0
};

/* --------------------------------------------------------------- */
/*  WAVE GEN Driver_A SINE test                                    */
/* --------------------------------------------------------------- */

/* 与 d075b307 相同：只写 0x77213 打开 EN，不要先写 0x77212。
   冷机先关 EN 再开，升压会整场起不来。 */
static void stim_power_on(void)
{
    CMSDK_SYSCON->APB_CLKEN |= 0x4000u | 0x1000u;
    CMSDK_ANAC->BOOST_CTRL = 0x77213;
    CMSDK_ANAC->PMU_CTRL = 0x10;
}

/* 电流波形幅值缩放系数：
   原 0.6587f 按“满档 20 = 2.0 mA 有效值 (RMS)”标定；
   对外改以正弦峰峰值 (I_p-p) 定义后，I_RMS = I_p-p / (2 * sqrt(2))，
   即满档 20 对应 2.0 mA(p-p)（峰值 ±1.0 mA，有效值 0.707 mA）。 */
#define STIM_WAVE_SCALE_PP  (0.6587f / 2.8284271f)

static void wavegen_slow_apply(uint8_t idx)
{
    CMSDK_WAVE_GEN_TypeDef *drv = WAVE_GEN_DRVA_BLK0;
    uint8_t neg = (idx >= 64u) ? 1u : 0u;
    uint8_t half = neg ? (uint8_t)(idx - 64u) : idx;
    int code = (int)((float)sine_slow_data[half] * STIM_WAVE_SCALE_PP * (g_current / 20) + 0.5f);
    int i;

    if (code < 0) {
        code = 0;
    } else if (code > 255) {
        code = 255;
    }
    for (i = 0; i < 64; i++) {
        drv->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i;
        drv->WAVE_GEN_DRV_IN_WAVE_REG = code;
    }
    if (neg != s_slow_neg) {
        if (neg) {
            drv->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = SLOW_MIN_US;
            drv->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = SLOW_HOLD_US;
            drv->WAVE_GEN_DRV_CONFIG_REG = 0x0000004A;
        } else {
            drv->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = SLOW_HOLD_US;
            drv->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = SLOW_MIN_US;
            drv->WAVE_GEN_DRV_CONFIG_REG = 0x00000048;
        }
        s_slow_neg = neg;
    }
    drv->WAVE_GEN_DRV_CTRL_REG = 1;
    s_slow_idx = idx;
}

static void wavegen_slow_proc(void)
{
    uint32_t elapsed_us;
    uint32_t period_us;
    uint32_t pos_us;
    uint8_t idx;

    if (!s_slow_on) {
        return;
    }
    elapsed_us = get_hsi_us() - s_slow_t0_us;
    period_us = s_hsi_us_per_sec / s_slow_hz;
    if (period_us < 1000u) {
        period_us = 1000u;
    }
    pos_us = elapsed_us % period_us;
    idx = (uint8_t)((pos_us / 100u) * SLOW_STEPS / (period_us / 100u));
    if (idx >= SLOW_STEPS) {
        idx = (uint8_t)(SLOW_STEPS - 1u);
    }
    if (idx != s_slow_idx) {
        wavegen_slow_apply(idx);
    }
}

static void wavegen_sync_proc(void)
{
    uint32_t now_us;
    uint8_t hz;
    uint32_t half_us;
    uint8_t align_ok = 1u;

    if (!s_sync_pending) {
        return;
    }
    now_us = get_hsi_us();
    if ((int32_t)(now_us - s_sync_target_us) < 0) {
        return;
    }
    s_sync_pending = 0u;

    /* 1) 若与上一拍连续（允许跨过 1~2 拍单次抖动），用主控 8MHz 石英晶振校准本片 HSI 真实微秒数；
          已校准后对单拍串口抖动 (>1.5ms) 及主循环延迟 (>1ms) 进行门限过滤，防止污染频率或在非过零点硬复位波形。 */
    if (s_sync_has_prev) {
        uint8_t dseq = (uint8_t)(s_sync_pending_seq - s_sync_last_seq);
        if (dseq >= 1u && dseq <= 3u) {
            uint32_t avg_dt_us = (s_sync_target_us - s_sync_last_target_us + (dseq >> 1)) / dseq;
            if (avg_dt_us >= 950000u && avg_dt_us <= 1050000u) {
                if (!s_hsi_calibrated) {
                    if (dseq == 1u) {
                        s_hsi_us_per_sec = avg_dt_us;
                        s_hsi_calibrated = 1u;
                        s_sync_outlier_cnt = 0u;
                    }
                } else {
                    uint32_t err_us = (avg_dt_us > s_hsi_us_per_sec)
                                    ? (avg_dt_us - s_hsi_us_per_sec)
                                    : (s_hsi_us_per_sec - avg_dt_us);
                    if (err_us <= 1500u) {
                        s_hsi_us_per_sec = (s_hsi_us_per_sec * 3u + avg_dt_us + 2u) >> 2;
                        s_sync_outlier_cnt = 0u;
                    } else if (++s_sync_outlier_cnt >= 3u) {
                        /* 连续 3 拍超限视为真实温漂/重基准，重新校准并锁定 */
                        s_hsi_us_per_sec = avg_dt_us;
                        s_sync_outlier_cnt = 0u;
                    } else {
                        /* 单拍抖动超限：本拍不更新频率、不覆盖上一正常拍基准、不硬切 CTRL_REG */
                        align_ok = 0u;
                    }
                }
            } else {
                align_ok = 0u;
            }
        }
    }
    if (align_ok) {
        s_sync_last_target_us = s_sync_target_us;
        s_sync_last_seq = s_sync_pending_seq;
        s_sync_has_prev = 1u;
    } else if ((uint8_t)(s_sync_pending_seq - s_sync_last_seq) >= 3u) {
        /* 连续抖动且累计跨度已达 3 拍仍未恢复：放弃旧基准，从本拍重建 */
        s_sync_has_prev = 0u;
        s_sync_outlier_cnt = 0u;
    }

    if (!g_stim_running || !align_ok) {
        return;
    }
    if ((uint32_t)(now_us - s_sync_target_us) > 1000u) {
        /* 主循环阻塞超过 1ms 才轮询到本拍：跳过本拍硬复位，由已校准频率维持相位 */
        return;
    }

    hz = (g_freq < 1.0f) ? 1u : (g_freq > 100.0f) ? 100u : (uint8_t)g_freq;
    if (s_slow_on) {
        if ((hz != 3u) || ((s_sync_pending_seq % 3u) == 0u)) {
            s_slow_t0_us = s_sync_target_us;
            s_slow_idx = 0xFF;
            wavegen_slow_proc();
        }
        return;
    }

    /* 2) 5–100Hz 硬件正弦：每秒恰好含整数个周期 (hz 个周期)。
          写入晶振校准后的半周期并在四板统一时刻对齐 0° 过零点相位。 */
    half_us = (s_hsi_us_per_sec + hz) / (2u * hz);
    WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_CTRL_REG = 0;
    WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = half_us;
    WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = half_us;
    /* 确保 CTRL_REG=0 低电平至少保持 3us (>1MHz 波形时钟 3 个周期)，避免复位脉冲被漏采 */
    {
        uint32_t t0 = get_hsi_us();
        while ((uint32_t)(get_hsi_us() - t0) < 3u) {
        }
    }
    WAVE_GEN_DRVA_BLK0->WAVE_GEN_DRV_CTRL_REG = 1;
}

int wavegen_driverA_sine_test(CMSDK_WAVE_GEN_TypeDef *CMSDK_WAVEGEN_DRVA, int units){
    int return_val=0;
    int err_code=0; 
    int Current_units=0;
    //puts("\nDRIVER A SINE TEST\n");

    stim_power_on();
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_REST_T_REG = 0x00000000;   //???????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CLK_FREQ_REG = 0x00000020;
    /* 5–100Hz：半周按校准后的 s_hsi_us_per_sec/(2*Hz) 计算。1–4Hz：短半周当电流保持，点数由 wavegen_slow_proc 写。 */
    {
        uint8_t hz = (g_freq < 1.0f) ? 1u : (g_freq > 100.0f) ? 100u : (uint8_t)g_freq;
        uint32_t half_us = (s_hsi_us_per_sec + hz) / (2u * hz);
        uint32_t neg_us = half_us;
        uint32_t cfg = 0x0000004A;
        s_slow_on = 0;
        if (hz <= SLOW_HZ_MAX) {
            s_slow_hz = hz;
            s_slow_on = 1;
            half_us = SLOW_HOLD_US;
            neg_us = SLOW_MIN_US;
            cfg = 0x00000048;
        }
        CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_SILENT_T_REG = 0;
        CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_HLF_WAVE_PRD_REG = half_us;
        CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG = neg_us;
        CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CONFIG_REG = cfg;
    }
    /* 0x4A 连续正弦。1–4Hz 正半周 0x48、负半周 0x4A。 */ 
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

    if (!s_slow_on) {
        for(int i=0; i<64; i++){
            CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_ADDR_REG = i;
            CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_IN_WAVE_REG = (int)( (float)sine_data[i]*STIM_WAVE_SCALE_PP *(g_current/20) + 0.5f );
        }
    }
   
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_LIM_REG = 0x00000000;//  ????? 
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_ALT_SILENT_LIM_REG = 0x00000000; // ?????????????????????????????????b????????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_DELAY_LIM_REG = 0x00000000;  // ????????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_SCALE_REG = 0x00000001;  //??????????????????^??255?????0???????
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_NEG_OFFSET_REG = 0x00000000; //?????????????????^??255?????0?????????
                                                                  //?????????????250??????????10????????????5 ??????
		
    CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_INT_REG = 0x0;//  interrupt register set to 0
    sync_reset_phase_state();
    if (s_slow_on) {
        s_slow_t0_us = get_hsi_us();
        s_slow_idx = 0xFF;
        s_slow_neg = 0xFF;
        wavegen_slow_apply(0);
    } else {
        CMSDK_WAVEGEN_DRVA->WAVE_GEN_DRV_CTRL_REG = 0x00000001; //??? driver
    }

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
	s_slow_on = 0;
	s_slow_idx = 0xFF;
	s_slow_neg = 0xFF;
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
   闭路时脚被拉在高门限以下；开路毛刺不到 2ms。
   g_tick_ms 仍是毫秒，给统计窗用。 */
void SysTick_Handler(void)
{
    uint8_t level;

    g_tick_100us++;
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
        s_high_run = 0;
        s_low_run++;
        if (s_low_run > s_max_low_run) {
            s_max_low_run = s_low_run;
        }
    } else if (s_high_run < LOD_HIGH_GAP_TICKS) {
        s_high_run++;
    } else {
        s_low_run = 0;
    }
}

/* GPIO16–23 共用这个入口。LOD_Init 已关掉 GPIO19 EXTI，正常不会进来。
   若上电残留把沿挂起了，两沿都清掉，避免反复进中断把 UART 饿死。 */
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

/* 1–4Hz 的正弦半周是 50ms，低电平宽度按 10Hz 算。 */
static uint8_t lod_shape_hz(uint8_t freq_hz)
{
    if (freq_hz < 5u) {
        return 10u;
    }
    if (freq_hz > 100u) {
        return 100u;
    }
    return freq_hz;
}

/* 判贴合所需的最长连续低电平（采样点数）：周期的 LOD_LOW_MIN_PCT，不低于 LOD_LOW_FLOOR_TICKS */
static uint32_t lod_low_min_ticks(uint8_t freq_hz)
{
    uint32_t t;
    freq_hz = lod_shape_hz(freq_hz);
    t = LOD_SAMPLE_HZ / freq_hz * LOD_LOW_MIN_PCT / 100u;
    if (t < LOD_LOW_FLOOR_TICKS) {
        t = LOD_LOW_FLOOR_TICKS;
    }
    return t;
}

static void lod_uplink_seq(uint8_t state, uint8_t seq)
{
    send_buf[0] = 0xA5;
    send_buf[1] = 0x0A;
    send_buf[2] = 0x02;
    send_buf[3] = state;
    send_buf[4] = (state == 0x02u) ? s_stim_start_seq : seq;
    send_buf[5] = crc8_calc(send_buf, 5);
    send_buf[6] = 0x5A;
    Uart_Send(CMSDK_UART1, send_buf, 7);
}

/* 主动上报给主控，不再等小程序来查。0xFF=开机待机，1=刺激中因脱落进入待机。
   主控收到后若蓝牙已连接，把这一帧转给小程序。 */
static void lod_uplink(uint8_t state)
{
    lod_uplink_seq(state, 0u);
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
    s_open_cnt = 0;
    lod_uplink(1);
}

/* 刺激开始：先报贴合，计数清零，并空出整整一个统计窗再允许判定。
   刚启动时波形还没起来，立刻统计会把空窗当成开路。 */
static void lod_mark_stim_start(void)
{
    uint32_t now = g_tick_ms;

    g_stim_running = 1;
    s_sync_wdog_tick = now;
    s_sync_stopped = 0u;
    s_sync_stop_rep_tick = 0u;
    s_sync_stop_rep_cnt = 0u;
    g_lead_off_ch0 = 0;
    g_lead_off_ch2 = 0;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    s_open_cnt = 0;
    s_open_latched = 0;
    s_contact_ok = 0;
    s_duty_tick = now;
    /* 升压未稳时没有光耦低脉冲。空一个统计窗再加 LOD_BOOST_ARM_MS，这段不切输出。 */
    s_lod_arm_tick = now + lod_window_ms((uint8_t)g_freq) + LOD_BOOST_ARM_MS;
}

/* 上位机停刺激。没有光耦电流，不能再报贴合或脱落，状态回到 0xFF，并放开脱落锁存。 */
static void lod_mark_stim_stop(void)
{
    g_stim_running = 0;
    s_sync_stopped = 0u;
    s_sync_stop_rep_tick = 0u;
    s_sync_stop_rep_cnt = 0u;
    g_lead_off_ch0 = 0xFF;
    g_lead_off_ch2 = 0;
    s_sample_cnt = 0;
    s_low_run = 0;
    s_max_low_run = 0;
    s_open_cnt = 0;
    s_open_latched = 0;
    s_contact_ok = 0;
    sync_reset_phase_state();
}

/* GPIO19/GPIO22 配成无上下拉输入。GPIO22 本版不参与判定。 */
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
    /* 脉宽只在 SysTick 里采。不要开 GPIO19 双边沿 EXTI：C3 拆掉后空脚会抖，
       和 UART1 同优先级把起始帧打掉。4be0880 没开这条 EXTI，关机一分钟后 Key4 仍有电流。 */
    NVIC_DisableIRQ(GPIO16_23_IRQn);
    NVIC_ClearPendingIRQ(GPIO16_23_IRQn);
    CMSDK_EXTI->IMR &= ~(1u << EXTI_GPIO19);
    CMSDK_EXTI->FTSR &= ~(1u << EXTI_GPIO19);
    CMSDK_EXTI->RTSR &= ~(1u << EXTI_GPIO19);
    CMSDK_EXTI->FALL_PEND_CLR = (1u << EXTI_GPIO19);
    CMSDK_EXTI->RISE_PEND_CLR = (1u << EXTI_GPIO19);
    /* 4be0880 不写比较器和 Driver C。这里写 CTRL=0 会把电流通路关掉，冷机整场没电流。 */
    SysTick_Config(APB_Clock_Freq / LOD_SAMPLE_HZ);
    /* SysTick 优先级必须高于 UART1，避免 UART1_Handler 内调用 get_hsi_us() 时
       SysTick->VAL 已回绕重载而 g_tick_100us 尚未累加造成 100us 阶跃跳变。 */
    NVIC_SetPriority(SysTick_IRQn, 0);
    NVIC_SetPriority(UART1_IRQn, 1);
}

/* 主循环调用。未刺激报 0xFF；已锁存脱落则保持停输出。
   满窗之后：最长连续低电平达到 lod_low_min_ticks() 判贴合；连续 LOD_OPEN_WINDOWS 个窗不达标判脱落并切断。 */
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

    g_lead_off_ch2 = 0;
    if (max_run >= lod_low_min_ticks((uint8_t)g_freq)) {
        s_open_cnt = 0;
        s_contact_ok = 1;
        g_lead_off_ch0 = 0;
        return;
    }
    if (!s_contact_ok) {
        return;
    }
    if (++s_open_cnt >= LOD_OPEN_WINDOWS) {
#if (LOD_CUT_ENABLE != 0)
        g_lead_off_ch0 = 1;
        lod_cut_output();
#else
        (void)lod_cut_output;
        s_open_cnt = 0;
#endif
    }
}

static void stim_sync_watchdog_proc(void)
{
    if (g_stim_running) {
        if ((uint32_t)(g_tick_ms - s_sync_wdog_tick) > STIM_SYNC_WATCHDOG_MS) {
            wavegen_Stop(WAVE_GEN_DRVA_BLK0);
            lod_mark_stim_stop();
            s_sync_stopped = 1u;
            s_sync_stop_rep_tick = g_tick_ms;
            s_sync_stop_rep_cnt = 1u;
            lod_uplink(0x02); /* 0x02 = 同步信标超时自停，主动串口上报 */
        }
        return;
    }
    if ((s_sync_stopped != 0u) &&
        (s_sync_stop_rep_cnt < STIM_SYNC_STOP_REP_MAX) &&
        ((uint32_t)(g_tick_ms - s_sync_stop_rep_tick) >= STIM_SYNC_STOP_REP_MS)) {
        /* 首条自停上报若在串口丢包，周期重报直到主控下发显式 STOP/START 清除或达到上限 */
        s_sync_stop_rep_tick = g_tick_ms;
        s_sync_stop_rep_cnt++;
        lod_uplink(0x02);
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
	UART_RXCLR(CMSDK_UART1);
	rev_len = 0;
	rev_fin = 0;
	
	
	CMSDK_SYSCON->APB_CLKEN = 0x4000|0x1000|CMSDK_SYSCON->APB_CLKEN;
	stim_power_on();
	LOD_Init();
	lod_uplink(0xFF);

	while(1){
		wavegen_sync_proc();
		wavegen_slow_proc();
		stim_sync_watchdog_proc();
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
			  if(rev_buf[1] == 0x00 && (rev_buf[2] == 0x01 || rev_buf[2] == 0x02)){
				  uint8_t act = rev_buf[3];
				  uint8_t req_seq = (rev_buf[2] >= 0x02) ? rev_buf[4] : 0x00u;
				  if(act == 0x01){//start
					  s_stim_start_seq = req_seq;
					  wavegen_driverA_sine_test(WAVE_GEN_DRVA_BLK0,3);
					  lod_mark_stim_start();
				  }else if(act == 0x02){//pause
					  s_stim_start_seq = 0u;
					  wavegen_Stop(WAVE_GEN_DRVA_BLK0);
					  lod_mark_stim_stop();
				  }else if(act == 0x00){//stop
					  s_stim_start_seq = 0u;
					  wavegen_Stop(WAVE_GEN_DRVA_BLK0);
					  lod_mark_stim_stop();
				  }else{
					  act = 0xFFu;
				  }
				  if(act != 0xFFu){
					  send_buf[0]=0xA5;
					  send_buf[1]=0x09 ;
					  send_buf[2]=0x03 ;
					  send_buf[3]=0x00 ;
					  send_buf[4]=(uint8_t)(0x80u | act) ;
					  send_buf[5]=req_seq ;
					  send_buf[6]=crc8_calc(send_buf,6) ;
					  send_buf[7]=0x5A ;
					  send_len = 8;
					  for(count = 0 ; count < 200; count++){;}
					  Uart_Send(CMSDK_UART1,send_buf,send_len);
				  }
			  }else if(rev_buf[1] == 0x02 && rev_buf[2] == 0x03){//set param
					g_freq = rev_buf[4];
					if (g_freq < 1.0f) {
						g_freq = 1.0f;
					} else if (g_freq > 100.0f) {
						g_freq = 100.0f;
					}
					g_current = rev_buf[3];
					if (g_current < 1.0f) {
						g_current = 1.0f;
					} else if (g_current > 20.0f) {
						g_current = 20.0f;
					}
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
					
				}else if(rev_buf[1] == 0x05 && rev_buf[2] == 0x03){//1s crystal sync & phase lock beacon
					uint8_t sync_seq = rev_buf[3];
					uint8_t delay_100us = rev_buf[5];
					s_sync_target_us = g_rx_fin_us + ((uint32_t)delay_100us * 100u);
					s_sync_pending_seq = sync_seq;
					s_sync_pending = 1u;
					if (g_stim_running) {
						s_sync_wdog_tick = g_tick_ms;
					} else if (s_sync_stopped != 0u) {
						/* 主控仍在发送信标说明尚未收到先前的自停通知，立即重报自停状态 */
						s_sync_stop_rep_tick = g_tick_ms;
						lod_uplink(0x02);
					}
				}else if(rev_buf[1] == 0x03 && rev_buf[2] == 0x02 && rev_buf[3] == 0x01){
					/* 状态回读：原样回显请求编号 rev_buf[4]，区分主动开机 0xFF 与 STOP 后回读确认的 0xFF */
					uint8_t query_seq = rev_buf[4];
					lod_uplink_seq(g_stim_running ? 0x00u
					             : (s_sync_stopped ? 0x02u
					             : (s_open_latched ? 0x01u : 0xFFu)),
					             query_seq);
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


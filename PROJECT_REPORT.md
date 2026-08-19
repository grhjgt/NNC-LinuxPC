# ENS_UART_DEMO 项目报告

> 项目名称：ENS_UART_DEMO_run_ok_20260715  
> 报告日期：2026-08-19  
> 参考手册：《NNC6121 数据手册 A7-20260326》（Nanochap）  
> 目标芯片：NNC6121（可编程通用刺激双通道芯片）  

---

## 1. 项目概述

本项目是运行在 **NNC6121** 神经刺激芯片上的 UART 通信 + 电刺激波形发生 DEMO 固件。它演示了：

- 通过 UART1 接收上位机（主控 MCU）下发的自定义命令帧；
- 解析帧头/命令/长度/CRC8/帧尾，执行刺激启动/暂停/停止/参数设置/状态查询；
- 利用芯片内置 **任意波发生器（AWG）** 在 STIMU0/STIMU1 电极对上输出正弦刺激波形；
- 通过内部 BOOST 升压电路提供高压驱动电源（HVDD）。

该工程对应数据手册第 9 章"典型应用电路"中"通过 UART 和主控 MCU 进行通讯"的应用场景。手册明确指出：**UART 通讯方式对时钟要求较高，当需要通过 UART 和主控 MCU 进行通讯时，则需要使用外部晶振**；本 DEMO 默认使用内部 HSI 32MHz，工程中保留了对 HSE 外部晶振切换的支持代码（`SysHseSet` / `SwitchHseFun`）。

### 1.1 芯片定位

NNC6121 是一颗单芯片刺激装置，集成：

| 维度 | 规格 |
|------|------|
| 内核 | 32 位 ARM Cortex-M0，最高 32MHz |
| 存储 | 32KB MTP + 8KB SRAM |
| 刺激通道 | 2 个高范围驱动器（4 电极），最大 55V |
| 输出电流 | 33μA ~ 67mA，255 级，单位电流 33~264μA 可调（8 级） |
| 刺激频率 | 1 ~ 20kHz（正弦/三角/方波/任意波） |
| 脉宽 | 2μs ~ 无限 |
| 升压 | 集成 5~55V 高压 BOOST（内置/外置 MOS 可选） |
| 通讯 | UART×1（带硬件流控）/ SPI×2 / I2C×1 |
| 外设 | RTC、WDT、PWM、32 位定时器×2、双定时器、SysTick |
| 封装 | 64QFN 7×7，-40℃~85℃ |
| 应用 | TENS、IFT/IFC、EMS、肌肉强化与弱肌康复、可穿戴电刺激 |

---

## 2. 工程目录结构

工程基于 Keil MDK（ARMCC）开发，目录组织如下：

```
3、ENS_UART_DEMO_run_ok_20260715/
├── ENS001_BASIC_PRJ.uvprojx      # Keil 工程文件
├── Bin/                          # 输出固件
│   └── ENS001_BASIC_PRJ.bin
├── CORE/                         # Cortex-M0 内核与启动文件
│   ├── ARM/startup_CMSDK_CM0.s  # 启动文件
│   ├── GCC/startup_CMSDK_CM0.s
│   └── INCLUDE/                  # 内核头文件
│       ├── CMSDK_CM0.h           # 芯片寄存器定义（核心）
│       ├── core_cm0.h
│       └── system_CMSDK_CM0.h
├── FWLIB/                        # 固件库（BSP）
│   ├── include/                  # 外设头文件
│   │   ├── ENS1_UART.h / ENS1_CLOCK.h / ENS1_GPIO.h
│   │   ├── ENS1_ANAC.h / ENS1_BOOST.h / ENS1_ADC.h
│   │   ├── ENS1_SPI.h / ENS1_IIC.h / ENS1_PWM.h
│   │   ├── ENS1_TIMER.h / ENS1_EXTI.h / ENS1_WATCHDOG.h
│   │   └── ENS1_MTP.h / ENS_CURRENT_CALIBRATION.h
│   └── source/                   # 外设驱动实现
│       ├── ENS1_UART.c           # UART 驱动 + 中断处理（关键）
│       ├── ENS1_CLOCK.c          # 时钟初始化
│       ├── ENS1_ANAC.c           # 模拟控制（比较器/PGA/PMU/低压检测）
│       ├── ENS1_BOOST.c          # BOOST 升压配置
│       ├── ENS1_GPIO.c           # GPIO 复用/模拟通道
│       ├── ENS_ADC.c             # ADC 驱动
│       └── retarget.c            # printf 重定向到 UART1
├── USER/                         # 用户应用层
│   ├── mian.c                    # 主程序（核心，含波形发生与协议解析）
│   ├── ENS001_CONFIG.h           # 系统频率配置
│   └── MY_HEADER.h               # 公共类型定义
└── Objects/ / Listings/           # 编译产物
```

> 注：主文件名为 `mian.c`（疑似 `main.c` 笔误，但工程配置已匹配，不影响编译）。

---

## 3. 核心代码解析

### 3.1 主程序流程（[mian.c](file:///D:/Code/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/USER/mian.c)）

`main()` 函数执行流程：

```
启动
  │
  ├─ Delay_268ms(1)           # 上电延时等待电源稳定
  ├─ MTP_init()               # MTP 存储器初始化
  ├─ ClockInit()              # 时钟初始化：HSI 32MHz，APB/AHB 不分频
  ├─ UART_Init(UART1, ...)    # UART1 初始化：9600, 8N1, FIFO 8字节触发
  ├─ UART_ITConfig(UART1,...) # 使能 RLSI + RDAI 中断
  ├─ APB_CLKEN |= 0x4000|0x1000  # 使能 WAVE_GEN(位12) + ADC(位13) PCLK
  ├─ ANAC->BOOST_CTRL = 0x77213   # 内部 BOOST：26V, 256分频, 80%占空, 关外置MOS
  ├─ ANAC->PMU_CTRL = 0x10        # 使能 Bandgap 基准
  │
  └─ while(1) 主循环
       │
       └─ 若 rev_fin==1（收到完整帧）且 rev_len>0
            ├─ 校验 0xA5 帧头 / 0x5A 帧尾 / CRC8
            ├─ 按 cmd+length+data 分发命令
            ├─ 构造响应帧 Uart_Send 回送
            └─ 清空接收缓冲，等待下一帧
```

### 3.2 通信协议

本工程自定义了一套轻量帧协议，结构与收发逻辑如下：

#### 3.2.1 帧格式

```
┌──────┬──────┬──────┬─────────────┬──────┬──────┐
│ 帧头 │ CMD  │ LEN  │ DATA[LEN]    │ CRC8 │ 帧尾 │
│ 0xA5 │ 1B   │ 1B   │ LEN 字节    │ 1B   │ 0x5A │
└──────┴──────┴──────┴─────────────┴──────┴──────┘
```

- **帧头**：固定 `0xA5`
- **CMD**：命令字节（0x00 控制类 / 0x02 参数类 / 0x03 查询类）
- **LEN**：DATA 字段长度
- **DATA**：命令参数
- **CRC8**：对帧头到 DATA 末字节计算，多项式 0x07（查表法）
- **帧尾**：固定 `0x5A`

#### 3.2.2 命令列表

| 命令 (CMD, LEN, DATA[0]) | 含义 | 执行动作 | 响应 DATA |
|--------------------------|------|----------|-----------|
| 0x00, 0x01, 0x01 | 启动刺激 | `wavegen_driverA_sine_test(BLK0, 3)` | 0x00,0x00（成功） |
| 0x00, 0x01, 0x02 | 暂停刺激 | `wavegen_Stop(BLK0)` | 0x00,0x00 |
| 0x00, 0x01, 0x00 | 停止刺激 | `wavegen_Stop(BLK0)` | 0x00,0x00 |
| 0x02, 0x03, current, freq | 设置参数 | 更新 `g_current`/`g_freq` | 0x02,0x00 |
| 0x03, 0x01 | 状态查询 | — | 0x67, 0x0F(15) |

响应帧：`0xA5 | 0x09(或0x0A) | 0x02 | 状态1 | 状态2 | CRC8 | 0x5A`

#### 3.2.3 接收状态机（[ENS1_UART.c UART1_Handler](file:///D:/Code/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/FWLIB/source/ENS1_UART.c)）

UART1 中断在 FIFO 触发（8 字节）或接收超时时进入，按 `rev_len` 状态推进：

```
rev_len=0 且 byte==0xA5  → 存帧头，rev_len=1
rev_len=1                → 存 CMD，rev_len=2
rev_len=2                → 存 LEN，rev_len=3
rev_len∈[3, 3+LEN+2)     → 逐字节存入 DATA+CRC+帧尾
                           收满 3+LEN+2 字节 → rev_fin=1
```

主循环检测 `rev_fin` 后做整体 CRC 与帧尾校验，校验失败丢弃并复位状态。

### 3.3 波形发生（AWG）驱动

`wavegen_driverA_sine_test()` 操作 **WAVE_GEN_DRVA_BLK0**（基地址 0x4000C000），对应数据手册 8.17.1 节"任意波发生器"。核心配置：

#### 3.3.1 寄存器配置对照

| 寄存器 | 写入值 | 手册含义 |
|--------|--------|----------|
| `WAVE_GEN_DRV_REST_T_REG` | 0 | 正负半波间静息时间 0μs |
| `WAVE_GEN_DRV_SILENT_T_REG` | 0 | 周期前静默时间 0μs |
| `WAVE_GEN_DRV_CLK_FREQ_REG` | 0x20 (32) | AWG 时钟 32MHz（=PCLK） |
| `WAVE_GEN_DRV_HLF_WAVE_PRD_REG` | `(500/g_freq)*1000` | 正半波周期（μs） |
| `WAVE_GEN_DRV_NEG_HLF_WAVE_PRD_REG` | 同上 | 负半波周期（μs） |
| `WAVE_GEN_DRV_CONFIG_REG` | 0x4A | bit1 负向使能 + bit3 源B使能 + bit6 多电极（bit0/bit2 静息静默未使能，bit5=0 连续重复模式） |
| `WAVE_GEN_DRV_ISEL_REG` | 1 | 电流档位选择（低3位） |
| `WAVE_GEN_DRV_CTRL_REG` | 0x01 | 使能该波发生块 |

> 电流选择 ISEL 在 BLK0 对应使用 **WAVE_GEN_DRVC_BLK5**（0x4000C154）的低 3 位，符合手册"DRVA_ISEL0_EXT = 0x4000C154"的定义。

#### 3.3.2 正弦波查表

工程内置 64 点正弦半波表 `sine_data[64]`，幅值 0~255。写入时按电流缩放：

```c
WAVE_GEN_DRV_IN_WAVE_REG = (int)((float)sine_data[i] * 0.6455 * (g_current/20));
```

- `0.6455`：满量程缩放系数（约对应 20mA 基准下的幅度归一化）；
- `g_current/20`：按上位机下发电流值线性调节幅度；
- 64 点写入完成后由硬件 AWG 循环重复输出（CONFIG bit5=0 连续模式）。

这与手册 8.17.1.4 节给出的正弦波输入示例一致（64 点，末点为 0）。

#### 3.3.3 刺激频率换算

```
半波周期(μs) = (500 / g_freq) * 1000
```

- `g_freq` 默认 10.0 → 半波周期 50000μs = 50ms → 全波周期 100ms → 10Hz
- 频率越高，半波周期越短。该公式将上位机频率参数映射为 AWG 半波周期寄存器值。

#### 3.3.4 停止函数

`wavegen_Stop()` 将 ISEL 清零、64 点波形数据清零，使输出归零。

### 3.4 升压电路配置

`main()` 中直接写寄存器配置内部 BOOST（对应手册 6.2 节、8.16 节）：

```c
CMSDK_ANAC->BOOST_CTRL = 0x77213;   // 内部 boost
CMSDK_ANAC->PMU_CTRL   = 0x10;      // 使能 Bandgap
```

- `0x77213` 解析：占空比相关字段约 80%（1-20%）、256 分频、目标 26V、关闭外置 MOS 模式；
- `PMU_CTRL=0x10`：置位 bit4 使能 Bandgap 基准电压源，为 BOOST 提供参考。

固件库 `ENS1_BOOST.c` 中的 `Boost_Voltage_Sel()` 提供了 11V/15V/26V/45V/55V 预设，本工程直接使用 26V 内部升压。

### 3.5 时钟系统

`ClockInit()` 配置（[ENS1_CLOCK.c](file:///D:/Code/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/3%E3%80%81ENS_UART_DEMO_run_ok_20260715/FWLIB/source/ENS1_CLOCK.c)）：

- 时钟源：HSI（内部 RC）32MHz
- 系统时钟选择：HSI → SYSCLK
- AHB 分频：/1 → HCLK = 32MHz
- APB 分频：/1 → PCLK = 32MHz
- `APB_Clock_Freq` 全局变量用于 UART 波特率计算

**外部晶振切换**：`SysHseSet()` 将 GPIO0 复用为 HSE_CLK，切换 SYSCLK 到 HSE 并关闭 HSI。`SwitchHseFun()` 提供带延时（5~30s）的切换封装。该功能当前被注释未启用，符合手册"UART 通讯建议使用外部晶振"的预留设计。

### 3.6 UART 驱动

`ENS1_UART.c` 实现完整 UART 驱动：

- **初始化** `UART_Init()`：GPIO 复用（UART1_RX=GPIO12, UART1_TX=GPIO13, ALT_FUNC1）、波特率除数计算（`APB_Clock_Freq / BaudRate / OverSamp - 1`）、FIFO 触发等级、停止位/数据位/校验位配置；
- **中断** `UART_ITConfig()`：使能 RLSI（接收线状态）+ RDAI（数据就绪）中断，挂接到 NVIC；
- **中断处理** `UART1_Handler()`：读 FIFO 数据量 → 逐字节状态机组帧 → 置 `rev_fin`；
- **发送** `Uart_Send()`：轮询 THRE 空标志逐字节发送。

当前配置：波特率 9600、8 数据位、1 停止位、无校验、FIFO 8 字节触发。

### 3.7 retarget.c（printf 重定向）

将标准 `printf`/`fputc` 重定向到 UART1，便于调试输出。本 DEMO 主循环未使用 printf，但保留了该能力。

---

## 4. 关键全局变量

| 变量 | 类型 | 作用 |
|------|------|------|
| `rev_buf[16]` | uint8_t | 接收帧缓冲 |
| `rev_len` | uint8_t | 接收状态机进度 |
| `rev_fin` | uint8_t | 帧接收完成标志 |
| `send_buf[16]` | uint8_t | 发送帧缓冲 |
| `g_freq` | float | 刺激频率参数（默认 10.0） |
| `g_current` | float | 刺激电流参数（默认 10.0） |
| `APB_Clock_Freq` | uint32_t | APB 时钟频率（波特率计算用） |

---

## 5. 数据手册对照与符合性

| 功能点 | 代码实现 | 手册章节 | 符合性 |
|--------|----------|----------|--------|
| Cortex-M0 内核 | startup_CMSDK_CM0.s | 8.4 | ✅ |
| HSI 32MHz 时钟 | ClockInit() | 8.3.3 | ✅ |
| MTP 启动 | MTP_init() + BOOT 默认 | 8.2 / 8.1.3 | ✅ |
| UART1 GPIO12/13 | UART_Init ALT_FUNC1 | 表2 引脚11/12 | ✅ |
| UART FIFO 8字节触发 | UART1_Fifo.level=bytes_8 | 8.9.3.3 | ✅ |
| UART 波特率除数公式 | DLL/DLH 计算 | 8.9.3.1 | ✅ |
| WAVE_GEN DRVA_BLK0 | 基址 0x4000C000 | 8.17.1.3 | ✅ |
| ISEL via DRVC_BLK5 | 0x4000C154 | 8.17.1.3 | ✅ |
| 64 点波形表 | sine_data[64] | 8.17.1.4 | ✅ |
| CONFIG 寄存器位定义 | 0x4A | 8.17.1.3 | ✅ |
| 内部 BOOST 26V | BOOST_CTRL=0x77213 | 6.2 / 8.16 | ✅ |
| Bandgap 使能 | PMU_CTRL=0x10 | 8.16 | ✅ |
| HSE 切换流程 | SysHseSet() | A7 版本新增 | ✅ |

---

## 6. 工程文件依赖关系

```
mian.c
  ├── MY_HEADER.h        (类型/枚举/extern 声明)
  ├── ENS1_MTP.h         (MTP 初始化)
  ├── ENS1_CLOCK.h       (ClockInit/PCLK_Enable)
  ├── ENS1_ANAC.h        (ANAC 寄存器)
  └── ENS1_UART.h        (UART 驱动)
        ├── ENS1_CLOCK.h
        ├── ENS1_GPIO.h   (GPIO 复用)
        └── CMSDK_CM0.h   (寄存器定义, 含 WAVE_GEN/ANAC/SYSCON)
```

`CMSDK_CM0.h` 是芯片级寄存器定义头文件，包含所有外设（UART/SPI/I2C/GPIO/WAVE_GEN/ANAC/ADC/TIMER/PWM/RTC/WDT/EXTI）的结构体与基地址宏。

---

## 7. 注意事项与设计说明

### 7.1 时钟与 UART 精度

手册第 9 章明确指出 UART 通讯对时钟要求较高，建议使用外部晶振。本 DEMO 默认使用 HSI 内部 RC（精度约 ±1~2%），在 9600 波特率下尚可工作，但若提升至 115200 或长距离通讯，建议启用 `SwitchHseFun()` 切换到 HSE 外部晶振以保证波特率精度。

### 7.2 发热功率

手册第 10 章"注意事项"给出发热功率计算方法：

```
有效输出占比 = (正脉宽 + 负脉宽) / (正脉宽 + 死区 + 负脉宽 + 静默)
平均电流 = 负载电流 × 有效输出占比
发热功率 = (HVDD - 负载阻抗×平均电流) × 平均电流 × 通道数
```

内部 BOOST 推荐最大发热功率 ≤ 2.0W，HVDD=26V 时建议单通道、负载 ≥ 500Ω 长时间工作。本 DEMO 静息时间与静默时间均设为 0，有效输出占比高，需注意长时间运行散热。

### 7.3 CRC8 校验

采用标准 CRC-8（多项式 0x07，初值 0x00，查表法），覆盖帧头到 DATA 末字节。帧尾 0x5A 不参与 CRC 计算。

### 7.4 电流参数映射

`g_current` 上位机参数（默认 10.0）经 `/20` 归一化后乘以正弦表值与系数 0.6455 写入 8 位波形寄存器。实际输出电流由 ISEL 档位（当前固定为 1）与波形幅度共同决定，需结合手册 8.17.2.1 节"高范围驱动器"参数标定。

---

## 8. 编译与烧录

- **IDE**：Keil MDK-ARM（工程文件 `ENS001_BASIC_PRJ.uvprojx`）
- **编译器**：ARMCC（retarget.c 中 `#if defined(__CC_ARM)` 分支）
- **输出**：`Objects/ENS001_BASIC_PRJ.axf`（ELF）、`Objects/ENS001_BASIC_PRJ.hex`、`Bin/ENS001_BASIC_PRJ.bin`
- **烧录**：通过 JLink（`JLinkSettings.ini` / `JLinkLog.txt` 已配置）
- **启动模式**：BOOT0=0, BOOT1=0 → 从 MTP 基地址启动

---

## 9. 结论

本项目是一个功能完整的 NNC6121 UART 刺激控制 DEMO，验证了：

1. **UART 帧协议通信**：自定义 0xA5...0x5A 帧 + CRC8，中断接收 + 主循环解析架构清晰；
2. **AWG 正弦波刺激**：64 点查表 + 频率/电流参数化配置，符合手册 AWG 设计规范；
3. **内部 BOOST 升压**：26V 内置 MOS 模式，配置简洁；
4. **时钟管理**：HSI 32MHz 默认 + HSE 切换预留，兼顾易用性与精度需求。

该 DEMO 可作为 NNC6121 芯片 UART 控制刺激的基础应用模板，后续可在此基础上扩展多通道、多波形（方波/三角波）、脱落检测、外部 BOOST 高压模式等功能。

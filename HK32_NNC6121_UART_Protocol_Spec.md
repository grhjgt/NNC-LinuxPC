# HK32 与 NNC6121 串口通信及电流校准对接协议规范

> **版本**: V1.0  
> **串口参数**: 波特率 9600 bps，数据位 8，停止位 1，无校验 (8N1)  
> **通信主从关系**: HK32 (主控 MCU / Master) $\rightarrow$ NNC6121 (电刺激芯片 / Slave)  

---

## 1. 通信数据包基础帧格式

所有 UART 通信帧均采用自定义校验帧结构，包头固定为 `0xA5`，包尾固定为 `0x5A`。

### 1.1 HK32 发送请求帧格式

| 字节偏移 | 字段名 | 字节数 | 示例值 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| `0` | **Frame Header** | 1 | `0xA5` | 固定包头 |
| `1` | **Cmd ID** | 1 | `0x04` / `0x05` ... | 命令类型字 |
| `2` | **Data Length** | 1 | `N` | 后续数据 Payload 长度 (字节数) |
| `3 ~ 3+N-1` | **Payload Data** | N | `...` | 具体的命令数据字节 |
| `3+N` | **CRC8** | 1 | `0xXX` | 前 `3+N` 字节的 CRC8 校验值 |
| `3+N+1` | **Frame Tail** | 1 | `0x5A` | 固定包尾 |

### 1.2 NNC6121 应答 (ACK) 帧格式

| 字节偏移 | 字段名 | 字节数 | 示例值 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| `0` | **Frame Header** | 1 | `0xA5` | 固定包头 |
| `1` | **Response Flag** | 1 | `0x09` / `0x0A` | 固定应答标识标志 |
| `2` | **Payload Length**| 1 | `M` | 应答载荷字节数 |
| `3` | **Echo Cmd ID** | 1 | `0x04` / `0x05` ... | 对应的请求命令字 |
| `4` | **Status Code** | 1 | `0x00` / `0x01` ... | 执行状态码 (0x00 代表成功) |
| `5 ~ 3+M-1` | **Response Data**| M-2 | `...` | 状态扩展与数据字段 |
| `3+M` | **CRC8** | 1 | `0xXX` | CRC8 校验值 |
| `3+M+1` | **Frame Tail** | 1 | `0x5A` | 固定包尾 |

---

## 2. 核心校准指令对接说明

### 2.1 指令 `0x04`: 多次 RAM 电流逼近校准 (不写入 MTP)

在生产测试或软件校准阶段，HK32 测量输出电流后，通过此指令多次下发实测值。NNC6121 会根据四舍五入逼近算法在 **RAM 中动态更新单元电流**并实时对波形生效，**绝不消耗 MTP 写入寿命**。

#### 1) HK32 请求帧格式 (14 字节 / 7 字节)

```text
帧结构: A5 04 03 target_mA measured_hi measured_lo CRC 5A
```

* `Cmd ID` = `0x04`
* `Data Length` = `0x03`
* `Payload`:
  * `Byte 3` (`target_mA`): 目标期望电流 (单位: mA，范围 `1` ~ `20`)。
    * 💡 **特殊功能（软复位）**：若下发 `target_mA = 0xFF`，NNC6121 将一键将 RAM 中的校准值重置为默认值 (`33`μA)，用于撤销本次不满意的逼近过程。
  * `Byte 4~5` (`measured_x100`): 外部高精度采样表测得的实际电流 $\times 100$（16位大端格式）。
    * *例：实际测得 $2.00\text{mA}$，填 $200 = \text{0x0096}$；实际测得 $1.50\text{mA}$，填 $150 = \text{0x0096}$。*

#### 2) NNC6121 ACK 响应帧格式

* **校准成功应答 (14 字节长帧)**:
  ```text
  A5 09 08 04 00 unit ftH ftL pH pL ISEL peak CRC 5A
  ```
  * `Status` = `0x00` (成功)
  * `unit`: 逼近校准后的 RAM 单元电流 (单位: μA)
  * `ftH ftL`: 计算得到的 FT 预测值 ($16\text{bit} = \text{unit} \times 832$)
  * `pH pL`: 在当前单元电流下产生的全标度理论电流 (单位: μA)
  * `ISEL`: 当前硬件档位 (低3位)
  * `peak`: 正弦波表峰值档位

* **校准失败应答 (7 字节短帧)**:
  ```text
  A5 09 02 04 01 CRC 5A
  ```
  * `Status` = `0x01` (NACK 参数非法或计算出的 `unit` 超出 $3 \sim 250\mu\text{A}$ 合理范围)

---

### 2.2 指令 `0x05`: 一键落盘指令 (存入 MTP)

当 HK32 进行多轮 `0x04` 校准满意后，下发 `0x05` 指令。NNC6121 将把当前 RAM 中的校准参数写进芯片内部的 MTP 非易失存储器中。

#### 1) HK32 请求帧格式

```text
帧结构: A5 05 01 ch_mask CRC 5A
```

* `Cmd ID` = `0x05`
* `Data Length` = `0x01`
* `Payload`:
  * `Byte 3` (`ch_mask`): 通道掩码，Bit0 对应通道 0 (通常填 `0x01` 或 `0x0F`)。

#### 2) NNC6121 ACK 响应帧格式 (10 字节)

```text
帧结构: A5 09 04 05 ST unit ftH ftL CRC 5A
```

| 状态码 `ST` | 状态说明 | 处理建议 |
| :--- | :--- | :--- |
| `0x00` | **成功落盘 (Success)** | MTP 擦除、编程写与读回校验全部成功。 |
| `0x02` | **钳制警告 (CLAMPED)** | 数据已安全写入 MTP，但由于 `unit > 78` 或 `< 10` 被安全钳制为 78μA。 |
| `0x11` | **扇区擦除失败** | MTP Sector 6 擦除异常。 |
| `0x12` | **编程写入失败** | MTP 硬件写超时。 |
| `0x13` | **读回校验失败** | MTP 写入后与期望值比对不一致。 |

---

## 3. 其他常规控制指令速查

### 3.1 指令 `0x00`: 波形使能与停止控制

* **HK32 发送请求**: `A5 00 01 TYPE CRC 5A`
  * `TYPE = 0x01`: 启动电刺激波形输出
  * `TYPE = 0x02`: 暂停电刺激波形输出
  * `TYPE = 0x00`: 停止电刺激波形输出
* **NNC6121 应答**: `A5 09 02 00 00 CRC 5A` (`ST=0x00` 成功)

### 3.2 指令 `0x02`: 设置目标电流与频率

* **HK32 发送请求**: `A5 02 02 g_current g_freq CRC 5A`
  * `g_current`: 幅值设置 (例如 `20` 代表 2.0mA)
  * `g_freq`: 频率设置 (例如 `25` 代表 2.5kHz)
* **NNC6121 应答**: `A5 09 02 02 00 CRC 5A` (`ST=0x00` 成功)

### 3.3 指令 `0x03`: 外设运行状态查询

* **HK32 发送请求**: `A5 03 01 00 CRC 5A`
* **NNC6121 应答**: `A5 0A 02 STATUS FAULT CRC 5A`

---

## 4. 标准产线校准推荐对接流程

```mermaid
sequence diagram
    autonumber
    participant App as 产线测试工装/HK32
    participant NNC as NNC6121 芯片
    participant Meter as 外部采样电流表

    Note over App, NNC: 步骤 1: 设置输出基准波形
    App->>NNC: 发送 0x02 指令 (设置电流 2.0mA, 频率 2.5kHz)
    App->>NNC: 发送 0x00 指令 (启动波形输出)
    
    Note over App, NNC: 步骤 2: 多次 RAM 迭代逼近校准 (不写 MTP)
    loop 逼近 2~3 次直至误差符合要求
        App->>Meter: 读取电流表实测值 (例 1.85mA -> 185)
        App->>NNC: 发送 0x04 指令 (target=2, measured=185)
        NNC->>NNC: 更新 RAM 中的 unit_current (不写 MTP)
        NNC-->>App: 应答 ACK (返回新 unit 与预测值)
    end
    
    Note over App, NNC: 步骤 3: 确定无误后一键落盘
    App->>NNC: 发送 0x05 指令 (落盘存入 MTP)
    NNC->>NNC: 擦除 Sector 6 -> 编程 4 通道 FT -> 读回校验
    NNC-->>App: 应答 ACK (ST=0x00 代表真正写入成功)
```

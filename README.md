# STM32 交通信号灯

基于 STM32F103 的交通信号灯模拟工程，使用红、黄、绿三路 LED 实现正常通行状态机，并通过 OLED 和串口输出当前状态。项目使用 Keil MDK 工程文件管理，依赖 STM32F10x 标准外设库。

## 功能特性

- 绿灯、黄灯、红灯按固定时长循环切换
- OLED 显示当前灯色和倒计时
- USART1 每秒输出当前状态和灯光信息
- PB1 按键切换夜间模式
- 夜间模式下黄灯以 0.5 秒节拍闪烁，红灯和绿灯关闭

## 硬件连接

| 模块 | 引脚 | 说明 |
| --- | --- | --- |
| 红灯 | PA4 | 低电平点亮 |
| 黄灯 | PA5 | 低电平点亮 |
| 绿灯 | PA6 | 低电平点亮 |
| 按键 | PB1 | 上拉输入，低电平按下 |
| OLED SCL | PB8 | 软件 I2C |
| OLED SDA | PB9 | 软件 I2C |
| USART1 TX | PA9 | 串口发送 |
| USART1 RX | PA10 | 串口接收 |

串口参数：`9600-8-N-1`。

## 状态逻辑

正常模式的状态机顺序如下：

```text
GREEN 30s -> YELLOW 3s -> RED 30s -> GREEN ...
```

主程序使用 `TIM2` 产生 0.5 秒节拍：

- 每 2 个节拍递减一次倒计时
- 每 2 个节拍通过串口输出一次状态
- 夜间模式下每个节拍翻转一次黄灯状态，实现 0.5 秒闪烁

按键行为：

- 按住 PB1 约 1 秒切换进入或退出夜间模式
- 夜间模式下短按 PB1 会退出夜间模式并回到绿灯初始状态

## 目录结构

```text
.
├── Project.uvprojx          # Keil MDK 工程文件
├── Project.uvoptx           # Keil MDK 工程选项
├── User/
│   ├── main.c               # 主程序、状态机、TIM2 中断
│   └── stm32f10x_conf.h     # 标准外设库配置
├── Hardware/
│   ├── LED.c/.h             # 板载 LED 和交通灯 GPIO 驱动
│   ├── Key.c/.h             # 按键初始化与读取
│   ├── OLED.c/.h            # OLED 显示驱动
│   └── Serial.c/.h          # USART1 串口驱动
├── System/
│   └── Delay.c/.h           # 延时函数
├── Start/                   # CMSIS 和启动文件
├── Library/                 # STM32F10x 标准外设库
└── docs/                    # 设计文档
```

## 编译与下载

1. 使用 Keil MDK 打开 `Project.uvprojx`。
2. 确认目标芯片与开发板实际型号一致。
3. 编译工程。
4. 通过 ST-Link、J-Link 或其他下载器烧录到 STM32F103 开发板。
5. 打开串口助手，选择 `9600` 波特率、8 数据位、无校验、1 停止位。

## 串口输出示例

```text
[GREEN] Countdown: 30s | R:OFF Y:OFF G:ON
[GREEN] Countdown: 29s | R:OFF Y:OFF G:ON
[YELLOW] Countdown: 03s | R:OFF Y:ON  G:OFF
[RED] Countdown: 30s | R:ON  Y:OFF G:OFF
[NIGHT] Yellow Blinking | R:OFF Y:BLINK G:OFF
```

## 主要参数

这些参数定义在 `User/main.c`：

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `GREEN_TIME` | `30` | 绿灯持续时间，单位秒 |
| `YELLOW_TIME` | `3` | 黄灯持续时间，单位秒 |
| `RED_TIME` | `30` | 红灯持续时间，单位秒 |

## 许可证

本项目使用 MIT License，详见 `LICENSE`。

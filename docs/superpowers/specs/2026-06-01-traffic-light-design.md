# 交通信号灯设计文档

## 概述

使用 STM32F103 开发板的 RGB LED（三路独立 LED）模拟交通信号灯，TIM2 定时器产生 1s 时间基准，状态机驱动灯色循环，串口实时输出倒计时与灯状态。支持按键切换夜间模式。

## 硬件映射

| 外设  | GPIO | 说明        |
|-------|------|-------------|
| 红灯  | PB5  | 推挽输出    |
| 黄灯  | PB7  | 推挽输出    |
| 绿灯  | PB8  | 推挽输出    |
| 按键  | PB1  | 上拉输入，夜间模式切换 |
| USART1| PA9/PA10 | 9600-8-N-1 |

## 软件架构

### 文件规划

```
User/main.c              — 主程序 + 状态机逻辑（唯一修改文件）
Hardware/LED.h           — 扩展：添加交通灯宏与 GPIO 声明
Hardware/LED.c           — 扩展：添加 PB5/PB7/PB8 初始化
```

**不改动** Serial、Delay、Key、OLED 等已有模块。

### 状态机

```
GREEN ──(30s)──→ YELLOW ──(3s)──→ RED ──(30s)──→ GREEN ...
```

每个状态持有：灯色配置 + 持续时间（秒）+ 状态名称。

```
夜间模式：GREEN/YELLOW/RED 中任意状态 → 黄灯 0.5s 闪烁 → 返回原状态
```

### 定时器

TIM2 配置为 1s 周期中断：
- 预分频：7200（72MHz / 7200 = 10kHz）
- 自动重装载：10000（10kHz / 10000 = 1Hz）
- 中断内：`tim1sFlag = 1`，通知主循环执行倒计时

### 主循环流程

```
1. 检查 tim1sFlag → 递减倒计时
2. 倒计时归零 → 切换下一状态
3. 每秒 Serial_Printf 输出：[GREEN] Countdown: 25s | R:OFF Y:OFF G:ON
4. 检测按键长按（>1s）→ 进入/退出夜间模式
5. OLED 刷新显示阶段名 + 倒计时
```

### 夜间模式

- 长按 PB1（按下持续 >1s）切换夜间模式
- 夜间模式：黄灯以 0.5s 周期闪烁（亮 500ms / 灭 500ms），其它灯全灭
- 短按 PB1 退出夜间模式，返回正常状态机

### 串口输出格式

```
[GREEN] Countdown: 25s | R:OFF Y:OFF G:ON
[YELLOW] Countdown: 02s | R:OFF Y:ON  G:OFF
[RED]    Countdown: 30s | R:ON  Y:OFF G:OFF
[NIGHT]  Blinking Yellow  | R:OFF Y:BLINK G:OFF
```

## 关键常量

| 常量          | 值   |
|---------------|------|
| GREEN_TIME    | 30s  |
| YELLOW_TIME   | 3s   |
| RED_TIME      | 30s  |
| NIGHT_BLINK   | 500ms|

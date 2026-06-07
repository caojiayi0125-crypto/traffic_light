# 串口控制台 + 动态配时 设计文档

## 概述

在现有交通信号灯基础上新增两个功能模块：
- **串口命令行（方案A）**：通过 USART1 接收命令，实时控制信号灯行为
- **智能动态配时（方案B）**：支持4种配时模式，通过命令切换，模拟真实路口场景
- **统筹调度层**：新增 controller 模块，负责组合各模块、驱动主循环

不依赖按键，所有交互通过串口完成。

## 硬件映射

沿用现有配置，无新增硬件。

| 外设   | GPIO       | 说明       |
|--------|------------|------------|
| 红灯   | PA4        | 推挽输出   |
| 黄灯   | PA5        | 推挽输出   |
| 绿灯   | PA6        | 推挽输出   |
| OLED   | PB8/PB9    | I2C 模拟   |
| USART1 | PA9/PA10   | 9600-8-N-1，收发双工 |

## 软件架构

### 文件规划

```
App/
├── traffic_light.h          — 保留：TrafficState 枚举、GREEN_TIME 等时间宏
├── traffic_light.c          — 精简：TIM2 初始化 + 全局 tim1sFlag + LED辅助函数
├── command.h / .c           — 方案A：串口命令行解析与分发
├── scheduler.h / .c         — 方案B：多模式动态配时状态机
└── controller.h / .c        — 统筹调度层：初始化所有模块，驱动主循环
```

`Driver/` 目录不改动。

### 模块接口

#### scheduler（智能配时引擎）

```
void Sched_Init(void);
void Sched_SetMode(SchedMode mode);
SchedMode Sched_GetMode(void);
const char *Sched_GetModeName(void);
void Sched_Tick(void);           // 每秒调用，推进状态机
TrafficState Sched_GetState(void);
uint32_t Sched_GetCountdown(void);
```

配时配置表（每种模式有独立的 Green/Yellow/Red 时长）：

| 模式     | 绿灯  | 黄灯 | 红灯  | 说明           |
|----------|-------|------|-------|----------------|
| NORMAL   | 30s   | 3s   | 30s   | 默认标准配时   |
| IDLE     | 10s   | 3s   | 10s   | 闲时快速轮转   |
| RUSH     | 60s   | 3s   | 30s   | 干线方向延长   |
| NIGHT    | —     | 0.5s | —     | 黄灯慢闪       |

NIGHT 模式不参与状态机切换，黄灯以 0.5s 周期闪烁（每 0.5s 翻转一次），其他灯全灭。

#### command（串口命令行）

```
void Cmd_Init(void);
void Cmd_Process(void);      // 非阻塞，每轮主循环调用
```

支持的命令（`\r\n` 结尾）：

| 命令           | 功能                          |
|----------------|-------------------------------|
| `mode normal`  | 切换到标准配时模式            |
| `mode rush`    | 切换到高峰配时模式            |
| `mode idle`    | 切换到闲时配时模式            |
| `mode night`   | 切换到夜间黄闪模式            |
| `status`       | 查询当前模式、灯色、倒计时    |
| `help`         | 打印全部命令及说明            |

命令表驱动实现：每个命令一条 `{name, handler, help}` 结构体，`Cmd_Process` 逐字节接收、遇 `\r\n` 后匹配执行，不匹配则返回 `Unknown command` 提示。

#### controller（统筹调度）

```
void Ctrl_Init(void);
void Ctrl_Run(void);         // 永不返回的主循环
```

主循环：

```
while (1):
  if tim1sFlag:
    tim1sFlag = 0
    Sched_Tick()
    更新 LED（根据 Sched_GetState）
    串口打印状态（沿用现有格式）
    OLED 显示模式名 + 倒计时数字
  Cmd_Process()              // 非阻塞轮询
```

初始化流程：`LED_Init → OLED_Init → Serial_Init → TIM2_Init → Sched_Init → Cmd_Init`

#### traffic_light（基础设施）

精简后仅保留：
- `TrafficState` 枚举（GREEN / YELLOW / RED）
- `GREEN_TIME` / `YELLOW_TIME` / `RED_TIME` 宏（NORMAL 模式使用）
- `TIM2_Init()` 函数 + `tim1sFlag` 全局变量
- `set_led_by_state()`、`led_all_off()` 辅助函数

原有状态机逻辑迁移至 `scheduler.c`。

### 数据流

```
串口 RXNE 中断 → 环形缓冲区 → Cmd_Process() 逐字节读 → 匹配命令 → Sched_SetMode()
                                                                          ↓
TIM2 中断 → tim1sFlag = 1 → Ctrl_Run() 检出 → Sched_Tick() 推进状态机
                                                  ↓
                  LED 更新   ← Sched_GetState()
                  OLED 刷新  ← Sched_GetCountdown() + Sched_GetModeName()
                  USART 输出 ← Serial_Printf(state, countdown)
```

### 夜间模式细节

- NIGHT 模式下 `Sched_Tick` 不推进 Green→Yellow→Red 状态机
- 黄灯每 500ms 翻转一次亮灭（利用 tim1sFlag 计数到 0.5s 不可整除时用分频处理，实际实现使用 1s 周期内两次翻转：每 tick 翻一次即可近似 1s 周期闪烁；如需精确 0.5s，可将 TIM2 改为 500ms 时基，NIGHT 模式启用）

> 实现时采用简化方案：NIGHT 模式维持 1s tick，黄灯每 tick 翻转一次（1s 周期亮灭），视觉效果与 0.5s 无本质差异，且代码更简洁。

### 串口输出格式

```
=== Traffic Light Console ===
Commands: mode <normal|rush|idle|night> | status | help
> mode rush
[MODE] Switched to RUSH
[RUSH|GREEN] Countdown: 58s | R:OFF Y:OFF G:ON
> status
[MODE] RUSH | State: GREEN | Countdown: 25s
```

## 约束

- 不改动 `Driver/` 目录下任何文件
- 不引入按键相关代码
- 各模块通过头文件声明接口，`.c` 文件封装实现细节
- 模块间通过函数调用通信，不引入全局变量（`tim1sFlag` 除外）

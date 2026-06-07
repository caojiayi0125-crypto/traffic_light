# 串口控制台 + 动态配时 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 新增串口命令行控制台和4种动态配时模式，通过 controller 模块统筹调度。

**Architecture:** 3 个新模块 + 1 个重构模块。`scheduler` 管理配时状态机，`command` 解析串口命令，`controller` 统筹调度。`traffic_light` 精简为纯基础设施层。

**Tech Stack:** STM32F103 / ST 标准外设库 / Keil MDK

---

### Task 1: 重构 traffic_light.h — 导出 tim1sFlag 和辅助函数

**Files:**
- Modify: `App/traffic_light.h`

- [ ] **Step 1: 重构 traffic_light.h**

当前文件保留 TrafficState 枚举和时间宏，新增 extern 声明，移除 TrafficLight_Init/TrafficLight_Run 声明（这两个函数将被 controller 替代）：

```c
#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H

#include "stm32f10x.h"

/** @brief 绿灯持续时间（秒）。 */
#define GREEN_TIME    30
/** @brief 黄灯持续时间（秒）。 */
#define YELLOW_TIME    3
/** @brief 红灯持续时间（秒）。 */
#define RED_TIME      30

/**
  * @brief 交通信号灯状态枚举。
  */
typedef enum {
	STATE_GREEN,   /**< 绿灯状态。 */
	STATE_YELLOW,  /**< 黄灯状态。 */
	STATE_RED      /**< 红灯状态。 */
} TrafficState;

/** @brief 1秒时基标志（TIM2中断置1，主循环清零）。 */
extern volatile uint8_t tim1sFlag;

/**
  * @brief  初始化TIM2产生1秒时基中断。
  * @param  无。
  * @retval 无。
  */
void TIM2_Init(void);

/**
  * @brief  根据当前状态点亮对应的LED。
  * @param  state 当前交通灯状态。
  * @retval 无。
  */
void set_led_by_state(TrafficState state);

/**
  * @brief  熄灭所有LED。
  * @param  无。
  * @retval 无。
  */
void led_all_off(void);

#endif
```

- [ ] **Step 2: 验证文件语法**

```bash
gcc -fsyntax-only -I./Library -I./Driver -I./App App/traffic_light.h
```

- [ ] **Step 3: 提交**

```bash
git add App/traffic_light.h
git commit -m "refactor: 精简 traffic_light.h，导出 tim1sFlag 和辅助函数"
```

---

### Task 2: 重构 traffic_light.c — 移除状态机逻辑

**Files:**
- Modify: `App/traffic_light.c`

- [ ] **Step 1: 重写 traffic_light.c**

移除 `TrafficLight_Init`、`TrafficLight_Run`、`TrafficState` 枚举（已移至 .h）、状态机函数（`next_state`、`state_duration`、`state_name`、`print_status`）。保留 `TIM2_Init`、`led_all_off`、`set_led_by_state` 和 `tim1sFlag`，去掉 static 修饰：

```c
#include "traffic_light.h"
#include "LED.h"

/** @brief TIM2的预分频值（72MHz / 7200 = 10kHz）。 */
#define TIM2_PRESCALER    7199
/** @brief TIM2的自动重装载值（10kHz / 10000 = 1Hz）。 */
#define TIM2_PERIOD       9999

/** @brief 1秒时基标志（TIM2中断置1，主循环清零）。 */
volatile uint8_t tim1sFlag;

/**
  * @brief  配置TIM2产生1秒时基中断。
  * @param  无。
  * @retval 无。
  */
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = TIM2_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = TIM2_PRESCALER;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  熄灭所有LED。
  * @param  无。
  * @retval 无。
  */
void led_all_off(void)
{
	led_off(LED_RED);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}

/**
  * @brief  根据当前状态点亮对应的LED。
  * @param  state 当前交通灯状态。
  * @retval 无。
  */
void set_led_by_state(TrafficState state)
{
	led_all_off();
	switch (state) {
	case STATE_GREEN:  led_on(LED_GREEN);  break;
	case STATE_YELLOW: led_on(LED_YELLOW); break;
	case STATE_RED:    led_on(LED_RED);    break;
	}
}

/**
  * @brief  TIM2中断服务函数。
  * @param  无。
  * @retval 无。
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		tim1sFlag = 1;
	}
}
```

- [ ] **Step 2: 提交**

```bash
git add App/traffic_light.c
git commit -m "refactor: 精简 traffic_light.c，移除状态机，保留定时器和LED辅助函数"
```

---

### Task 3: 创建 scheduler 模块头文件

**Files:**
- Create: `App/scheduler.h`

- [ ] **Step 1: 编写 scheduler.h**

```c
#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "stm32f10x.h"
#include "traffic_light.h"

/**
  * @brief 配时模式枚举。
  */
typedef enum {
	MODE_NORMAL,  /**< 标准模式：绿30s / 黄3s / 红30s。 */
	MODE_IDLE,    /**< 闲时模式：绿10s / 黄3s / 红10s。 */
	MODE_RUSH,    /**< 高峰模式：绿60s / 黄3s / 红30s。 */
	MODE_NIGHT    /**< 夜间模式：黄灯闪烁，其他灯全灭。 */
} SchedMode;

/**
  * @brief  初始化调度器，默认NORMAL模式，从绿灯开始。
  * @param  无。
  * @retval 无。
  */
void Sched_Init(void);

/**
  * @brief  切换配时模式，重置状态机到绿灯（夜间模式除外）。
  * @param  mode 目标配时模式。
  * @retval 无。
  */
void Sched_SetMode(SchedMode mode);

/**
  * @brief  获取当前配时模式。
  * @param  无。
  * @retval 当前配时模式。
  */
SchedMode Sched_GetMode(void);

/**
  * @brief  获取当前模式名称字符串。
  * @param  无。
  * @retval 模式名称（大写英文）。
  */
const char *Sched_GetModeName(void);

/**
  * @brief  每秒调用一次，推进状态机。
  * @note   NORMAL/IDLE/RUSH模式下递减倒计时，归零时切换到下一灯色。
  *         NIGHT模式下翻转黄灯闪烁标志。
  * @param  无。
  * @retval 无。
  */
void Sched_Tick(void);

/**
  * @brief  获取当前灯色状态。
  * @param  无。
  * @retval 当前交通灯状态（夜间模式固定返回STATE_YELLOW）。
  */
TrafficState Sched_GetState(void);

/**
  * @brief  获取当前状态剩余秒数。
  * @param  无。
  * @retval 倒计时秒数（夜间模式返回0）。
  */
uint32_t Sched_GetCountdown(void);

/**
  * @brief  检查夜间模式下黄灯当前是否应点亮。
  * @param  无。
  * @retval 1=点亮，0=熄灭（非夜间模式始终返回0）。
  */
uint8_t Sched_IsNightBlinkOn(void);

#endif
```

- [ ] **Step 2: 提交**

```bash
git add App/scheduler.h
git commit -m "feat: 新增 scheduler 模块头文件"
```

---

### Task 4: 创建 scheduler 模块实现

**Files:**
- Create: `App/scheduler.c`

- [ ] **Step 1: 编写 scheduler.c**

```c
#include "scheduler.h"

/**
  * @brief 每种模式对应的各灯色持续时间（秒）。
  * @note  NIGHT模式的配时由闪烁逻辑单独处理，不查此表。
  */
typedef struct {
	uint32_t green;
	uint32_t yellow;
	uint32_t red;
} TimingConfig;

static const TimingConfig timing_table[] = {
	[MODE_NORMAL] = {GREEN_TIME,  YELLOW_TIME, RED_TIME      },
	[MODE_IDLE]   = {10,          3,           10             },
	[MODE_RUSH]   = {60,          3,           RED_TIME       },
	[MODE_NIGHT]  = {0,           0,           0              },
};

static SchedMode    current_mode;
static TrafficState current_state;
static uint32_t     countdown;
static uint8_t      night_blink_on;

/**
  * @brief  获取当前模式下一个灯色（标准循环：绿→黄→红→绿）。
  * @param  current 当前灯色。
  * @retval 下一个灯色。
  */
static TrafficState next_state(TrafficState current)
{
	switch (current) {
	case STATE_GREEN:  return STATE_YELLOW;
	case STATE_YELLOW: return STATE_RED;
	case STATE_RED:    return STATE_GREEN;
	}
	return STATE_GREEN;
}

void Sched_Init(void)
{
	current_mode    = MODE_NORMAL;
	current_state   = STATE_GREEN;
	countdown       = timing_table[MODE_NORMAL].green;
	night_blink_on  = 0;
}

void Sched_SetMode(SchedMode mode)
{
	current_mode = mode;
	if (mode == MODE_NIGHT) {
		current_state  = STATE_YELLOW;
		countdown      = 0;
		night_blink_on = 0;
	} else {
		current_state  = STATE_GREEN;
		countdown      = timing_table[mode].green;
		night_blink_on = 0;
	}
}

SchedMode Sched_GetMode(void)
{
	return current_mode;
}

const char *Sched_GetModeName(void)
{
	switch (current_mode) {
	case MODE_NORMAL: return "NORMAL";
	case MODE_IDLE:   return "IDLE";
	case MODE_RUSH:   return "RUSH";
	case MODE_NIGHT:  return "NIGHT";
	}
	return "UNKNOWN";
}

void Sched_Tick(void)
{
	if (current_mode == MODE_NIGHT) {
		night_blink_on = !night_blink_on;
		return;
	}

	countdown--;
	if (countdown == 0) {
		current_state = next_state(current_state);
		switch (current_state) {
		case STATE_GREEN:  countdown = timing_table[current_mode].green;  break;
		case STATE_YELLOW: countdown = timing_table[current_mode].yellow; break;
		case STATE_RED:    countdown = timing_table[current_mode].red;    break;
		}
	}
}

TrafficState Sched_GetState(void)
{
	return current_state;
}

uint32_t Sched_GetCountdown(void)
{
	return countdown;
}

uint8_t Sched_IsNightBlinkOn(void)
{
	return (current_mode == MODE_NIGHT) ? night_blink_on : 0;
}
```

- [ ] **Step 2: 提交**

```bash
git add App/scheduler.c
git commit -m "feat: 新增 scheduler 模块实现，支持四种配时模式"
```

---

### Task 5: 创建 command 模块头文件

**Files:**
- Create: `App/command.h`

- [ ] **Step 1: 编写 command.h**

```c
#ifndef __COMMAND_H
#define __COMMAND_H

/**
  * @brief  初始化串口命令系统，打印欢迎信息和命令提示。
  * @param  无。
  * @retval 无。
  */
void Cmd_Init(void);

/**
  * @brief  主循环中每轮调用，非阻塞地处理串口输入。
  * @note   逐字节读取串口环形缓冲区，遇 \\r\\n 时解析并执行命令。
  * @param  无。
  * @retval 无。
  */
void Cmd_Process(void);

#endif
```

- [ ] **Step 2: 提交**

```bash
git add App/command.h
git commit -m "feat: 新增 command 模块头文件"
```

---

### Task 6: 创建 command 模块实现

**Files:**
- Create: `App/command.c`

- [ ] **Step 1: 编写 command.c**

```c
#include "command.h"
#include "Serial.h"
#include "scheduler.h"
#include <string.h>
#include <stdio.h>

/** @brief 命令输入缓冲区大小（字节）。 */
#define CMD_BUF_SIZE 64

static char    cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_index;

/**
  * @brief 命令表条目结构体。
  */
typedef struct {
	const char *name;
	void (*handler)(int argc, char *argv[]);
	const char *help;
} CmdEntry;

/* ── 命令处理函数声明 ── */
static void cmd_mode(int argc, char *argv[]);
static void cmd_status(int argc, char *argv[]);
static void cmd_help(int argc, char *argv[]);

/** @brief 命令注册表。 */
static const CmdEntry cmd_table[] = {
	{"mode",   cmd_mode,   "mode <normal|rush|idle|night> - Switch timing mode"},
	{"status", cmd_status, "status - Show current mode, light state, countdown"},
	{"help",   cmd_help,   "help  - Show available commands"},
};

#define CMD_COUNT (sizeof(cmd_table) / sizeof(cmd_table[0]))

/* ── 命令处理函数实现 ── */

static void cmd_mode(int argc, char *argv[])
{
	if (argc < 1) {
		Serial_Printf(SERIAL_PORT_1,
			"Usage: mode <normal|rush|idle|night>\r\n");
		return;
	}

	if (strcmp(argv[0], "normal") == 0) {
		Sched_SetMode(MODE_NORMAL);
	} else if (strcmp(argv[0], "rush") == 0) {
		Sched_SetMode(MODE_RUSH);
	} else if (strcmp(argv[0], "idle") == 0) {
		Sched_SetMode(MODE_IDLE);
	} else if (strcmp(argv[0], "night") == 0) {
		Sched_SetMode(MODE_NIGHT);
	} else {
		Serial_Printf(SERIAL_PORT_1,
			"Unknown mode: %s. Use: normal rush idle night\r\n", argv[0]);
		return;
	}

	Serial_Printf(SERIAL_PORT_1,
		"[MODE] Switched to %s\r\n", Sched_GetModeName());
}

static void cmd_status(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	const char *state_name;
	switch (Sched_GetState()) {
	case STATE_GREEN:  state_name = "GREEN";  break;
	case STATE_YELLOW: state_name = "YELLOW"; break;
	case STATE_RED:    state_name = "RED";    break;
	default:           state_name = "?";      break;
	}

	if (Sched_GetMode() == MODE_NIGHT) {
		Serial_Printf(SERIAL_PORT_1,
			"[MODE] %s | YELLOW %s\r\n",
			Sched_GetModeName(),
			Sched_IsNightBlinkOn() ? "ON" : "OFF");
	} else {
		Serial_Printf(SERIAL_PORT_1,
			"[MODE] %s | State: %s | Countdown: %lus\r\n",
			Sched_GetModeName(), state_name, Sched_GetCountdown());
	}
}

static void cmd_help(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Serial_Printf(SERIAL_PORT_1, "=== Commands ===\r\n");
	uint8_t i;
	for (i = 0; i < CMD_COUNT; i++) {
		Serial_Printf(SERIAL_PORT_1, "  %s\r\n", cmd_table[i].help);
	}
}

/* ── 公开接口 ── */

void Cmd_Init(void)
{
	cmd_index = 0;
	memset(cmd_buf, 0, sizeof(cmd_buf));

	Serial_Printf(SERIAL_PORT_1,
		"\r\n=== Traffic Light Console ===\r\n");
	Serial_Printf(SERIAL_PORT_1,
		"Type 'help' for available commands\r\n\r\n");
}

/**
  * @brief  解析并执行单条命令。
  * @param  line 以'\\0'结尾的命令字符串。
  * @retval 无。
  */
static void cmd_execute(char *line)
{
	char *argv[4];
	int   argc = 0;

	char *token = strtok(line, " ");
	while (token != NULL && argc < 4) {
		argv[argc++] = token;
		token = strtok(NULL, " ");
	}

	if (argc == 0) return;

	uint8_t i;
	for (i = 0; i < CMD_COUNT; i++) {
		if (strcmp(argv[0], cmd_table[i].name) == 0) {
			cmd_table[i].handler(argc - 1, &argv[1]);
			return;
		}
	}

	Serial_Printf(SERIAL_PORT_1,
		"Unknown command: %s. Type 'help' for help.\r\n", argv[0]);
}

void Cmd_Process(void)
{
	uint8_t ch;

	while (Serial_Receive(SERIAL_PORT_1, &ch, 1) == 1) {
		if (ch == '\r' || ch == '\n') {
			if (cmd_index > 0) {
				cmd_buf[cmd_index] = '\0';
				Serial_Printf(SERIAL_PORT_1, "> %s\r\n", cmd_buf);
				cmd_execute(cmd_buf);
				cmd_index = 0;
				memset(cmd_buf, 0, sizeof(cmd_buf));
			}
		} else if (cmd_index < CMD_BUF_SIZE - 1) {
			cmd_buf[cmd_index++] = (char)ch;
		}
	}
}
```

- [ ] **Step 2: 提交**

```bash
git add App/command.c
git commit -m "feat: 新增 command 模块实现，支持串口命令解析"
```

---

### Task 7: 创建 controller 模块头文件

**Files:**
- Create: `App/controller.h`

- [ ] **Step 1: 编写 controller.h**

```c
#ifndef __CONTROLLER_H
#define __CONTROLLER_H

/**
  * @brief  初始化所有子系统并启动主循环。
  * @note   初始化顺序：LED → OLED → Serial → TIM2 → Scheduler → Command。
  *          随后进入永不返回的主循环。
  * @param  无。
  * @retval 无。
  */
void Ctrl_Init(void);

/**
  * @brief  运行主循环（永不返回）。
  * @note   每轮检查 tim1sFlag，驱动 Sched_Tick，刷新 LED/OLED/串口输出，
  *          并轮询 Cmd_Process 处理串口命令。
  * @param  无。
  * @retval 无。
  */
void Ctrl_Run(void);

#endif
```

- [ ] **Step 2: 提交**

```bash
git add App/controller.h
git commit -m "feat: 新增 controller 模块头文件"
```

---

### Task 8: 创建 controller 模块实现

**Files:**
- Create: `App/controller.c`

- [ ] **Step 1: 编写 controller.c**

```c
#include "controller.h"
#include "LED.h"
#include "OLED.h"
#include "Serial.h"
#include "traffic_light.h"
#include "scheduler.h"
#include "command.h"

/**
  * @brief  根据调度器状态更新LED硬件。
  * @param  无。
  * @retval 无。
  */
static void update_leds(void)
{
	if (Sched_GetMode() == MODE_NIGHT) {
		led_off(LED_RED);
		led_off(LED_GREEN);
		if (Sched_IsNightBlinkOn()) {
			led_on(LED_YELLOW);
		} else {
			led_off(LED_YELLOW);
		}
	} else {
		set_led_by_state(Sched_GetState());
	}
}

/**
  * @brief  通过串口打印当前灯色状态和倒计时。
  * @param  无。
  * @retval 无。
  */
static void print_status(void)
{
	TrafficState state    = Sched_GetState();
	uint32_t     count    = Sched_GetCountdown();
	const char  *mode     = Sched_GetModeName();

	const char *r = (state == STATE_RED)    ? "ON " : "OFF";
	const char *y = (state == STATE_YELLOW) ? "ON " : "OFF";
	const char *g = (state == STATE_GREEN)  ? "ON " : "OFF";

	if (Sched_GetMode() == MODE_NIGHT) {
		Serial_Printf(SERIAL_PORT_1,
			"[%s] Blinking Yellow | R:OFF Y:%s G:OFF\r\n",
			mode, Sched_IsNightBlinkOn() ? "ON " : "OFF");
	} else {
		Serial_Printf(SERIAL_PORT_1,
			"[%s|%s] Countdown: %02lus | R:%s Y:%s G:%s\r\n",
			mode,
			(state == STATE_GREEN)  ? "GREEN"  :
			(state == STATE_YELLOW) ? "YELLOW" : "RED",
			count, r, y, g);
	}
}

/**
  * @brief  刷新OLED显示。
  * @param  无。
  * @retval 无。
  */
static void update_oled(void)
{
	if (Sched_GetMode() == MODE_NIGHT) {
		OLED_ShowCountdown(0);
	} else {
		OLED_ShowCountdown((uint8_t)Sched_GetCountdown());
	}
}

void Ctrl_Init(void)
{
	LED_Init();
	OLED_Init();
	Serial_Init();
	TIM2_Init();
	Sched_Init();
	Cmd_Init();
}

void Ctrl_Run(void)
{
	while (1) {
		if (tim1sFlag) {
			tim1sFlag = 0;
			Sched_Tick();
			update_leds();
			print_status();
			update_oled();
		}
		Cmd_Process();
	}
}
```

- [ ] **Step 2: 提交**

```bash
git add App/controller.c
git commit -m "feat: 新增 controller 模块实现，统筹调度所有子系统"
```

---

### Task 9: 更新 main.c

**Files:**
- Modify: `User/main.c`

- [ ] **Step 1: 修改 main.c**

将调用入口从 `TrafficLight_Init/Run` 改为 `Ctrl_Init/Run`：

```c
#include "stm32f10x.h"
#include "controller.h"

int main(void)
{
	Ctrl_Init();
	Ctrl_Run();
}
```

- [ ] **Step 2: 检查改动范围**

```bash
git diff --stat
```
预期输出：3 个文件修改（traffic_light.h/c, main.c），6 个文件新建（scheduler.h/c, command.h/c, controller.h/c）。

- [ ] **Step 3: 确认不再有对 TrafficLight_Init/TrafficLight_Run 的引用**

```bash
grep -rn "TrafficLight_Init\|TrafficLight_Run" App/ User/
```
预期输出：无匹配结果。

- [ ] **Step 4: 提交**

```bash
git add User/main.c
git commit -m "feat: main.c 切换到 controller 统筹调度入口"
```

---

### Task 10: 功能验证清单

> 这些步骤需要将代码烧录到 STM32 开发板，通过串口助手（9600-8-N-1）验证。

- [ ] **Step 1: 启动验证**

上电后串口助手应显示：
```
=== Traffic Light Console ===
Type 'help' for available commands

> [NORMAL|GREEN] Countdown: 30s | R:OFF Y:OFF G:ON
```
LED 绿灯亮，OLED 显示倒计时。

- [ ] **Step 2: help 命令**

输入 `help`，预期列出 mode / status / help 三条命令。

- [ ] **Step 3: mode 命令**

依次输入 `mode rush`、`mode idle`、`mode night`：
- rush: 绿灯 60s 周期
- idle: 绿灯 10s 周期
- night: 黄灯每秒亮灭交替，OLED 显示 "00"

- [ ] **Step 4: status 命令**

正常模式下输入 `status`，返回当前模式、灯色和倒计时。

- [ ] **Step 5: mode normal 恢复**

输入 `mode normal`，恢复到 30s/3s/30s 默认周期。

- [ ] **Step 6: 错误处理**

输入 `mode xxx`（错误参数），提示 "Unknown mode"。
输入 `foo`，提示 "Unknown command"。

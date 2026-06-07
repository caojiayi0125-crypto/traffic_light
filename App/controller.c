/**
  * @file    controller.c
  * @brief   顶层统筹调度模块实现。
  * @note    负责初始化所有子系统（LED / OLED / 串口 / 定时器 / 调度器 / 命令），
  *          并在主循环中协调定时器走秒、硬件刷新和命令处理。
  */

#include "controller.h"
#include "LED.h"
#include "OLED.h"
#include "Serial.h"
#include "traffic_light.h"
#include "scheduler.h"
#include "command.h"

/**
  * @brief  根据调度器状态更新LED硬件。
  * @note   夜间模式：红绿灯全灭，黄灯按闪烁标志亮灭。
  *         其他模式：调用 set_led_by_state 映射灯色。
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
  * @note   格式示例：
  *         - [NORMAL|GREEN] Countdown: 25s | R:OFF Y:OFF G:ON
  *         - [NIGHT] Blinking Yellow | R:OFF Y:OFF G:OFF
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
  * @brief  刷新OLED显示倒计时。
  * @note   夜间模式显示 "00"，其他模式显示当前倒计时数值。
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

/**
  * @brief  按顺序初始化所有子系统。
  * @note   顺序：LED → OLED → Serial → TIM2 → Scheduler → Command。
  *         串口必须先于命令系统初始化，否则欢迎信息无法输出。
  * @param  无。
  * @retval 无。
  */
void Ctrl_Init(void)
{
	LED_Init();
	OLED_Init();
	Serial_Init();
	TIM2_Init();
	Sched_Init();
	Cmd_Init();
}

/**
  * @brief  永不返回的主循环。
  * @note   tim1sFlag 由 TIM2 中断置位，主循环检测后清零并驱动一整套刷新。
  *          Cmd_Process 不受 tim1sFlag 门控，保证串口交互低延迟。
  * @param  无。
  * @retval 无。
  */
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

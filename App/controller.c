#include "controller.h"
#include "LED.h"
#include "OLED.h"
#include "Serial.h"
#include "traffic_light.h"
#include "scheduler.h"
#include "command.h"

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

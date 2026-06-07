#include "scheduler.h"

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

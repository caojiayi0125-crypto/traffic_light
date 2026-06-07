/**
  * @file    scheduler.c
  * @brief   动态配时调度器实现。
  * @note    提供 4 种配时模式：NORMAL / IDLE / RUSH / NIGHT。
  *          通过 Sched_Tick() 每秒推进状态机，夜间模式触发黄灯闪烁。
  */

#include "scheduler.h"

/**
  * @brief 每种模式对应的各灯色持续时间。
  */
typedef struct {
	uint32_t green;   /**< 绿灯持续时间（秒）。 */
	uint32_t yellow;  /**< 黄灯持续时间（秒）。 */
	uint32_t red;     /**< 红灯持续时间（秒）。 */
} TimingConfig;

/** @brief 配时配置表，按 SchedMode 索引。 */
static const TimingConfig timing_table[] = {
	[MODE_NORMAL] = {GREEN_TIME,  YELLOW_TIME, RED_TIME      },
	[MODE_IDLE]   = {10,          3,           10             },
	[MODE_RUSH]   = {60,          3,           RED_TIME       },
	[MODE_NIGHT]  = {0,           0,           0              },
};

/** @brief 当前配时模式。 */
static SchedMode    current_mode;
/** @brief 当前灯色状态。 */
static TrafficState current_state;
/** @brief 当前状态倒计时（秒）。 */
static uint32_t     countdown;
/** @brief 夜间模式黄灯闪烁标志：1 = 亮，0 = 灭。 */
static uint8_t      night_blink_on;

/**
  * @brief  获取标准循环中当前灯色的下一个灯色。
  * @param  current 当前灯色。
  * @retval 下一个灯色（绿→黄→红→绿）。
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

/**
  * @brief  初始化调度器。
  * @note   默认 NORMAL 模式，从绿灯开始。
  * @param  无。
  * @retval 无。
  */
void Sched_Init(void)
{
	current_mode    = MODE_NORMAL;
	current_state   = STATE_GREEN;
	countdown       = timing_table[MODE_NORMAL].green;
	night_blink_on  = 0;
}

/**
  * @brief  切换配时模式。
  * @note   夜间模式：固定黄灯、初始灭、countdown归零。
  *         非夜间模式：重置到绿灯、加载新配时的绿灯时长。
  * @param  mode 目标配时模式。
  * @retval 无。
  */
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

/**
  * @brief  获取当前模式名称字符串。
  * @param  无。
  * @retval 模式名称（大写英文）。
  */
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

/**
  * @brief  每秒推进状态机。
  * @note   夜间模式：翻转闪烁标志后直接返回。
  *         非夜间模式：递减倒计时，归零时切换灯色并加载下一段的配时。
  * @param  无。
  * @retval 无。
  */
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

/**
  * @brief  查询夜间模式下黄灯是否应点亮。
  * @note   仅在 MODE_NIGHT 下有效，非夜间模式固定返回 0。
  * @param  无。
  * @retval 1 = 点亮，0 = 熄灭。
  */
uint8_t Sched_IsNightBlinkOn(void)
{
	return (current_mode == MODE_NIGHT) ? night_blink_on : 0;
}

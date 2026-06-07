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
  * @brief  初始化调度器，默认进入NORMAL模式，从绿灯开始。
  * @param  无。
  * @retval 无。
  */
void Sched_Init(void);

/**
  * @brief  切换配时模式。
  * @note   非夜间模式下重置状态机到绿灯并加载对应配时；
  *         夜间模式下固定黄灯、启动闪烁。
  * @param  mode 目标配时模式。
  * @retval 无。
  */
void Sched_SetMode(SchedMode mode);

/**
  * @brief  获取当前配时模式。
  * @param  无。
  * @retval 当前配时模式枚举值。
  */
SchedMode Sched_GetMode(void);

/**
  * @brief  获取当前模式名称字符串，供串口和OLED显示。
  * @param  无。
  * @retval 模式名称（大写英文，如 "NORMAL"）。
  */
const char *Sched_GetModeName(void);

/**
  * @brief  每秒调用一次，推进状态机。
  * @note   非夜间模式：递减倒计时，归零时切换到下一灯色。
  *         夜间模式：翻转黄灯闪烁标志。
  * @param  无。
  * @retval 无。
  */
void Sched_Tick(void);

/**
  * @brief  获取当前灯色状态。
  * @param  无。
  * @retval 当前交通灯状态（夜间模式固定返回 STATE_YELLOW）。
  */
TrafficState Sched_GetState(void);

/**
  * @brief  获取当前状态剩余秒数。
  * @param  无。
  * @retval 倒计时数值（夜间模式返回0）。
  */
uint32_t Sched_GetCountdown(void);

/**
  * @brief  判断夜间模式下黄灯当前是否应点亮。
  * @note   非夜间模式始终返回0。
  * @param  无。
  * @retval 1 = 点亮，0 = 熄灭。
  */
uint8_t Sched_IsNightBlinkOn(void);

#endif

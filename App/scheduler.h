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

void Sched_Init(void);
void Sched_SetMode(SchedMode mode);
SchedMode Sched_GetMode(void);
const char *Sched_GetModeName(void);
void Sched_Tick(void);
TrafficState Sched_GetState(void);
uint32_t Sched_GetCountdown(void);
uint8_t Sched_IsNightBlinkOn(void);

#endif

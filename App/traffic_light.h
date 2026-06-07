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

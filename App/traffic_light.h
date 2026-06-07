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
  * @brief  初始化交通信号灯系统。
  * @note   配置TIM2产生1秒时基信号、熄灭所有LED并清除OLED屏幕。
  * @param  无。
  * @retval 无。
  */
void TrafficLight_Init(void);

/**
  * @brief  运行交通信号灯主循环（永不返回）。
  * @note   按照 绿灯(30s)→黄灯(3s)→红灯(30s) 的顺序循环切换。
  *         由TIM2的1秒时基信号驱动，每秒更新LED、串口日志和OLED倒计时显示。
  * @param  无。
  * @retval 无。
  */
void TrafficLight_Run(void);

#endif

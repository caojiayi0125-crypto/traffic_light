#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

/** @brief 红色LED GPIO端口。 */
#define LED_RED_GPIO_PORT     GPIOA
/** @brief 红色LED GPIO引脚。 */
#define LED_RED_GPIO_PIN      GPIO_Pin_4
/** @brief 黄色LED GPIO端口。 */
#define LED_YELLOW_GPIO_PORT  GPIOA
/** @brief 黄色LED GPIO引脚。 */
#define LED_YELLOW_GPIO_PIN   GPIO_Pin_5
/** @brief 绿色LED GPIO端口。 */
#define LED_GREEN_GPIO_PORT   GPIOA
/** @brief 绿色LED GPIO引脚。 */
#define LED_GREEN_GPIO_PIN    GPIO_Pin_6

/**
  * @brief LED 标识符枚举。
  */
typedef enum
{
	LED_RED = 0,  /**< 红色LED。 */
	LED_YELLOW,   /**< 黄色LED。 */
	LED_GREEN     /**< 绿色LED。 */
} LED_TypeDef;

/**
  * @brief  初始化所有LED的GPIO引脚。
  * @param  无。
  * @retval 无。
  */
void LED_Init(void);

/**
  * @brief  点亮指定LED。
  * @param  led LED标识符。
  * @retval 无。
  */
void led_on(LED_TypeDef led);

/**
  * @brief  熄灭指定LED。
  * @param  led LED标识符。
  * @retval 无。
  */
void led_off(LED_TypeDef led);

/**
  * @brief  翻转指定LED的亮灭状态。
  * @param  led LED标识符。
  * @retval 无。
  */
void led_turn(LED_TypeDef led);

#endif

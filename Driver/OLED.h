#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

/** @brief OLED的GPIO时钟。 */
#define OLED_GPIO_CLOCK     RCC_APB2Periph_GPIOB
/** @brief OLED的GPIO端口。 */
#define OLED_GPIO_PORT      GPIOB
/** @brief OLED的SCL引脚。 */
#define OLED_SCL_GPIO_PIN   GPIO_Pin_8
/** @brief OLED的SDA引脚。 */
#define OLED_SDA_GPIO_PIN   GPIO_Pin_9

/**
  * @brief  初始化OLED的GPIO引脚及SSD1306控制器。
  * @param  无。
  * @retval 无。
  */
void OLED_Init(void);

/**
  * @brief  清除整个OLED屏幕。
  * @param  无。
  * @retval 无。
  */
void OLED_Clear(void);

/**
  * @brief  在OLED屏幕上显示两位倒计时数字。
  * @param  value 倒计时数值，超过99则显示为99。
  * @retval 无。
  */
void OLED_ShowCountdown(uint8_t value);

#endif

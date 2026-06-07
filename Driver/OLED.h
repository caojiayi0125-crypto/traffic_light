#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

#define OLED_GPIO_CLOCK     RCC_APB2Periph_GPIOB
#define OLED_GPIO_PORT      GPIOB
#define OLED_SCL_GPIO_PIN   GPIO_Pin_8
#define OLED_SDA_GPIO_PIN   GPIO_Pin_9

/**
  * @brief  Initialize OLED GPIO and controller.
  * @param  None.
  * @retval None.
  */
void OLED_Init(void);

/**
  * @brief  Clear the whole OLED screen.
  * @param  None.
  * @retval None.
  */
void OLED_Clear(void);

/**
  * @brief  Show a two-digit countdown number on the full OLED screen.
  * @param  value Countdown value. Values greater than 99 are displayed as 99.
  * @retval None.
  */
void OLED_ShowCountdown(uint8_t value);

#endif

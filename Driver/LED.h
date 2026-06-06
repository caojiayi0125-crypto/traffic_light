#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

/** @brief Red LED GPIO port. */
#define LED_RED_GPIO_PORT     GPIOA
/** @brief Red LED GPIO pin. */
#define LED_RED_GPIO_PIN      GPIO_Pin_4
/** @brief Yellow LED GPIO port. */
#define LED_YELLOW_GPIO_PORT  GPIOA
/** @brief Yellow LED GPIO pin. */
#define LED_YELLOW_GPIO_PIN   GPIO_Pin_5
/** @brief Green LED GPIO port. */
#define LED_GREEN_GPIO_PORT   GPIOA
/** @brief Green LED GPIO pin. */
#define LED_GREEN_GPIO_PIN    GPIO_Pin_6

/**
  * @brief LED identifier.
  */
typedef enum
{
	LED_RED = 0,  /**< Red LED. */
	LED_YELLOW,   /**< Yellow LED. */
	LED_GREEN     /**< Green LED. */
} LED_TypeDef;

/**
  * @brief  Initialize all LED GPIO pins.
  * @param  None.
  * @retval None.
  */
void LED_Init(void);

/**
  * @brief  Turn on the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_on(LED_TypeDef led);

/**
  * @brief  Turn off the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_off(LED_TypeDef led);

/**
  * @brief  Toggle the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_turn(LED_TypeDef led);

#endif

#include "LED.h"

/**
  * @brief  Get the GPIO port for the specified LED.
  * @param  led LED identifier.
  * @retval GPIO port pointer. Returns 0 for an invalid LED value.
  */
static GPIO_TypeDef *LED_GetPort(LED_TypeDef led)
{
	switch (led)
	{
	case LED_RED:    return LED_RED_GPIO_PORT;
	case LED_YELLOW: return LED_YELLOW_GPIO_PORT;
	case LED_GREEN:  return LED_GREEN_GPIO_PORT;
	default:         return 0;
	}
}

/**
  * @brief  Get the GPIO pin for the specified LED.
  * @param  led LED identifier.
  * @retval GPIO pin mask. Returns 0 for an invalid LED value.
  */
static uint16_t LED_GetPin(LED_TypeDef led)
{
	switch (led)
	{
	case LED_RED:    return LED_RED_GPIO_PIN;
	case LED_YELLOW: return LED_YELLOW_GPIO_PIN;
	case LED_GREEN:  return LED_GREEN_GPIO_PIN;
	default:         return 0;
	}
}

/**
  * @brief  Initialize all traffic-light LED GPIO pins.
  * @param  None.
  * @retval None.
  * @note   LEDs are active-low, so this function turns all LEDs off after init.
  */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = LED_RED_GPIO_PIN | LED_YELLOW_GPIO_PIN | LED_GREEN_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	led_off(LED_RED);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}

/**
  * @brief  Turn on the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_on(LED_TypeDef led)
{
	GPIO_TypeDef *port = LED_GetPort(led);
	uint16_t pin = LED_GetPin(led);

	if (port != 0 && pin != 0)
	{
		GPIO_ResetBits(port, pin);
	}
}

/**
  * @brief  Turn off the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_off(LED_TypeDef led)
{
	GPIO_TypeDef *port = LED_GetPort(led);
	uint16_t pin = LED_GetPin(led);

	if (port != 0 && pin != 0)
	{
		GPIO_SetBits(port, pin);
	}
}

/**
  * @brief  Toggle the specified LED.
  * @param  led LED identifier.
  * @retval None.
  */
void led_turn(LED_TypeDef led)
{
	GPIO_TypeDef *port = LED_GetPort(led);
	uint16_t pin = LED_GetPin(led);

	if (port == 0 || pin == 0)
	{
		return;
	}

	if (GPIO_ReadOutputDataBit(port, pin) == Bit_RESET)
	{
		GPIO_SetBits(port, pin);
	}
	else
	{
		GPIO_ResetBits(port, pin);
	}
}

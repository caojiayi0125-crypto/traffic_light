#include "LED.h"

/**
  * @brief  获取指定LED对应的GPIO端口。
  * @param  led LED标识符。
  * @retval GPIO端口指针，无效LED时返回0。
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
  * @brief  获取指定LED对应的GPIO引脚。
  * @param  led LED标识符。
  * @retval GPIO引脚掩码，无效LED时返回0。
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
  * @brief  初始化所有交通灯LED的GPIO引脚。
  * @param  无。
  * @retval 无。
  * @note   LED为低电平点亮，初始化完成后所有LED处于熄灭状态。
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
  * @brief  点亮指定LED。
  * @param  led LED标识符。
  * @retval 无。
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
  * @brief  熄灭指定LED。
  * @param  led LED标识符。
  * @retval 无。
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
  * @brief  翻转指定LED的亮灭状态。
  * @param  led LED标识符。
  * @retval 无。
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

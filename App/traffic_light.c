#include "traffic_light.h"
#include "LED.h"

/** @brief TIM2的预分频值（72MHz / 7200 = 10kHz）。 */
#define TIM2_PRESCALER    7199
/** @brief TIM2的自动重装载值（10kHz / 10000 = 1Hz）。 */
#define TIM2_PERIOD       9999

/** @brief 1秒时基标志（TIM2中断置1，主循环清零）。 */
volatile uint8_t tim1sFlag;

/**
  * @brief  配置TIM2产生1秒时基中断。
  * @param  无。
  * @retval 无。
  */
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = TIM2_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = TIM2_PRESCALER;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  熄灭所有LED。
  * @param  无。
  * @retval 无。
  */
void led_all_off(void)
{
	led_off(LED_RED);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}

/**
  * @brief  根据当前状态点亮对应的LED。
  * @param  state 当前交通灯状态。
  * @retval 无。
  */
void set_led_by_state(TrafficState state)
{
	led_all_off();
	switch (state) {
	case STATE_GREEN:  led_on(LED_GREEN);  break;
	case STATE_YELLOW: led_on(LED_YELLOW); break;
	case STATE_RED:    led_on(LED_RED);    break;
	}
}

/**
  * @brief  TIM2中断服务函数。
  * @param  无。
  * @retval 无。
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		tim1sFlag = 1;
	}
}

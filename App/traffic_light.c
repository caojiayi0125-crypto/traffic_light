#include "traffic_light.h"
#include "LED.h"
#include "Serial.h"
#include "OLED.h"

/** @brief TIM2的预分频值（72MHz / 7200 = 10kHz）。 */
#define TIM2_PRESCALER    7199
/** @brief TIM2的自动重装载值（10kHz / 10000 = 1Hz）。 */
#define TIM2_PERIOD       9999

/** @brief 1秒时基标志（TIM2中断置1，主循环清零）。 */
static volatile uint8_t tim1sFlag;

/**
  * @brief 交通信号灯状态枚举。
  */
typedef enum {
	STATE_GREEN,   /**< 绿灯状态。 */
	STATE_YELLOW,  /**< 黄灯状态。 */
	STATE_RED      /**< 红灯状态。 */
} TrafficState;

/**
  * @brief  配置TIM2产生1秒时基中断。
  * @note   TIM2挂载在APB1总线上，时钟为72MHz。
  *         预分频7200、自动重装载10000，产生1Hz更新中断。
  * @param  无。
  * @retval 无。
  */
static void TIM2_Init(void)
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
static void led_all_off(void)
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
static void set_led_by_state(TrafficState state)
{
	led_all_off();
	switch (state) {
	case STATE_GREEN:  led_on(LED_GREEN);  break;
	case STATE_YELLOW: led_on(LED_YELLOW); break;
	case STATE_RED:    led_on(LED_RED);    break;
	}
}

/**
  * @brief  获取指定状态的持续时间。
  * @param  state 交通灯状态。
  * @retval 持续时间（秒）。
  */
static uint32_t state_duration(TrafficState state)
{
	switch (state) {
	case STATE_GREEN:  return GREEN_TIME;
	case STATE_YELLOW: return YELLOW_TIME;
	case STATE_RED:    return RED_TIME;
	}
	return 0;
}

/**
  * @brief  获取指定状态的名称字符串。
  * @param  state 交通灯状态。
  * @retval 状态名称（大写英文）。
  */
static const char *state_name(TrafficState state)
{
	switch (state) {
	case STATE_GREEN:  return "GREEN";
	case STATE_YELLOW: return "YELLOW";
	case STATE_RED:    return "RED";
	}
	return "UNKNOWN";
}

/**
  * @brief  通过串口打印当前灯色状态和倒计时。
  * @param  state 当前交通灯状态。
  * @param  countdown 剩余秒数。
  * @retval 无。
  */
static void print_status(TrafficState state, uint32_t countdown)
{
	const char *r = (state == STATE_RED)    ? "ON " : "OFF";
	const char *y = (state == STATE_YELLOW) ? "ON " : "OFF";
	const char *g = (state == STATE_GREEN)  ? "ON " : "OFF";

	Serial_Printf(SERIAL_PORT_1,
		"[%s] Countdown: %02lus | R:%s Y:%s G:%s\r\n",
		state_name(state), countdown, r, y, g);
}

/**
  * @brief  获取当前状态的下一个状态。
  * @param  current 当前交通灯状态。
  * @retval 下一个状态。
  */
static TrafficState next_state(TrafficState current)
{
	switch (current) {
	case STATE_GREEN:  return STATE_YELLOW;
	case STATE_YELLOW: return STATE_RED;
	case STATE_RED:    return STATE_GREEN;
	}
	return STATE_GREEN;
}

/**
  * @brief  初始化交通信号灯系统。
  * @note   启动TIM2产生1秒时基、通过串口打印启动信息、熄灭所有LED并清除OLED屏幕。
  * @param  无。
  * @retval 无。
  */
void TrafficLight_Init(void)
{
	TIM2_Init();

	Serial_Printf(SERIAL_PORT_1,
		"\r\n=== Traffic Light System Boot ===\r\n");
	Serial_Printf(SERIAL_PORT_1,
		"Cycle: GREEN(%ds) -> YELLOW(%ds) -> RED(%ds)\r\n\r\n",
		GREEN_TIME, YELLOW_TIME, RED_TIME);

	led_all_off();
	OLED_Clear();
}

/**
  * @brief  运行交通信号灯主循环（永不返回）。
  * @note   轮询TIM2的1秒时基标志来驱动状态切换。
  *         每秒更新LED、串口日志和OLED倒计时显示。
  * @param  无。
  * @retval 无。
  */
void TrafficLight_Run(void)
{
	TrafficState state = STATE_GREEN;
	uint32_t countdown = GREEN_TIME;

	set_led_by_state(state);
	print_status(state, countdown);
	OLED_ShowCountdown((uint8_t)countdown);

	while (1) {
		if (tim1sFlag) {
			tim1sFlag = 0;
			countdown--;

			if (countdown == 0) {
				state = next_state(state);
				countdown = state_duration(state);
				set_led_by_state(state);
			}

			print_status(state, countdown);
			OLED_ShowCountdown((uint8_t)countdown);
		}
	}
}

/**
  * @brief  TIM2中断服务函数。
  * @note   每次更新中断置位1秒时基标志。
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

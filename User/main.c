#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"
#include "Key.h"
#include <stdio.h>
#include <string.h>

#define GREEN_TIME   30
#define YELLOW_TIME  3
#define RED_TIME     30

typedef enum {
	STATE_GREEN = 0,
	STATE_YELLOW,
	STATE_RED
} TrafficState;

const uint8_t StateTime[] = {GREEN_TIME, YELLOW_TIME, RED_TIME};
const char *StateName[] = {"GREEN", "YELLOW", "RED"};

volatile uint16_t timTick = 0;

static TrafficState currentState = STATE_GREEN;
static int8_t countdown = GREEN_TIME;
static uint8_t nightMode = 0;
static uint8_t keyHeldTicks = 0;
static uint8_t keyTriggered = 0;

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		timTick++;
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

static void SetTrafficLight(TrafficState s)
{
	TrafficLight_AllOff();
	switch (s)
	{
	case STATE_GREEN:  TrafficLight_Green();  break;
	case STATE_YELLOW: TrafficLight_Yellow(); break;
	case STATE_RED:    TrafficLight_Red();    break;
	}
}

static void NextState(void)
{
	currentState = (currentState + 1) % 3;
	countdown = StateTime[currentState];
	SetTrafficLight(currentState);
}

static void PrintStatus(void)
{
	if (nightMode)
	{
		Serial_Printf("[NIGHT] Yellow Blinking | R:OFF Y:BLINK G:OFF\r\n");
	}
	else
	{
		Serial_Printf("[%s] Countdown: %02ds | R:%s Y:%s G:%s\r\n",
			StateName[currentState],
			countdown,
			currentState == STATE_RED    ? "ON " : "OFF",
			currentState == STATE_YELLOW ? "ON " : "OFF",
			currentState == STATE_GREEN  ? "ON"  : "OFF");
	}
}

int main(void)
{
	OLED_Init();
	LED_Init();
	TrafficLight_Init();
	Key_Init();
	Serial_Init();

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 5000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	SetTrafficLight(STATE_GREEN);
	OLED_ShowString(1, 1, "Traffic Light");

	uint16_t lastTick = 0;
	uint8_t  halfSec = 0;
	uint8_t  blinkState = 0;
	uint8_t  printHalf = 0;

	while (1)
	{
		uint8_t keyPressed = (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);

		if (timTick != lastTick)
		{
			lastTick = timTick;

			if (keyPressed)
			{
				keyHeldTicks++;
				if (keyHeldTicks >= 2 && !keyTriggered)
				{
					keyTriggered = 1;
					nightMode = !nightMode;
					if (nightMode)
					{
						TrafficLight_AllOff();
						OLED_Clear();
						OLED_ShowString(1, 1, "Traffic Light");
					}
					else
					{
						currentState = STATE_GREEN;
						countdown = GREEN_TIME;
						SetTrafficLight(STATE_GREEN);
					}
				}
			}
			else
			{
				if (keyHeldTicks == 1 && nightMode)
				{
					nightMode = 0;
					currentState = STATE_GREEN;
					countdown = GREEN_TIME;
					SetTrafficLight(STATE_GREEN);
				}
				keyHeldTicks = 0;
				keyTriggered = 0;
			}

			if (nightMode)
			{
				blinkState = !blinkState;
				if (blinkState)
					TrafficLight_Yellow();
				else
					TrafficLight_AllOff();
			}
			else
			{
				halfSec++;
				if (halfSec >= 2)
				{
					halfSec = 0;
					countdown--;
					if (countdown <= 0)
						NextState();
				}
			}

			printHalf++;
			if (printHalf >= 2)
			{
				printHalf = 0;
				PrintStatus();
			}
		}

		if (nightMode)
		{
			OLED_ShowString(3, 1, "NIGHT MODE     ");
			OLED_ShowString(4, 1, "Yellow Blink   ");
		}
		else
		{
			OLED_ShowString(3, 1, "State:        ");
			OLED_ShowString(3, 7, (char *)StateName[currentState]);
			OLED_ShowString(4, 1, "Countdown:   s");
			OLED_ShowNum(4, 11, countdown, 2);
		}
	}
}

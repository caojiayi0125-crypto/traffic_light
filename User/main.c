#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"
#include "Key.h"

int main(void)
{
	OLED_Init();
	LED_Init();
	Key_Init();
	Serial_Init();

	while (1)
	{
	}
}

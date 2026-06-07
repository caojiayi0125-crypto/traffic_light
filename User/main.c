#include "stm32f10x.h"
#include "LED.h"
#include "Serial.h"
#include "OLED.h"
#include "traffic_light.h"

int main(void)
{
	LED_Init();
	Serial_Init();
	OLED_Init();
	TrafficLight_Init();
	TrafficLight_Run();
}

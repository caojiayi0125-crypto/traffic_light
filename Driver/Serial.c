#include "Serial.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define SERIAL_USE_USART1      SERIAL_PORT1_ENABLED
#define SERIAL_USE_USART2      SERIAL_PORT2_ENABLED

#define SERIAL1_USART          USART1
#define SERIAL1_USART_CLOCK    RCC_APB2Periph_USART1
#define SERIAL1_IRQ            USART1_IRQn
#define SERIAL1_GPIO_CLOCK     RCC_APB2Periph_GPIOA
#define SERIAL1_GPIO_PORT      GPIOA
#define SERIAL1_TX_GPIO_PIN    GPIO_Pin_9
#define SERIAL1_RX_GPIO_PIN    GPIO_Pin_10

#define SERIAL2_USART          USART2
#define SERIAL2_USART_CLOCK    RCC_APB1Periph_USART2
#define SERIAL2_IRQ            USART2_IRQn
#define SERIAL2_GPIO_CLOCK     RCC_APB2Periph_GPIOA
#define SERIAL2_GPIO_PORT      GPIOA
#define SERIAL2_TX_GPIO_PIN    GPIO_Pin_2
#define SERIAL2_RX_GPIO_PIN    GPIO_Pin_3

typedef struct
{
	volatile uint8_t *buffer;
	uint16_t size;
	volatile uint16_t writeIndex;
	volatile uint16_t readIndex;
} Serial_RxBufferTypeDef;

#if SERIAL_USE_USART1
static volatile uint8_t Serial1_Buffer[SERIAL1_BUFFER_SIZE];
static Serial_RxBufferTypeDef Serial1_RxBuffer;
#endif

#if SERIAL_USE_USART2
static volatile uint8_t Serial2_Buffer[SERIAL2_BUFFER_SIZE];
static Serial_RxBufferTypeDef Serial2_RxBuffer;
#endif

/**
  * @brief  Get USART peripheral pointer by serial identifier.
  * @param  serial Serial hardware identifier.
  * @retval USART peripheral pointer. Returns 0 when the port is disabled or invalid.
  */
static USART_TypeDef *Serial_GetUsart(Serial_TypeDef serial)
{
	switch (serial)
	{
#if SERIAL_USE_USART1
	case SERIAL_PORT_1: return SERIAL1_USART;
#endif
#if SERIAL_USE_USART2
	case SERIAL_PORT_2: return SERIAL2_USART;
#endif
	default:            return 0;
	}
}

/**
  * @brief  Get RX buffer by serial identifier.
  * @param  serial Serial hardware identifier.
  * @retval RX buffer pointer. Returns 0 when the port is disabled or invalid.
  */
static Serial_RxBufferTypeDef *Serial_GetRxBuffer(Serial_TypeDef serial)
{
	switch (serial)
	{
#if SERIAL_USE_USART1
	case SERIAL_PORT_1: return &Serial1_RxBuffer;
#endif
#if SERIAL_USE_USART2
	case SERIAL_PORT_2: return &Serial2_RxBuffer;
#endif
	default:            return 0;
	}
}

/**
  * @brief  Store one received byte into the selected RX buffer.
  * @param  rxBuffer RX buffer pointer.
  * @param  data Received byte.
  * @retval None.
  * @note   When the buffer is full, the oldest unread byte is overwritten.
  */
static void Serial_RxBufferPush(Serial_RxBufferTypeDef *rxBuffer, uint8_t data)
{
	uint16_t nextIndex;

	if (rxBuffer == 0)
	{
		return;
	}

	nextIndex = (rxBuffer->writeIndex + 1) % rxBuffer->size;
	if (nextIndex == rxBuffer->readIndex)
	{
		rxBuffer->readIndex = (rxBuffer->readIndex + 1) % rxBuffer->size;
	}

	rxBuffer->buffer[rxBuffer->writeIndex] = data;
	rxBuffer->writeIndex = nextIndex;
}

/**
  * @brief  Configure GPIO pins for USART1.
  * @param  None.
  * @retval None.
  */
#if SERIAL_USE_USART1
static void Serial1_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(SERIAL1_GPIO_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SERIAL1_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SERIAL1_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SERIAL1_RX_GPIO_PIN;
	GPIO_Init(SERIAL1_GPIO_PORT, &GPIO_InitStructure);
}
#endif

/**
  * @brief  Configure GPIO pins for USART2.
  * @param  None.
  * @retval None.
  */
#if SERIAL_USE_USART2
static void Serial2_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(SERIAL2_GPIO_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SERIAL2_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SERIAL2_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SERIAL2_RX_GPIO_PIN;
	GPIO_Init(SERIAL2_GPIO_PORT, &GPIO_InitStructure);
}
#endif

/**
  * @brief  Apply common USART configuration.
  * @param  usart USART peripheral pointer.
  * @retval None.
  */
static void Serial_UsartInit(USART_TypeDef *usart, uint32_t baudRate, uint16_t wordLength, uint16_t stopBits, uint16_t parity, uint16_t flowControl, uint16_t mode)
{
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = baudRate;
	USART_InitStructure.USART_HardwareFlowControl = flowControl;
	USART_InitStructure.USART_Mode = mode;
	USART_InitStructure.USART_Parity = parity;
	USART_InitStructure.USART_StopBits = stopBits;
	USART_InitStructure.USART_WordLength = wordLength;
	USART_Init(usart, &USART_InitStructure);

	USART_ITConfig(usart, USART_IT_RXNE, ENABLE);
	USART_Cmd(usart, ENABLE);
}

/**
  * @brief  Configure NVIC for selected USART interrupt.
  * @param  irqChannel IRQ channel number.
  * @retval None.
  */
static void Serial_NVICInit(uint8_t irqChannel)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = irqChannel;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Initialize one serial port.
  * @param  serial Serial hardware identifier.
  * @retval None.
  */
static void Serial_InitPort(Serial_TypeDef serial)
{
	Serial_RxBufferTypeDef *rxBuffer = Serial_GetRxBuffer(serial);

	if (rxBuffer == 0)
	{
		return;
	}

	rxBuffer->writeIndex = 0;
	rxBuffer->readIndex = 0;

	switch (serial)
	{
#if SERIAL_USE_USART1
	case SERIAL_PORT_1:
		rxBuffer->buffer = Serial1_Buffer;
		rxBuffer->size = SERIAL1_BUFFER_SIZE;
		RCC_APB2PeriphClockCmd(SERIAL1_USART_CLOCK, ENABLE);
		Serial1_GPIOInit();
		Serial_UsartInit(SERIAL1_USART, SERIAL1_BAUD_RATE, SERIAL1_WORD_LENGTH, SERIAL1_STOP_BITS, SERIAL1_PARITY, SERIAL1_FLOW_CONTROL, SERIAL1_MODE);
		Serial_NVICInit(SERIAL1_IRQ);
		break;
#endif

#if SERIAL_USE_USART2
	case SERIAL_PORT_2:
		rxBuffer->buffer = Serial2_Buffer;
		rxBuffer->size = SERIAL2_BUFFER_SIZE;
		RCC_APB1PeriphClockCmd(SERIAL2_USART_CLOCK, ENABLE);
		Serial2_GPIOInit();
		Serial_UsartInit(SERIAL2_USART, SERIAL2_BAUD_RATE, SERIAL2_WORD_LENGTH, SERIAL2_STOP_BITS, SERIAL2_PARITY, SERIAL2_FLOW_CONTROL, SERIAL2_MODE);
		Serial_NVICInit(SERIAL2_IRQ);
		break;
#endif

	default:
		break;
	}
}

/**
  * @brief  Initialize enabled serial ports according to SERIAL_USE_MODE.
  * @param  None.
  * @retval None.
  */
void Serial_Init(void)
{
#if SERIAL_USE_USART1
	Serial_InitPort(SERIAL_PORT_1);
#endif

#if SERIAL_USE_USART2
	Serial_InitPort(SERIAL_PORT_2);
#endif
}

/**
  * @brief  Send data through the selected serial port.
  * @param  serial Serial hardware identifier.
  * @param  data Data buffer to send.
  * @param  length Number of bytes to send.
  * @retval Number of bytes sent.
  */
uint16_t Serial_Send(Serial_TypeDef serial, const uint8_t *data, uint16_t length)
{
	uint16_t i;
	USART_TypeDef *usart = Serial_GetUsart(serial);

	if (usart == 0 || data == 0)
	{
		return 0;
	}

	for (i = 0; i < length; i++)
	{
		USART_SendData(usart, data[i]);
		while (USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET)
		{
		}
	}

	return length;
}

/**
  * @brief  Read data from the selected serial port receive buffer.
  * @param  serial Serial hardware identifier.
  * @param  data Destination buffer.
  * @param  length Maximum number of bytes to read.
  * @retval Number of bytes read.
  */
uint16_t Serial_Receive(Serial_TypeDef serial, uint8_t *data, uint16_t length)
{
	uint16_t count = 0;
	Serial_RxBufferTypeDef *rxBuffer = Serial_GetRxBuffer(serial);

	if (rxBuffer == 0 || data == 0)
	{
		return 0;
	}

	while (count < length && rxBuffer->readIndex != rxBuffer->writeIndex)
	{
		data[count] = rxBuffer->buffer[rxBuffer->readIndex];
		rxBuffer->readIndex = (rxBuffer->readIndex + 1) % rxBuffer->size;
		count++;
	}

	return count;
}

void Serial_Printf(Serial_TypeDef serial, const char *format, ...)
{
	char buf[128];
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	Serial_Send(serial, (uint8_t *)buf, (uint16_t)strlen(buf));
}

#if SERIAL_USE_USART1
/**
  * @brief  USART1 interrupt handler.
  * @param  None.
  * @retval None.
  */
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(SERIAL1_USART, USART_IT_RXNE) == SET)
	{
		Serial_RxBufferPush(&Serial1_RxBuffer, (uint8_t)USART_ReceiveData(SERIAL1_USART));
		USART_ClearITPendingBit(SERIAL1_USART, USART_IT_RXNE);
	}
}
#endif

#if SERIAL_USE_USART2
/**
  * @brief  USART2 interrupt handler.
  * @param  None.
  * @retval None.
  */
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(SERIAL2_USART, USART_IT_RXNE) == SET)
	{
		Serial_RxBufferPush(&Serial2_RxBuffer, (uint8_t)USART_ReceiveData(SERIAL2_USART));
		USART_ClearITPendingBit(SERIAL2_USART, USART_IT_RXNE);
	}
}
#endif

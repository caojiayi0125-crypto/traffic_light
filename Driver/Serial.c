#include "Serial.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define SERIAL_USE_USART1      SERIAL_PORT1_ENABLED  /**< 是否启用USART1。 */
#define SERIAL_USE_USART2      SERIAL_PORT2_ENABLED  /**< 是否启用USART2。 */

#define SERIAL1_USART          USART1                /**< USART1外设基地址。 */
#define SERIAL1_USART_CLOCK    RCC_APB2Periph_USART1 /**< USART1时钟。 */
#define SERIAL1_IRQ            USART1_IRQn           /**< USART1中断号。 */
#define SERIAL1_GPIO_CLOCK     RCC_APB2Periph_GPIOA  /**< USART1 GPIO时钟。 */
#define SERIAL1_GPIO_PORT      GPIOA                 /**< USART1 GPIO端口。 */
#define SERIAL1_TX_GPIO_PIN    GPIO_Pin_9            /**< USART1 TX引脚：PA9。 */
#define SERIAL1_RX_GPIO_PIN    GPIO_Pin_10           /**< USART1 RX引脚：PA10。 */

#define SERIAL2_USART          USART2                /**< USART2外设基地址。 */
#define SERIAL2_USART_CLOCK    RCC_APB1Periph_USART2 /**< USART2时钟。 */
#define SERIAL2_IRQ            USART2_IRQn           /**< USART2中断号。 */
#define SERIAL2_GPIO_CLOCK     RCC_APB2Periph_GPIOA  /**< USART2 GPIO时钟。 */
#define SERIAL2_GPIO_PORT      GPIOA                 /**< USART2 GPIO端口。 */
#define SERIAL2_TX_GPIO_PIN    GPIO_Pin_2            /**< USART2 TX引脚：PA2。 */
#define SERIAL2_RX_GPIO_PIN    GPIO_Pin_3            /**< USART2 RX引脚：PA3。 */

/**
  * @brief 串口接收环形缓冲区结构体。
  */
typedef struct
{
	volatile uint8_t *buffer;     /**< 缓冲区数据指针。 */
	uint16_t size;                /**< 缓冲区容量。 */
	volatile uint16_t writeIndex; /**< 写指针（ISR写入）。 */
	volatile uint16_t readIndex;  /**< 读指针（主循环读出）。 */
} Serial_RxBufferTypeDef;

#if SERIAL_USE_USART1
static volatile uint8_t Serial1_Buffer[SERIAL1_BUFFER_SIZE];   /**< USART1接收缓冲区。 */
static Serial_RxBufferTypeDef Serial1_RxBuffer;                /**< USART1缓冲区控制结构。 */
#endif

#if SERIAL_USE_USART2
static volatile uint8_t Serial2_Buffer[SERIAL2_BUFFER_SIZE];   /**< USART2接收缓冲区。 */
static Serial_RxBufferTypeDef Serial2_RxBuffer;                /**< USART2缓冲区控制结构。 */
#endif

/**
  * @brief  根据串口标识符获取USART外设指针。
  * @param  serial 串口硬件标识符。
  * @retval USART外设指针，端口未启用或无效时返回0。
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
  * @brief  根据串口标识符获取接收缓冲区指针。
  * @param  serial 串口硬件标识符。
  * @retval 接收缓冲区指针，端口未启用或无效时返回0。
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
  * @brief  将接收到的一个字节存入指定接收缓冲区。
  * @param  rxBuffer 接收缓冲区指针。
  * @param  data 接收到的字节。
  * @retval 无。
  * @note   缓冲区满时，最旧未读的字节将被覆盖（丢弃策略）。
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
  * @brief  配置USART1的GPIO引脚。
  * @param  无。
  * @retval 无。
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
  * @brief  配置USART2的GPIO引脚。
  * @param  无。
  * @retval 无。
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
  * @brief  配置USART通用参数并启用。
  * @param  usart USART外设指针。
  * @param  baudRate 波特率。
  * @param  wordLength 数据位长度。
  * @param  stopBits 停止位。
  * @param  parity 校验位。
  * @param  flowControl 硬件流控。
  * @param  mode 通信模式（TX/RX）。
  * @retval 无。
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
  * @brief  配置指定USART中断的NVIC。
  * @param  irqChannel 中断通道号。
  * @retval 无。
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
  * @brief  初始化单个串口的全部硬件（GPIO + USART + NVIC）。
  * @param  serial 串口硬件标识符。
  * @retval 无。
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
  * @brief  根据SERIAL_USE_MODE初始化已启用的串口。
  * @param  无。
  * @retval 无。
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
  * @brief  通过指定串口发送数据。
  * @param  serial 串口硬件标识符。
  * @param  data 待发送的数据缓冲区。
  * @param  length 发送的字节数。
  * @retval 实际发送的字节数。
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
  * @brief  从指定串口的接收缓冲区读取数据。
  * @param  serial 串口硬件标识符。
  * @param  data 目标缓冲区。
  * @param  length 最多读取的字节数。
  * @retval 实际读取的字节数。
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

/**
  * @brief  通过指定串口发送格式化字符串。
  * @param  serial 串口硬件标识符。
  * @param  format 格式化字符串（printf风格）。
  * @param  ... 可变参数列表。
  * @retval 无。
  */
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
  * @brief  USART1中断服务函数。
  * @param  无。
  * @retval 无。
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
  * @brief  USART2中断服务函数。
  * @param  无。
  * @retval 无。
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

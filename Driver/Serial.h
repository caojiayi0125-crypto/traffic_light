#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"

#define SERIAL_USE_PORT1       1
#define SERIAL_USE_PORT2       2
#define SERIAL_USE_BOTH        3

#define SERIAL_USE_MODE        SERIAL_USE_PORT1

#define SERIAL_PORT1_ENABLED   ((SERIAL_USE_MODE == SERIAL_USE_PORT1) || (SERIAL_USE_MODE == SERIAL_USE_BOTH))
#define SERIAL_PORT2_ENABLED   ((SERIAL_USE_MODE == SERIAL_USE_PORT2) || (SERIAL_USE_MODE == SERIAL_USE_BOTH))

#if SERIAL_PORT1_ENABLED
#define SERIAL1_BUFFER_SIZE    128
#define SERIAL1_BAUD_RATE      9600
#define SERIAL1_WORD_LENGTH    USART_WordLength_8b
#define SERIAL1_STOP_BITS      USART_StopBits_1
#define SERIAL1_PARITY         USART_Parity_No
#define SERIAL1_FLOW_CONTROL   USART_HardwareFlowControl_None
#define SERIAL1_MODE           (USART_Mode_Tx | USART_Mode_Rx)
#endif

#if SERIAL_PORT2_ENABLED
#define SERIAL2_BUFFER_SIZE    128
#define SERIAL2_BAUD_RATE      9600
#define SERIAL2_WORD_LENGTH    USART_WordLength_8b
#define SERIAL2_STOP_BITS      USART_StopBits_1
#define SERIAL2_PARITY         USART_Parity_No
#define SERIAL2_FLOW_CONTROL   USART_HardwareFlowControl_None
#define SERIAL2_MODE           (USART_Mode_Tx | USART_Mode_Rx)
#endif

#if !SERIAL_PORT1_ENABLED && !SERIAL_PORT2_ENABLED
#error "SERIAL_USE_MODE must be SERIAL_USE_PORT1, SERIAL_USE_PORT2, or SERIAL_USE_BOTH."
#endif

/**
  * @brief Serial hardware identifier.
  */
typedef enum
{
	SERIAL_PORT_1 = 0,  /**< USART1 on PA9/PA10. */
	SERIAL_PORT_2       /**< USART2 on PA2/PA3. */
} Serial_TypeDef;

/**
  * @brief  Initialize enabled serial ports according to SERIAL_USE_MODE.
  * @param  None.
  * @retval None.
  */
void Serial_Init(void);

/**
  * @brief  Send data through the selected serial port.
  * @param  serial Serial hardware identifier.
  * @param  data Data buffer to send.
  * @param  length Number of bytes to send.
  * @retval Number of bytes sent.
  */
uint16_t Serial_Send(Serial_TypeDef serial, const uint8_t *data, uint16_t length);

/**
  * @brief  Read data from the selected serial port receive buffer.
  * @param  serial Serial hardware identifier.
  * @param  data Destination buffer.
  * @param  length Maximum number of bytes to read.
  * @retval Number of bytes read.
  */
uint16_t Serial_Receive(Serial_TypeDef serial, uint8_t *data, uint16_t length);

#endif

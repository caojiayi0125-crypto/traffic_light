#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"

#define SERIAL_USE_PORT1       1      /**< 仅使用串口1。 */
#define SERIAL_USE_PORT2       2      /**< 仅使用串口2。 */
#define SERIAL_USE_BOTH        3      /**< 同时使用串口1和串口2。 */

#define SERIAL_USE_MODE        SERIAL_USE_PORT1  /**< 当前启用的串口模式。 */

#define SERIAL_PORT1_ENABLED   ((SERIAL_USE_MODE == SERIAL_USE_PORT1) || (SERIAL_USE_MODE == SERIAL_USE_BOTH))
#define SERIAL_PORT2_ENABLED   ((SERIAL_USE_MODE == SERIAL_USE_PORT2) || (SERIAL_USE_MODE == SERIAL_USE_BOTH))

#if SERIAL_PORT1_ENABLED
#define SERIAL1_BUFFER_SIZE    128                   /**< 串口1接收缓冲区大小（字节）。 */
#define SERIAL1_BAUD_RATE      9600                  /**< 串口1波特率。 */
#define SERIAL1_WORD_LENGTH    USART_WordLength_8b   /**< 串口1数据位长度：8位。 */
#define SERIAL1_STOP_BITS      USART_StopBits_1      /**< 串口1停止位：1位。 */
#define SERIAL1_PARITY         USART_Parity_No       /**< 串口1校验位：无。 */
#define SERIAL1_FLOW_CONTROL   USART_HardwareFlowControl_None  /**< 串口1流控：无。 */
#define SERIAL1_MODE           (USART_Mode_Tx | USART_Mode_Rx) /**< 串口1模式：收发双工。 */
#endif

#if SERIAL_PORT2_ENABLED
#define SERIAL2_BUFFER_SIZE    128                   /**< 串口2接收缓冲区大小（字节）。 */
#define SERIAL2_BAUD_RATE      9600                  /**< 串口2波特率。 */
#define SERIAL2_WORD_LENGTH    USART_WordLength_8b   /**< 串口2数据位长度：8位。 */
#define SERIAL2_STOP_BITS      USART_StopBits_1      /**< 串口2停止位：1位。 */
#define SERIAL2_PARITY         USART_Parity_No       /**< 串口2校验位：无。 */
#define SERIAL2_FLOW_CONTROL   USART_HardwareFlowControl_None  /**< 串口2流控：无。 */
#define SERIAL2_MODE           (USART_Mode_Tx | USART_Mode_Rx) /**< 串口2模式：收发双工。 */
#endif

#if !SERIAL_PORT1_ENABLED && !SERIAL_PORT2_ENABLED
#error "SERIAL_USE_MODE must be SERIAL_USE_PORT1, SERIAL_USE_PORT2, or SERIAL_USE_BOTH."
#endif

/**
  * @brief 串口硬件标识符。
  */
typedef enum
{
	SERIAL_PORT_1 = 0,  /**< USART1，引脚PA9/PA10。 */
	SERIAL_PORT_2       /**< USART2，引脚PA2/PA3。 */
} Serial_TypeDef;

/**
  * @brief  根据SERIAL_USE_MODE初始化已启用的串口。
  * @param  无。
  * @retval 无。
  */
void Serial_Init(void);

/**
  * @brief  通过指定串口发送数据。
  * @param  serial 串口硬件标识符。
  * @param  data 待发送的数据缓冲区。
  * @param  length 发送的字节数。
  * @retval 实际发送的字节数。
  */
uint16_t Serial_Send(Serial_TypeDef serial, const uint8_t *data, uint16_t length);

/**
  * @brief  从指定串口的接收缓冲区读取数据。
  * @param  serial 串口硬件标识符。
  * @param  data 目标缓冲区。
  * @param  length 最多读取的字节数。
  * @retval 实际读取的字节数。
  */
uint16_t Serial_Receive(Serial_TypeDef serial, uint8_t *data, uint16_t length);

/**
  * @brief  通过指定串口发送格式化字符串。
  * @param  serial 串口硬件标识符。
  * @param  format 格式化字符串（printf风格）。
  * @param  ... 可变参数列表。
  * @retval 无。
  */
void Serial_Printf(Serial_TypeDef serial, const char *format, ...);

#endif

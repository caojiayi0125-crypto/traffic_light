#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

/**
  * @brief  微秒级延时。
  * @param  us 延时时长（微秒），范围：0~233015。
  * @retval 无。
  */
void Delay_us(uint32_t us);

/**
  * @brief  毫秒级延时。
  * @param  ms 延时时长（毫秒），范围：0~4294967295。
  * @retval 无。
  */
void Delay_ms(uint32_t ms);

/**
  * @brief  秒级延时。
  * @param  s 延时时长（秒），范围：0~4294967295。
  * @retval 无。
  */
void Delay_s(uint32_t s);

#endif

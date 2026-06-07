/**
  * @file    main.c
  * @brief   程序入口。
  * @note    初始化统筹调度器后进入主循环，永不返回。
  */

#include "stm32f10x.h"
#include "controller.h"

/**
  * @brief  程序入口函数。
  * @param  无。
  * @retval 无（永不返回）。
  */
int main(void)
{
	Ctrl_Init();
	Ctrl_Run();
}

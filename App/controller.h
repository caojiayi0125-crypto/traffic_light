#ifndef __CONTROLLER_H
#define __CONTROLLER_H

/**
  * @brief  初始化所有子系统并进入主循环。
  * @note   初始化顺序：LED → OLED → Serial → TIM2 → Scheduler → Command。
  * @param  无。
  * @retval 无。
  */
void Ctrl_Init(void);

/**
  * @brief  运行主循环（永不返回）。
  * @note   每轮检测 tim1sFlag，驱动调度器走秒、刷新LED/OLED/串口输出，
  *          然后轮询 Cmd_Process 处理串口命令。
  * @param  无。
  * @retval 无。
  */
void Ctrl_Run(void);

#endif

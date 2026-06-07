#ifndef __COMMAND_H
#define __COMMAND_H

/**
  * @brief  初始化串口命令系统。
  * @note   清空输入缓冲区，通过串口打印欢迎横幅和操作提示。
  * @param  无。
  * @retval 无。
  */
void Cmd_Init(void);

/**
  * @brief  非阻塞轮询串口输入，解析并执行命令。
  * @note   逐字节读取串口环形缓冲区，遇回车换行时解析整条命令。
  *         需在主循环中高频调用以保证响应速度。
  * @param  无。
  * @retval 无。
  */
void Cmd_Process(void);

#endif

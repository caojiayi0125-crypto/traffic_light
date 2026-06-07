/**
  * @file    command.c
  * @brief   串口命令行模块实现。
  * @note    支持 mode / status / help 三条命令，采用命令表驱动架构。
  *          通过 USART1 中断接收、环形缓冲、主循环轮询的方式实现非阻塞解析。
  */

#include "command.h"
#include "Serial.h"
#include "scheduler.h"
#include <string.h>

/** @brief 命令行输入缓冲区大小（字节）。 */
#define CMD_BUF_SIZE 64

/** @brief 命令行输入缓冲区。 */
static char    cmd_buf[CMD_BUF_SIZE];
/** @brief 当前缓冲区写入位置。 */
static uint8_t cmd_index;

/**
  * @brief 命令表条目。
  */
typedef struct {
	const char *name;                              /**< 命令名称。 */
	void (*handler)(int argc, char *argv[]);       /**< 命令处理函数。 */
	const char *help;                              /**< 帮助信息。 */
} CmdEntry;

/* ── 命令处理函数声明 ── */
static void cmd_mode(int argc, char *argv[]);
static void cmd_status(int argc, char *argv[]);
static void cmd_help(int argc, char *argv[]);

/** @brief 命令注册表。 */
static const CmdEntry cmd_table[] = {
	{"mode",   cmd_mode,   "mode <normal|rush|idle|night> - Switch timing mode"},
	{"status", cmd_status, "status - Show current mode, light state, countdown"},
	{"help",   cmd_help,   "help  - Show available commands"},
};

/** @brief 命令注册表条目数。 */
#define CMD_COUNT (sizeof(cmd_table) / sizeof(cmd_table[0]))

/**
  * @brief  mode 命令处理：切换配时模式。
  * @param  argc 参数个数（不含命令名）。
  * @param  argv 参数数组。
  * @retval 无。
  */
static void cmd_mode(int argc, char *argv[])
{
	if (argc < 1) {
		Serial_Printf(SERIAL_PORT_1,
			"Usage: mode <normal|rush|idle|night>\r\n");
		return;
	}

	if (strcmp(argv[0], "normal") == 0) {
		Sched_SetMode(MODE_NORMAL);
	} else if (strcmp(argv[0], "rush") == 0) {
		Sched_SetMode(MODE_RUSH);
	} else if (strcmp(argv[0], "idle") == 0) {
		Sched_SetMode(MODE_IDLE);
	} else if (strcmp(argv[0], "night") == 0) {
		Sched_SetMode(MODE_NIGHT);
	} else {
		Serial_Printf(SERIAL_PORT_1,
			"Unknown mode: %s. Use: normal rush idle night\r\n", argv[0]);
		return;
	}

	Serial_Printf(SERIAL_PORT_1,
		"[MODE] Switched to %s\r\n", Sched_GetModeName());
}

/**
  * @brief  status 命令处理：查询当前运行状态。
  * @param  argc 未使用。
  * @param  argv 未使用。
  * @retval 无。
  */
static void cmd_status(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	const char *state_name;
	switch (Sched_GetState()) {
	case STATE_GREEN:  state_name = "GREEN";  break;
	case STATE_YELLOW: state_name = "YELLOW"; break;
	case STATE_RED:    state_name = "RED";    break;
	default:           state_name = "?";      break;
	}

	if (Sched_GetMode() == MODE_NIGHT) {
		Serial_Printf(SERIAL_PORT_1,
			"[MODE] %s | YELLOW %s\r\n",
			Sched_GetModeName(),
			Sched_IsNightBlinkOn() ? "ON" : "OFF");
	} else {
		Serial_Printf(SERIAL_PORT_1,
			"[MODE] %s | State: %s | Countdown: %lus\r\n",
			Sched_GetModeName(), state_name, Sched_GetCountdown());
	}
}

/**
  * @brief  help 命令处理：列出全部可用命令。
  * @param  argc 未使用。
  * @param  argv 未使用。
  * @retval 无。
  */
static void cmd_help(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	Serial_Printf(SERIAL_PORT_1, "=== Commands ===\r\n");
	uint8_t i;
	for (i = 0; i < CMD_COUNT; i++) {
		Serial_Printf(SERIAL_PORT_1, "  %s\r\n", cmd_table[i].help);
	}
}

/**
  * @brief  初始化命令系统。
  * @note   清空缓冲区，通过串口打印欢迎横幅。
  * @param  无。
  * @retval 无。
  */
void Cmd_Init(void)
{
	cmd_index = 0;
	memset(cmd_buf, 0, sizeof(cmd_buf));

	Serial_Printf(SERIAL_PORT_1,
		"\r\n=== Traffic Light Console ===\r\n");
	Serial_Printf(SERIAL_PORT_1,
		"Type 'help' for available commands\r\n\r\n");
}

/**
  * @brief  解析并执行单条命令行。
  * @note   按空格拆分参数，在命令表中查找匹配项并调用处理函数。
  *         未匹配时打印 Unknown command 提示。
  * @param  line 以 '\\0' 结尾的命令字符串。
  * @retval 无。
  */
static void cmd_execute(char *line)
{
	char *argv[4];
	int   argc = 0;

	char *token = strtok(line, " ");
	while (token != NULL && argc < 4) {
		argv[argc++] = token;
		token = strtok(NULL, " ");
	}

	if (argc == 0) return;

	uint8_t i;
	for (i = 0; i < CMD_COUNT; i++) {
		if (strcmp(argv[0], cmd_table[i].name) == 0) {
			cmd_table[i].handler(argc - 1, &argv[1]);
			return;
		}
	}

	Serial_Printf(SERIAL_PORT_1,
		"Unknown command: %s. Type 'help' for help.\r\n", argv[0]);
}

/**
  * @brief  非阻塞轮询串口输入。
  * @note   逐字节读取环形缓冲区，累积到 \\r 或 \\n 时解析并回显。
  * @param  无。
  * @retval 无。
  */
void Cmd_Process(void)
{
	uint8_t ch;

	while (Serial_Receive(SERIAL_PORT_1, &ch, 1) == 1) {
		if (ch == '\r' || ch == '\n') {
			if (cmd_index > 0) {
				cmd_buf[cmd_index] = '\0';
				Serial_Printf(SERIAL_PORT_1, "> %s\r\n", cmd_buf);
				cmd_execute(cmd_buf);
				cmd_index = 0;
				memset(cmd_buf, 0, sizeof(cmd_buf));
			}
		} else if (cmd_index < CMD_BUF_SIZE - 1) {
			cmd_buf[cmd_index++] = (char)ch;
		}
	}
}

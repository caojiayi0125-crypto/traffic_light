#include "command.h"
#include "Serial.h"
#include "scheduler.h"
#include <string.h>
#include <stdio.h>

#define CMD_BUF_SIZE 64

static char    cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_index;

typedef struct {
	const char *name;
	void (*handler)(int argc, char *argv[]);
	const char *help;
} CmdEntry;

static void cmd_mode(int argc, char *argv[]);
static void cmd_status(int argc, char *argv[]);
static void cmd_help(int argc, char *argv[]);

static const CmdEntry cmd_table[] = {
	{"mode",   cmd_mode,   "mode <normal|rush|idle|night> - Switch timing mode"},
	{"status", cmd_status, "status - Show current mode, light state, countdown"},
	{"help",   cmd_help,   "help  - Show available commands"},
};

#define CMD_COUNT (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

void Cmd_Init(void)
{
	cmd_index = 0;
	memset(cmd_buf, 0, sizeof(cmd_buf));

	Serial_Printf(SERIAL_PORT_1,
		"\r\n=== Traffic Light Console ===\r\n");
	Serial_Printf(SERIAL_PORT_1,
		"Type 'help' for available commands\r\n\r\n");
}

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

#include "OLED.h"
#include "OLED_Font.h"

#define OLED_WIDTH             128
#define OLED_PAGE_COUNT        8
#define OLED_I2C_ADDRESS       0x78
#define OLED_DIGIT_GAP         8
#define OLED_COUNTDOWN_X       ((OLED_WIDTH - (OLED_DIGIT_WIDTH * 2) - OLED_DIGIT_GAP) / 2)

#define OLED_W_SCL(x)          GPIO_WriteBit(OLED_GPIO_PORT, OLED_SCL_GPIO_PIN, (BitAction)(x))
#define OLED_W_SDA(x)          GPIO_WriteBit(OLED_GPIO_PORT, OLED_SDA_GPIO_PIN, (BitAction)(x))

/**
  * @brief  Initialize GPIO pins used by OLED software I2C.
  * @param  None.
  * @retval None.
  */
static void OLED_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(OLED_GPIO_CLOCK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = OLED_SCL_GPIO_PIN | OLED_SDA_GPIO_PIN;
	GPIO_Init(OLED_GPIO_PORT, &GPIO_InitStructure);

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  Generate software I2C start condition.
  * @param  None.
  * @retval None.
  */
static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  Generate software I2C stop condition.
  * @param  None.
  * @retval None.
  */
static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  Send one byte through software I2C.
  * @param  byte Byte to send.
  * @retval None.
  */
static void OLED_I2C_SendByte(uint8_t byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(byte & (0x80 >> i)));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}

	OLED_W_SCL(1);
	OLED_W_SCL(0);
}

/**
  * @brief  Write one command byte to the OLED controller.
  * @param  command Command byte.
  * @retval None.
  */
static void OLED_WriteCommand(uint8_t command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(OLED_I2C_ADDRESS);
	OLED_I2C_SendByte(0x00);
	OLED_I2C_SendByte(command);
	OLED_I2C_Stop();
}

/**
  * @brief  Write one data byte to the OLED controller.
  * @param  data Data byte.
  * @retval None.
  */
static void OLED_WriteData(uint8_t data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(OLED_I2C_ADDRESS);
	OLED_I2C_SendByte(0x40);
	OLED_I2C_SendByte(data);
	OLED_I2C_Stop();
}

/**
  * @brief  Set SSD1306 page and column cursor.
  * @param  page Page index, range 0 to 7.
  * @param  column Column index, range 0 to 127.
  * @retval None.
  */
static void OLED_SetCursor(uint8_t page, uint8_t column)
{
	OLED_WriteCommand(0xB0 | page);
	OLED_WriteCommand(0x10 | ((column & 0xF0) >> 4));
	OLED_WriteCommand(0x00 | (column & 0x0F));
}

/**
  * @brief  Fill the whole OLED screen with one byte pattern.
  * @param  data Page byte to write across the screen.
  * @retval None.
  */
static void OLED_Fill(uint8_t data)
{
	uint8_t page;
	uint8_t column;

	for (page = 0; page < OLED_PAGE_COUNT; page++)
	{
		OLED_SetCursor(page, 0);
		for (column = 0; column < OLED_WIDTH; column++)
		{
			OLED_WriteData(data);
		}
	}
}

/**
  * @brief  Write one page row for a two-digit countdown value.
  * @param  page Page index, range 0 to 7.
  * @param  tens Tens digit, range 0 to 9.
  * @param  ones Ones digit, range 0 to 9.
  * @retval None.
  */
static void OLED_WriteCountdownPage(uint8_t page, uint8_t tens, uint8_t ones)
{
	uint8_t column;
	uint16_t offset = (uint16_t)page * OLED_DIGIT_WIDTH;

	OLED_SetCursor(page, 0);

	for (column = 0; column < OLED_COUNTDOWN_X; column++)
	{
		OLED_WriteData(0x00);
	}

	for (column = 0; column < OLED_DIGIT_WIDTH; column++)
	{
		OLED_WriteData(OLED_Digit48x64[tens][offset + column]);
	}

	for (column = 0; column < OLED_DIGIT_GAP; column++)
	{
		OLED_WriteData(0x00);
	}

	for (column = 0; column < OLED_DIGIT_WIDTH; column++)
	{
		OLED_WriteData(OLED_Digit48x64[ones][offset + column]);
	}

	for (column = 0; column < OLED_COUNTDOWN_X; column++)
	{
		OLED_WriteData(0x00);
	}
}

/**
  * @brief  Clear the whole OLED screen.
  * @param  None.
  * @retval None.
  */
void OLED_Clear(void)
{
	OLED_Fill(0x00);
}

/**
  * @brief  Show a two-digit countdown number on the full OLED screen.
  * @param  value Countdown value. Values greater than 99 are displayed as 99.
  * @retval None.
  */
void OLED_ShowCountdown(uint8_t value)
{
	uint8_t page;
	uint8_t tens;
	uint8_t ones;

	if (value > 99)
	{
		value = 99;
	}

	tens = value / 10;
	ones = value % 10;

	for (page = 0; page < OLED_PAGE_COUNT; page++)
	{
		OLED_WriteCountdownPage(page, tens, ones);
	}
}

/**
  * @brief  Initialize OLED GPIO and controller.
  * @param  None.
  * @retval None.
  */
void OLED_Init(void)
{
	uint32_t i;
	uint32_t j;

	for (i = 0; i < 1000; i++)
	{
		for (j = 0; j < 1000; j++)
		{
		}
	}

	OLED_I2C_Init();

	OLED_WriteCommand(0xAE);
	OLED_WriteCommand(0xD5);
	OLED_WriteCommand(0x80);
	OLED_WriteCommand(0xA8);
	OLED_WriteCommand(0x3F);
	OLED_WriteCommand(0xD3);
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0x40);
	OLED_WriteCommand(0xA1);
	OLED_WriteCommand(0xC8);
	OLED_WriteCommand(0xDA);
	OLED_WriteCommand(0x12);
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(0xCF);
	OLED_WriteCommand(0xD9);
	OLED_WriteCommand(0xF1);
	OLED_WriteCommand(0xDB);
	OLED_WriteCommand(0x30);
	OLED_WriteCommand(0xA4);
	OLED_WriteCommand(0xA6);
	OLED_WriteCommand(0x8D);
	OLED_WriteCommand(0x14);
	OLED_WriteCommand(0xAF);

	OLED_Clear();
}

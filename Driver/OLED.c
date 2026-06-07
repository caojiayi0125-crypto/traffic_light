#include "OLED.h"
#include "OLED_Font.h"

/** @brief OLED屏幕宽度（像素）。 */
#define OLED_WIDTH             128
/** @brief OLED页数（SSD1306每页8像素高）。 */
#define OLED_PAGE_COUNT        8
/** @brief OLED的I2C从机地址。 */
#define OLED_I2C_ADDRESS       0x78
/** @brief 两位数字之间的间距（像素）。 */
#define OLED_DIGIT_GAP         8
/** @brief 倒计时数字的起始X坐标（居中）。 */
#define OLED_COUNTDOWN_X       ((OLED_WIDTH - (OLED_DIGIT_WIDTH * 2) - OLED_DIGIT_GAP) / 2)

/** @brief 写SCL引脚电平。 */
#define OLED_W_SCL(x)          GPIO_WriteBit(OLED_GPIO_PORT, OLED_SCL_GPIO_PIN, (BitAction)(x))
/** @brief 写SDA引脚电平。 */
#define OLED_W_SDA(x)          GPIO_WriteBit(OLED_GPIO_PORT, OLED_SDA_GPIO_PIN, (BitAction)(x))

/**
  * @brief  初始化OLED软件I2C所用的GPIO引脚。
  * @param  无。
  * @retval 无。
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
  * @brief  软件I2C产生起始信号。
  * @param  无。
  * @retval 无。
  */
static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  软件I2C产生停止信号。
  * @param  无。
  * @retval 无。
  */
static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  通过软件I2C发送一个字节。
  * @param  byte 待发送的字节。
  * @retval 无。
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
  * @brief  向SSD1306控制器写入一个命令字节。
  * @param  command 命令字节。
  * @retval 无。
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
  * @brief  向SSD1306控制器写入一个数据字节。
  * @param  data 数据字节。
  * @retval 无。
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
  * @brief  设置SSD1306的页和列光标位置。
  * @param  page 页索引，取值范围0~7。
  * @param  column 列索引，取值范围0~127。
  * @retval 无。
  */
static void OLED_SetCursor(uint8_t page, uint8_t column)
{
	OLED_WriteCommand(0xB0 | page);
	OLED_WriteCommand(0x10 | ((column & 0xF0) >> 4));
	OLED_WriteCommand(0x00 | (column & 0x0F));
}

/**
  * @brief  用指定字节填充整个OLED屏幕。
  * @param  data 填充用的页字节。
  * @retval 无。
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
  * @brief  写入两位倒计时数字中某一页的数据。
  * @param  page 页索引，取值范围0~7。
  * @param  tens 十位数字，取值范围0~9。
  * @param  ones 个位数字，取值范围0~9。
  * @retval 无。
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
  * @brief  清除整个OLED屏幕（全写0）。
  * @param  无。
  * @retval 无。
  */
void OLED_Clear(void)
{
	OLED_Fill(0x00);
}

/**
  * @brief  在OLED屏幕上显示两位倒计时数字。
  * @param  value 倒计时数值，超过99则显示为99。
  * @retval 无。
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
  * @brief  初始化OLED的GPIO引脚及SSD1306控制器。
  * @note   上电延时后依次发送SSD1306初始化命令序列，最后清屏。
  * @param  无。
  * @retval 无。
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

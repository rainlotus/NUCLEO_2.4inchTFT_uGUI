/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 LCD TFT Library for 2.4" MCUfriend shield using 8080 8-bit parallel interface
*	based on Adafruit GFX & Adafruit TFT LCD libraries
*	20 Dec 2018 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.7 added support for 24-bit BMP
*		- v1.6 added support for SSD1297 (262k only)
*		- v1.5 added printf wipe line/screen defines
*		- v1.4 added support for R61505
*		- v1.3 added support for HX8347D
*		- v1.2 added Courier New font family with Cyrillic (CP1251), created using TheDotFactory font generator
*		- v1.1 added support for UNKNOWN1602, LCD_DrawBMP() & LCD_DrawBMPFromFile()
*		- v1.0 added support for ILI9325, ILI9328, ILI9340, ILI9341, ILI9341_00, R61505V, R61520, S6D0154 chips
       
	 ????????????????,????????????ILI9341???????? chenrj 2019.10.28
*/

// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license



#include "lcd.h"

static int16_t m_width;
static int16_t m_height;
static int16_t m_cursor_x;
static int16_t m_cursor_y;

static uint16_t m_textcolor;
static uint16_t m_textbgcolor;
static uint8_t m_font;
static uint8_t m_rotation;
static uint8_t m_wrap;


static font_t * fonts[] = {
#ifdef USE_FONT8
					&Font8,
#endif
#ifdef USE_FONT12
					&Font12,
#endif
#ifdef USE_FONT16
					&Font16,
#endif
#ifdef USE_FONT20
					&Font20,
#endif
#ifdef USE_FONT24
					&Font24,
#endif
					};

const static uint8_t fontsNum = sizeof(fonts) / sizeof(fonts[0]);

void delay(uint32_t t);
void GPIO_Init(uint32_t mode);
void LCD_Write8(uint8_t data);
uint8_t LCD_Read8(void);
void LCD_Write8Register8(uint8_t a, uint8_t d);
void LCD_Write16Register8(uint8_t a, uint16_t d);
void LCD_Write24Register8(uint8_t a, uint32_t d);
void LCD_Write32Register8(uint8_t a, uint32_t d);
void LCD_Write16Register16(uint16_t a, uint16_t d);
void LCD_Write16RegisterPair8(uint8_t aH, uint8_t aL, uint16_t d);
uint8_t LCD_Read8Register8(uint8_t a, uint8_t dummy);
uint32_t LCD_Color565_to_888(uint16_t color);
uint8_t LCD_Color565_to_R(uint16_t color);
uint8_t LCD_Color565_to_G(uint16_t color);
uint8_t LCD_Color565_to_B(uint16_t color);

// Initialization command tables for different LCD controllers
#if defined(ILI9341)
static const uint8_t ILI9341_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	ILI9341_POWERCONTROLA		, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	ILI9341_POWERCONTROLB		, 3, 0x00, 0xC1, 0x30,
	ILI9341_DRIVERTIMINGCTLA	, 3, 0x85, 0x00, 0x78,
	ILI9341_DRIVERTIMINGCTLB	, 2, 0x00, 0x00,
	ILI9341_POWERONSEQCONTROL	, 4, 0x64, 0x03, 0x12, 0x81,
	ILI9341_PUMPRATIOCONTROL	, 1, 0x20,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x3E, 0x28,
	ILI9341_VCOMCONTROL2		, 1, 0x86,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR, // change rotation here
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x18,
	ILI9341_DISPLAYFUNC			, 3, 0x08, 0x82, 0x27,
	ILI9341_ENABLE3G			, 1, 0x00,
	ILI9341_GAMMASET			, 1, 0x01,
	ILI9341_POSITIVEGAMMACORR	,15, 0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00,
	ILI9341_NEGATIVEGAMMACORR	,15, 0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(ILI9341_00)
static const uint8_t ILI9341_00_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	TFTLCD_DELAY				, 50,
	ILI9341_DISPLAYOFF			, 0,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x2B, 0x2B,
	ILI9341_VCOMCONTROL2		, 1, 0xC0,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR, // change rotation here  /0x48
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x1B,
	ILI9341_ENTRYMODE			, 1, 0x07,
	ILI9341_INVERTON			, 0,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	TFTLCD_DELAY				, 100,
	ILI9341_MEMORYWRITE			, 0,
};
#endif

/**
 * \brief Makes a delays for t processor cycles
 *
 * \param t		Number of "nop" cycles
 *
 * \return void
 */		
inline void delay(uint32_t t) {
	for (; t; t--) __asm("nop");
}

/**
 * \brief GPIO Initialization
 *
 * \param
 *
 * \return void
 */
void GPIO_Init(uint32_t mode) {

	// GPIO to data bus pin connections
	// ---- PORT Pin ---     --- Data ----  ARDUINO
	// GPIOA, GPIO_PIN_9  -> BIT 0       	D8
	// GPIOC, GPIO_PIN_7  -> BIT 1       	D9
	// GPIOA, GPIO_PIN_10 -> BIT 2       	D2
	// GPIOB, GPIO_PIN_3  -> BIT 3          D3
	// GPIOB, GPIO_PIN_5  -> BIT 4          D4
	// GPIOB, GPIO_PIN_4  -> BIT 5          D5
	// GPIOB, GPIO_PIN_10 -> BIT 6          D6
	// GPIOA, GPIO_PIN_8  -> BIT 7          D7

	// GPIO to control bus pin connections
	// ---- PORT Pin ---     --- Signal ----  ARDUINO

	// GPIOA, GPIO_PIN_0  -> RD              A0
	// GPIOA, GPIO_PIN_1  -> WR              A1
	// GPIOA, GPIO_PIN_4  -> RD	             A2
	// GPIOB, GPIO_PIN_0  -> CS				 A3
	// GPIOC, GPIO_PIN_1  -> RST             A4
	
	if (!(mode == GPIO_MODE_OUTPUT_PP || mode == GPIO_MODE_INPUT)) return;

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO data pins : PA8 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO data pins : PB3 PB4 PB5 PB10 */
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO data pins : PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO control pins : PA0 PA1 PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO control pins : PB0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO control pins : PC1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * \brief Writes 8-Bit data
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
inline void LCD_Write8(uint8_t data) {
	// ------ PORT -----     --- Data ----
	// GPIOA, GPIO_PIN_9  -> BIT 0 -> 0x01
	// GPIOC, GPIO_PIN_7  -> BIT 1 -> 0x02
	// GPIOA, GPIO_PIN_10 -> BIT 2 -> 0x04
	// GPIOB, GPIO_PIN_3  -> BIT 3 -> 0x08
	// GPIOB, GPIO_PIN_5  -> BIT 4 -> 0x10
	// GPIOB, GPIO_PIN_4  -> BIT 5 -> 0x20
	// GPIOB, GPIO_PIN_10 -> BIT 6 -> 0x40
	// GPIOA, GPIO_PIN_8  -> BIT 7 -> 0x80

	GPIOA->ODR = (GPIOA->ODR & 0xF8FF) | ((data & 0x01) << 9) | ((data & 0x04) << 8) | ((data & 0x80) << 1);
	GPIOB->ODR = (GPIOB->ODR & 0xFBC7) | (data & 0x08) | ((data & 0x10) << 1) | ((data & 0x20) >> 1) | ((data & 0x40) << 4);
	GPIOC->ODR = (GPIOC->ODR & 0xFF7F) | ((data & 0x02) << 6);

	LCD_WR_STROBE();
}

/**
 * \brief Reads 8-Bit data
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
inline uint8_t LCD_Read8(void) {
	// - Data - ----------- PORT -----------
	// BIT 0 -> GPIOA, GPIO_PIN_9  -> 0x0200
	// BIT 1 -> GPIOC, GPIO_PIN_7  -> 0x0080
	// BIT 2 -> GPIOA, GPIO_PIN_10 -> 0x0400
	// BIT 3 -> GPIOB, GPIO_PIN_3  -> 0x0008
	// BIT 4 -> GPIOB, GPIO_PIN_5  -> 0x0020
	// BIT 5 -> GPIOB, GPIO_PIN_4  -> 0x0010
	// BIT 6 -> GPIOB, GPIO_PIN_10 -> 0x0400
	// BIT 7 -> GPIOA, GPIO_PIN_8  -> 0x0100
	uint8_t data;
	LCD_RD_STROBE();
	data = ((GPIOA->IDR & 0x0200) >> 9) | ((GPIOA->IDR & 0x0400) >> 8) | ((GPIOA->IDR & 0x0100) >> 1) | (GPIOB->IDR & 0x0008)
			| ((GPIOB->IDR & 0x0020) >> 1) | ((GPIOB->IDR & 0x0010) << 1) | ((GPIOB->IDR & 0x0400) >> 4)
			| ((GPIOC->IDR & 0x0080) >> 6);
	return data;
}

/**
 * \brief Writes 8-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write8Register8(uint8_t a, uint8_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16Register8(uint8_t a, uint16_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 24-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write24Register8(uint8_t a, uint32_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 32-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write32Register8(uint8_t a, uint32_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 24);
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to register (16-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16Register16(uint16_t a, uint16_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a >> 8);
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to two registers (8-bit address)
 *
 * \param aH	Register for hi chunk of data
 * \param aL	Register for lo chunk of data
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16RegisterPair8(uint8_t aH, uint8_t aL, uint16_t d) {
	LCD_Write8Register8(aH, d >> 8);
	LCD_Write8Register8(aL, d);
}

/**
 * \brief Reads 8-Bit data from register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline uint8_t LCD_Read8Register8(uint8_t a, uint8_t dummy) {
	uint8_t data;
	LCD_CD_COMMAND();
	LCD_Write8(a);
	GPIO_Init(GPIO_MODE_INPUT);
	LCD_CD_DATA();
	if (dummy) LCD_RD_STROBE();
	data = LCD_Read8();
	GPIO_Init(GPIO_MODE_OUTPUT_PP);
	return data;
}

/**
 * \brief LCD Initialization
 *
 * \param
 *
 * \return void
 */
void LCD_Init(void) {
	m_width = TFTWIDTH;
	m_height = TFTHEIGHT;
	m_rotation = 0;
	m_cursor_y = m_cursor_x = 0;
	m_font = 0;
	m_textcolor = m_textbgcolor = 0xFFFF;
	m_wrap = 1;

	GPIO_Init(GPIO_MODE_OUTPUT_PP);

	LCD_Reset();
	HAL_Delay(50);

	LCD_CS_ACTIVE();

#if defined(ILI9341)
	uint8_t i = 0;
	while(i < sizeof(ILI9341_regValues)/sizeof(ILI9341_regValues[0])) {
		uint8_t r = ILI9341_regValues[i++];
		uint8_t len = ILI9341_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9341_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(ILI9341_00)
	uint8_t i = 0;
	while(i < sizeof(ILI9341_00_regValues)/sizeof(ILI9341_00_regValues[0])) {
		uint8_t r = ILI9341_00_regValues[i++];
		uint8_t len = ILI9341_00_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9341_00_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#endif

	LCD_CS_IDLE();
	LCD_FillScreen(BLACK);
	LCD_SetTextSize(0);
	LCD_SetTextColor(WHITE, BLACK);
}

/**
 * \brief Draws a point at the specified coordinates
 *
 * \param x		x-Coordinate
 * \param y		y-Coordinate
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color) {
	// Clip
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) return;

	LCD_CS_ACTIVE();
	LCD_SetAddrWindow(x, y, m_width - 1, m_height - 1);
	LCD_CS_ACTIVE();
	LCD_Write16Register8(ILI9341_MEMORYWRITE, color);
	LCD_CS_IDLE();
}

/**
 * \brief Flood  Моід
 *
 * \param color	Color
 * \param len	Length
 *
 * \return void
 */
void LCD_Flood(uint16_t color, uint32_t len) {
	uint8_t hi = color >> 8, lo = color;
	LCD_CS_ACTIVE();
	LCD_Write16Register8(ILI9341_MEMORYWRITE, color);
	len--;
	if (hi == lo) {
		while (len--) {
			LCD_WR_STROBE();
			LCD_WR_STROBE();
		}
	} else {
		while (len--) {
			LCD_Write8(hi);
			LCD_Write8(lo);
		}
	}
	LCD_CS_IDLE();
}

/**
 * \brief Fills the screen with the specified color
 *
 * \param color	Color
 *
 * \return void
 */
void LCD_FillScreen(uint16_t color) {
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
	LCD_Flood(color, (long) TFTWIDTH * (long) TFTHEIGHT);
}

/**
 * \brief Resets the Display
 *
 * \param
 *
 * \return void
 */
void LCD_Reset(void) {
	LCD_CS_IDLE();
	LCD_CD_DATA();
	LCD_WR_IDLE();
	LCD_RD_IDLE();

	LCD_RST_ACTIVE();
	HAL_Delay(2);
	LCD_RST_IDLE();
	HAL_Delay(120);
	// Data transfer sync
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
	LCD_Write8(0x00);
	for (uint8_t i = 0; i < 3; i++) LCD_WR_STROBE(); // Three extra 0x00s
	LCD_CS_IDLE();
}


/**
 * \brief Sets window address
 *
 * \param x1
 * \param y1
 * \param x2
 * \param y2
 *
 * \return void
 */
void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	LCD_CS_ACTIVE();
	LCD_Write32Register8(ILI9341_COLADDRSET, (x1 << 16) | x2);
	LCD_Write32Register8(ILI9341_PAGEADDRSET, (y1 << 16) | y2);
	LCD_CS_IDLE();
}


/**
 * \brief Set display rotation
 *
 * \param x	rotation
 *
 * \return void
 */
void LCD_SetRotation(uint8_t x) {
	m_rotation = (x & 3);
	switch (m_rotation) {
	default:
	case 0:
	case 2:
		m_width = TFTWIDTH;
		m_height = TFTHEIGHT;
		break;
	case 1:
	case 3:
		m_width = TFTHEIGHT;
		m_height = TFTWIDTH;
		break;
	}
	LCD_CS_ACTIVE();
    uint8_t t;
    switch (m_rotation) {
		default: t = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR; break;
		case 1: t = ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
		case 2: t = ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR; break;
		case 3:	t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
    }
    LCD_Write8Register8(ILI9341_MADCTL, t); // MADCTL
    // For 9341, init default full-screen address window:
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
	LCD_CS_IDLE();
}

/**
 * \brief  Draws a BMP picture loaded in the STM32 MCU internal memory.
 *
 * \param xPos		Bmp X position in the LCD
 * \param yPos		Bmp Y position in the LCD
 * \param pBmp		Pointer to Bmp picture address
 * \return void
 */
void LCD_DrawBMP(int16_t xPos, int16_t yPos, const uint8_t *pBmp) {
	uint8_t *ptr;
	uint8_t *start;
	uint8_t *end;
	uint32_t offset = 0, size = 0;
	int32_t height = 0, width = 0;
	uint16_t colordepth = 0;

	/* Read bitmap size */
	size = *(volatile uint16_t *) (pBmp + 2);
	size |= (*(volatile uint16_t *) (pBmp + 4)) << 16;
	/* Get bitmap data address offset */
	offset = *(volatile uint16_t *) (pBmp + 10);
	offset |= (*(volatile uint16_t *) (pBmp + 12)) << 16;
	/* Read bitmap width */
	width = *(uint16_t *) (pBmp + 18);
	width |= (*(uint16_t *) (pBmp + 20)) << 16;
	/* Read bitmap height */
	height = *(uint16_t *) (pBmp + 22);
	height |= (*(uint16_t *) (pBmp + 24)) << 16;
	/* Read color depth */
	colordepth = *(uint16_t *) (pBmp + 28);

	/* Calculate pixel data boundary addresses */
	start = (uint8_t *) pBmp + offset;
	end = (uint8_t *) pBmp + size;

	/* Start drawing */
	if ((xPos + width >= m_width) || (yPos + abs(height) >= m_height)) return;
	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + abs(height) - 1);
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
    LCD_Write8(ILI9341_MEMORYWRITE); // Write data to GRAM
	LCD_CD_DATA();
	if (height < 0) {
		/* Top-bottom file */
		ptr = start;
		/* Draw image */
		if (colordepth == 16) {
			while (ptr < end) {
				ptr += 2;
			}
		} else if (colordepth == 24) {
			while (ptr < end) {
				ptr += 3;
			}
		}
	} else {
		/* Bottom-top file */
		uint8_t *rowstart;
		uint8_t *rowend;
		/* Draw image */
		if (colordepth == 16) {
			for (uint16_t row = height - 1; row > 0; row--) {
				rowstart = start + row * width * 2;
				rowend = start + (row + 1) * width * 2 - 1;
				ptr = rowstart;
				while (ptr < rowend) {
					ptr += 2;
				}
			}
		} else if (colordepth == 24) {
			for (uint16_t row = height - 1; row > 0; row--) {
				rowstart = start + row * width * 3;
				rowend = start + (row + 1) * width * 3 - 1;
				ptr = rowstart;
				while (ptr < rowend) {
					ptr += 3;
				}
			}
		}
	}
	LCD_CS_IDLE();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

#if defined(USE_FATFS)
/**
 * \brief  Draws a bitmap picture from FatFs file.
 *
 * \param  xPos: Bmp X position in the LCD
 * \param  yPos: Bmp Y position in the LCD
 * \param  pFile: Pointer to FIL object with bmp picture
 * \retval None
 */
void LCD_DrawBMPFromFile(int16_t xPos, int16_t yPos, FIL * pFile) {
	uint8_t buf[TFTWIDTH * 3]; // max 240 pixels at a time (720B)
	uint32_t readBytes = 0, clusterSize = 0, clusterNum = 0, clusterTotal = 0;
	uint8_t* pBmp;
	uint8_t* start;
	uint8_t* end;
	uint32_t offset = 0, size = 0;
	int32_t height = 0, width = 0;
	uint16_t colordepth = 0;

	/* Read BMP header: 54 bytes = 14 bytes header + 40 bytes DIB header (assuming BITMAPINFOHEADER) */
	f_read(pFile, buf, 54, (UINT *) &readBytes);
	if (readBytes != 54) {
		return;
	}
	pBmp = buf;
	/* Read bitmap size */
	size = *(volatile uint16_t *) (pBmp + 2);
	size |= (*(volatile uint16_t *) (pBmp + 4)) << 16;
	/* Get bitmap data address offset */
	offset = *(volatile uint16_t *) (pBmp + 10);
	offset |= (*(volatile uint16_t *) (pBmp + 12)) << 16;
	/* Read bitmap width */
	width = *(uint16_t *) (pBmp + 18);
	width |= (*(uint16_t *) (pBmp + 20)) << 16;
	/* Read bitmap height */
	height = *(uint16_t *) (pBmp + 22);
	height |= (*(uint16_t *) (pBmp + 24)) << 16;
	/* Read color depth */
	colordepth = *(uint16_t *) (pBmp + 28);

	/* Calculate total number of clusters to read */
	clusterSize = width * colordepth / 8;
	clusterTotal = abs(height);

	/* Start drawing */
	if ((xPos + width > m_width) || (yPos + abs(height) > m_height) || clusterSize > sizeof(buf)) return;
	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + abs(height) - 1);
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
	LCD_Write8(ILI9341_MEMORYWRITE); // Write data to GRAM

	LCD_CD_DATA();
	if (height < 0) {
		/* Top-bottom file */
		/* Move read pointer to beginning of pixel data */
		f_lseek(pFile, offset);
		clusterNum = 0;
		while (clusterNum <= clusterTotal) {
			/* Read new cluster */
			f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
			start = buf;
			end = buf + readBytes;
			pBmp = start;
			/* Draw image */
			if (colordepth == 16) {
				while (pBmp < end) {
					pBmp += 2;
				}
			} else if (colordepth == 24) {
				while (pBmp < end) {
					pBmp += 3;
				}
			}
			clusterNum++;
		}
	} else {
		/* Bottom-top file */
		clusterNum = clusterTotal;
		while (clusterNum > 0) {
			f_lseek(pFile, offset + (clusterNum - 1) * clusterSize);
			f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
			start = buf;
			end = buf + readBytes;
			pBmp = start;
			if (colordepth == 16) {
				while (pBmp < end) {
					pBmp += 2;
				}
			} else if (colordepth == 24) {
				while (pBmp < end) {
					pBmp += 3;
				}
			}
			clusterNum--;
		}
	}
	LCD_CS_IDLE();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}
#endif

/**
 * \brief Draws a line connecting the two points specified by the coordinate pairs
 *
 * \param x0	The x-coordinate of the first point
 * \param y0	The y-coordinate of the first point
 * \param x1	The x-coordinate of the second point
 * \param y1	The y-coordinate of the second point.
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	// Bresenham's algorithm - thx wikpedia

	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			LCD_DrawPixel(y0, x0, color);
		} else {
			LCD_DrawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

/**
 * \brief Draws a horizontal line
 *
 * \param x			The x-coordinate of the first point
 * \param y			The y-coordinate of the first point
 * \param length	Length of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int16_t x2;
	
	// Initial off-screen clipping
	if ((length <= 0) || (y < 0) || (y >= m_height) || (x >= m_width) || 
		((x2 = (x + length - 1)) < 0)) return;

	if (x < 0) { // Clip left
		length += x;
		x = 0;
	}

	if (x2 >= m_width) { // Clip right
		x2 = m_width - 1;
		length = x2 - x + 1;
	}

	LCD_SetAddrWindow(x, y, x2, y);
	LCD_Flood(color, length);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);

}

/**
 * \brief Draws a vertical line
 *
 * \param x		The x-coordinate of the first point
 * \param y		The y-coordinate of the first point
 * \param h		High of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int16_t y2;

	// Initial off-screen clipping
	if ((length <= 0) || (x < 0) || (x >= m_width) || (y >= m_height) || 
		((y2 = (y+length-1)) <  0)) return;
		
	if (y < 0) { // Clip top
		length += y;
		y = 0;
	}
	if (y2 >= m_height) { // Clip bottom
		y2 = m_height - 1;
		length = y2 - y + 1;
	}
	LCD_SetAddrWindow(x, y, x, y2);
	LCD_Flood(color, length);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

/**
 * \brief Draws a rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	LCD_DrawFastHLine(x, y, w, color);
	LCD_DrawFastHLine(x, y + h - 1, w, color);
	LCD_DrawFastVLine(x, y, h, color);
	LCD_DrawFastVLine(x + w - 1, y, h, color);
}

/**
 * \brief Draws a filled rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param color			Color
 *
 * \return void
 */
void LCD_FillRect(int16_t x, int16_t y1, int16_t w, int16_t h, uint16_t color) {
	int16_t x2, y2;

	// Initial off-screen clipping
	if ((w <= 0) || (h <= 0) || (x >= m_width) || (y1 >= m_height)
			|| ((x2 = x + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
		return;
	if (x < 0) { // Clip left
		w += x;
		x = 0;
	}
	if (y1 < 0) { // Clip top
		h += y1;
		y1 = 0;
	}
	if (x2 >= m_width) { // Clip right
		x2 = m_width - 1;
		w = x2 - x + 1;
	}
	if (y2 >= m_height) { // Clip bottom
		y2 = m_height - 1;
		h = y2 - y1 + 1;
	}

	LCD_SetAddrWindow(x, y1, x2, y2);
	LCD_Flood(color, (uint32_t) w * (uint32_t) h);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

/**
 * \brief Draws an circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DrawPixel(x0, y0 + r, color);
	LCD_DrawPixel(x0, y0 - r, color);
	LCD_DrawPixel(x0 + r, y0, color);
	LCD_DrawPixel(x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DrawPixel(x0 + x, y0 + y, color);
		LCD_DrawPixel(x0 - x, y0 + y, color);
		LCD_DrawPixel(x0 + x, y0 - y, color);
		LCD_DrawPixel(x0 - x, y0 - y, color);
		LCD_DrawPixel(x0 + y, y0 + x, color);
		LCD_DrawPixel(x0 - y, y0 + x, color);
		LCD_DrawPixel(x0 + y, y0 - x, color);
		LCD_DrawPixel(x0 - y, y0 - x, color);
	}
}

/**
 * \brief Helper function drawing rounded corners
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param color			Color
 *
 * \return void
 */
void LCD_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			LCD_DrawPixel(x0 + x, y0 + y, color);
			LCD_DrawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			LCD_DrawPixel(x0 + x, y0 - y, color);
			LCD_DrawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			LCD_DrawPixel(x0 - y, y0 + x, color);
			LCD_DrawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			LCD_DrawPixel(x0 - y, y0 - x, color);
			LCD_DrawPixel(x0 - x, y0 - y, color);
		}
	}
}

/**
 * \brief Draws a filled circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	LCD_DrawFastVLine(x0, y0 - r, 2 * r + 1, color);
	LCD_FillCircleHelper(x0, y0, r, 3, 0, color);
}

/**
 * \brief Helper function to draw a filled circle
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param delta			Delta
 * \param color			Color
 *
 * \return void
 */
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			LCD_DrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			LCD_DrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

/**
 * \brief Draws a triangle specified by the coordinate pairs.
 *
 * \param x0		The x-coordinate of the first point
 * \param y0		The y-coordinate of the first point
 * \param x1		The x-coordinate of the second point
 * \param y1		The y-coordinate of the second point
 * \param x2		The x-coordinate of the third point
 * \param y2		The y-coordinate of the third point
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	LCD_DrawLine(x0, y0, x1, y1, color);
	LCD_DrawLine(x1, y1, x2, y2, color);
	LCD_DrawLine(x2, y2, x0, y0, color);
}

/**
 * \brief Draws a filled triangle specified by the coordinate pairs.
 *
 * \param x0		The x-coordinate of the first point
 * \param y0		The y-coordinate of the first point
 * \param x1		The x-coordinate of the second point
 * \param y1		The y-coordinate of the second point
 * \param x2		The x-coordinate of the third point
 * \param y2		The y-coordinate of the third point
 * \param color		Color
 *
 * \return void
 */
void LCD_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}
	if (y1 > y2) {
		swap(y2, y1); 
		swap(x2, x1);
	}
	if (y0 > y1) {
		swap(y0, y1); 
		swap(x0, x1);
	}

	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)      a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)      a = x2;
		else if(x2 > b) b = x2;
		LCD_DrawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t	dx01 = x1 - x0,
			dy01 = y1 - y0,
			dx02 = x2 - x0,
			dy02 = y2 - y0,
			dx12 = x2 - x1,
			dy12 = y2 - y1;
	int32_t	sa   = 0,
			sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2) last = y1;   // Include y1 scanline
	else last = y1 - 1; // Skip it

	for(y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) swap(a,b);
		LCD_DrawFastHLine(a, y, b-a+1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) swap(a,b);
		LCD_DrawFastHLine(a, y, b - a + 1, color);
	}
}

/**
 * \brief Draws a rectangle with rounded corners specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	LCD_DrawFastHLine(x + r, y, w - 2 * r, color); // Top
	LCD_DrawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
	LCD_DrawFastVLine(x, y + r, h - 2 * r, color); // Left
	LCD_DrawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
	// draw four corners
	LCD_DrawCircleHelper(x + r, y + r, r, 1, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	LCD_DrawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

/**
 * \brief Draws a filled rounded rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param r				Radius
 * \param fillcolor		Color
 *
 * \return void
 */
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	LCD_FillRect(x + r, y, w - 2 * r, h, color);

	// draw four corners
	LCD_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	LCD_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

/**
 * \brief Draws a character at the specified coordinates
 *
 * \param x			The x-coordinate
 * \param y			The y-coordinate
 * \param c			Character
 * \param color		Character color
 * \param bg		Background color
 * \param size		Character Size
 *
 * \return void
 */
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t fontindex) {
	uint16_t height, width, bytes;
	uint8_t offset;
	uint32_t charindex = 0;
	uint8_t *pchar;
	uint32_t line = 0;

	height = fonts[fontindex]->Height;
	width = fonts[fontindex]->Width;

	if ((x >= m_width) || // Clip right
		(y >= m_height) || // Clip bottom
		((x + width - 1) < 0) || // Clip left
		((y + height - 1) < 0))   // Clip top
		return;

	bytes = (width + 7) / 8;
	if (c < ' ') c = ' ';
#ifndef USE_CP1251
	else if (c > '~') c = ' ';
#endif
	charindex = (c - ' ') * height * bytes;
	offset = 8 * bytes - width;

	for (uint32_t i = 0; i < height; i++) {
		pchar = ((uint8_t *) &fonts[fontindex]->table[charindex] + (width + 7) / 8 * i);
		switch (bytes) {
		case 1:
			line = pchar[0];
			break;
		case 2:
			line = (pchar[0] << 8) | pchar[1];
			break;
		case 3:
		default:
			line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
			break;
		}
		for (uint32_t j = 0; j < width; j++) {
			if (line & (1 << (width - j + offset - 1))) {
				LCD_DrawPixel((x + j), y, color);
			} else {
				LCD_DrawPixel((x + j), y, bg);
			}
		}
		y++;
	}
}

/**
 * \brief Print the specified Text
 *
 * \param fmt	Format text
 * \param
 *
 * \return void
 */
void LCD_Printf(const char *fmt, ...) {
	static char buf[256];
	char *p;
	va_list lst;

	va_start(lst, fmt);
	vsprintf(buf, fmt, lst);
	va_end(lst);
	
	volatile uint16_t height, width;
	height = fonts[m_font]->Height;
	width = fonts[m_font]->Width;

	p = buf;
	while (*p) {
		if (*p == '\n') {
			m_cursor_y += height;
			m_cursor_x = 0;
		} else if (*p == '\r') {
			m_cursor_x = 0;
		} else if (*p == '\t') {
			m_cursor_x += width * 4;
		} else {
#ifdef WIPE_LINES
			if (m_cursor_x == 0) {
				LCD_SetAddrWindow(0, m_cursor_y, m_width - 1, m_cursor_y + height);
				LCD_Flood(m_textbgcolor, (long) m_width * height);
				LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
			}
#endif
			if (m_cursor_y >= (m_height - height)) {
				m_cursor_y = 0;
#ifdef WIPE_SCREEN
				LCD_FillScreen(m_textbgcolor);
#endif
			}
			LCD_DrawChar(m_cursor_x, m_cursor_y, *p, m_textcolor, m_textbgcolor, m_font);
			m_cursor_x += width;
			if (m_wrap && (m_cursor_x > (m_width - width))) {
				m_cursor_y += height;
				m_cursor_x = 0;
			}
		}
		p++;
	}
}

/**
 * \brief Sets the cursor coordinates
 *
 * \param x		The x-coordinate
 * \param y		The y-coordinate
 *
 * \return void
 */
void LCD_SetCursor(uint16_t x, uint16_t y) {
	m_cursor_x = x;
	m_cursor_y = y;
}

/**
 * \brief Sets the text size
 *
 * \param s	Size
 *
 * \return void
 */
void LCD_SetTextSize(int8_t s) {
	if (s < 0) {
		m_font = 0;
	} else if (s >= fontsNum) {
		m_font = fontsNum - 1;
	} else {
		m_font = s;
	}
}

/**
 * \brief Sets the text color
 *
 * \param c		Text color
 * \param b		Background color
 *
 * \return void
 */
void LCD_SetTextColor(uint16_t c, uint16_t b) {
	m_textcolor = c;
	m_textbgcolor = b;
}

/**
 * \brief Set Text wrap
 *
 * \param w
 *
 * \return void
 */
void LCD_SetTextWrap(uint8_t w) {
	m_wrap = w;
}

/**
 * \brief Get display rotation
 *
 * \param
 *
 * \return uint8_t rotation
 */
uint8_t LCD_GetRotation() {
	return m_rotation;
}

/**
 * \brief Gets the cursor x-coordinate
 *
 * \param 		
 *
 * \return int16_t x-coordinate
 */
int16_t LCD_GetCursorX(void) {
	return m_cursor_x;
}

/**
 * \brief Gets the cursor Y-coordinate
 *
 * \param 		
 *
 * \return int16_t y-coordinate
 */
int16_t LCD_GetCursorY(void) {
	return m_cursor_y;
}

/**
 * \brief Calucalte 16Bit-RGB
 *
 * \param r	Red
 * \param g	Green
 * \param b	Blue
 *
 * \return uint16_t	16Bit-RGB
 */
uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

inline uint32_t LCD_Color565_to_888(uint16_t color) {
    return (((color & 0xF800) << 8) | ((color & 0x7E0) << 5) | ((color & 0x1F) << 3));  // transform to rrrrrrxx ggggggxx bbbbbbxx
}

inline uint8_t LCD_Color565_to_R(uint16_t color) {
    return ((color & 0xF800) >> 8);  // transform to rrrrrrxx
}
inline uint8_t LCD_Color565_to_G(uint16_t color) {
    return ((color & 0x7E0) >> 3);  // transform to ggggggxx
}
inline uint8_t LCD_Color565_to_B(uint16_t color) {
    return ((color & 0x1F) << 3);  // transform to bbbbbbxx
}

/*
 * lcd_touch.c
 
 该文件说明原始程序所用的TFT板子和现在用的板子在实际接线方面的差异
 需要按以下关系对应修改
注：A板指arduino板对应接口，N板指nucleo板对应的原始管脚编号

      原始板                                修改板
    A板  N板  所用资源                  A板  N板  所用资源
Y+  A2  PA4   ADC1_IN4,EXIT4     YP     A3   PB0   ADC2_IN8, EXIT0
Y-  D6  PB10   IO                YM     D9   PC7     IO

X+  D7  PA8    IO                XP     D8   PA9     IO
X-  A1  PA1   ADC2_IN1           XM     A2   PA4    ADC1_IN4
*/

#include <stdlib.h>
#include "lcd_touch.h"

#define ADC_NO_TOUCH_X_OUTSIDE (4095 - 100)
#define TOUCH_ADC_X_MAX 3460     //该值可以将touchX(),touchY()得到的值用串口打印出来，然后再确定。
#define TOUCH_ADC_X_MIN 500
#define TOUCH_ADC_Y_MIN 530
#define TOUCH_ADC_Y_MAX 3660

static const float ADC_UNIT_PX_X = 1.0 / (TOUCH_ADC_X_MAX - TOUCH_ADC_X_MIN);
static const float ADC_UNIT_PX_Y = 1.0 / (TOUCH_ADC_Y_MAX - TOUCH_ADC_Y_MIN);

static ADC_HandleTypeDef* hadcX = NULL;
static ADC_HandleTypeDef* hadcY = NULL;
static uint32_t ADC_ChannelX;
static uint32_t ADC_ChannelY;
static LCD_TouchState m_touch_state = LCD_TOUCH_IDLE;
static LCD_TouchPoint* m_last_point_ref = NULL;

static float fclamp(float x, float l, float u) {
	return x < l ? l : (x > u ? u : x);
}

static float adc_norm_x(uint32_t x) {
	return (x - TOUCH_ADC_X_MIN) * ADC_UNIT_PX_X;
}

static float adc_norm_y(uint32_t y) {
	return (y - TOUCH_ADC_Y_MIN) * ADC_UNIT_PX_Y;
}

static uint32_t ADC_GetValue(ADC_HandleTypeDef* hadc, uint32_t channel) {
	uint32_t value;
    ADC_ChannelConfTypeDef sConfig;

	sConfig.Channel = channel;
	sConfig.Rank = 1;
	//sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	HAL_ADC_ConfigChannel(hadc, &sConfig);
	// start conversion
	HAL_ADC_Start(hadc);
   // wait until finish
	HAL_ADC_PollForConversion(hadc, 100);
	value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);
   return  value;
}

static void GPIO_SetPinMode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
		uint32_t GPIO_PinMode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_PinMode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void ADC_GPIOA_init(uint16_t GPIO_Pin) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
static void ADC_GPIOB_init(uint16_t GPIO_Pin) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


static uint32_t touchX(void) {
	GPIO_SetPinMode(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOC, GPIO_PIN_7, GPIO_MODE_INPUT);
	ADC_GPIOB_init(GPIO_PIN_0);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

	return ADC_GetValue(hadcX, ADC_ChannelX);
}

static uint32_t touchY(void) {
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);   //用串口打印时要禁用外部中断，否则可能会死机
	GPIO_SetPinMode(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOB, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_9, GPIO_MODE_INPUT);
	ADC_GPIOA_init(GPIO_PIN_4);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

	uint32_t adc_y = ADC_GetValue(hadcY, ADC_ChannelY);

	//FIXME: after enabling EXTI4 TOUCH_DOWN interrupt is generated
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	return adc_y;
}

/**
 * Saves ADC handles references to measure touch screen positions.
      保存ADC引用来测量处理触摸屏位置。
 */
void LCD_Touch_Init(ADC_HandleTypeDef* aHadcX, uint32_t aADC_ChannelX,
		ADC_HandleTypeDef* aHadcY, uint32_t aADC_ChannelY) {
	hadcX = aHadcX;
	hadcY = aHadcY;
	ADC_ChannelX = aADC_ChannelX;
	ADC_ChannelY = aADC_ChannelY;
}

static void GPIO_DrawMode() {    //将触摸功能所使用的管脚重新配置为绘图模式
	/* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	/*Configure GPIO pins: PA4 PA9 */
	GPIO_InitStruct.Pin =  GPIO_PIN_4 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO data pin PB0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*Configure GPIO data pin PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

static void GPIO_InterruptMode() {    //中断模式,即进入触摸检测模式
    __HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	/* X- PA1 */    /* Xm  PA4 */  
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	/* X+ PA8 */     /* Xp PA9 */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Y- PB10 */      /* Ym PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Y+ PA4 */     /* YP  PB0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * Set LCD's mode to either DRAW or TOUCH.
 * Set LCD_Mode to DRAW to draw anything with LCD, then switch back to TOUCH,
 * if you want to receive touches.
 */
HAL_StatusTypeDef LCD_SetMode(LCD_Mode mode) {
	switch (mode) {
	case LCD_MODE_TOUCH:
		GPIO_InterruptMode();
		return HAL_OK;

	case LCD_MODE_DRAW:
		GPIO_DrawMode();
		return HAL_OK;

	default:
		return HAL_ERROR;
	}
}

/*
 * Reads raw touch x- and y-positions and stores them in the LCD_TouchPoint point.
 */
LCD_TouchReadState LCD_Touch_Read(LCD_TouchPoint* p) {
	if (hadcX == NULL || hadcY == NULL) {
		return LCD_TOUCH_READ_NOT_INITIALIZED;
	}
	if (m_touch_state == LCD_TOUCH_IDLE) {
		return LCD_TOUCH_READ_NO_TOUCH;
	}
	uint32_t x = touchX();

    for(int i=0;i<9;i++) 
    {
    TXBuff[i]=TXBuff[i+1];  
    }
     TXBuff[9]=x;
	if (x > ADC_NO_TOUCH_X_OUTSIDE) {
		return LCD_TOUCH_READ_OUTSIDE;
	}

	uint32_t y = touchY();
  for(int i=0;i<9;i++) 
    {
    TYBuff[i]=TYBuff[i+1];  
    }
     TYBuff[9]=y;
    x=(TXBuff[0]+TXBuff[1]+TXBuff[2]+TXBuff[3]+TXBuff[4]+TXBuff[5]+TXBuff[6]+TXBuff[7]+TXBuff[8]+TXBuff[9])/10; //使用了一个简单的平均数滤波
    y=(TYBuff[0]+TYBuff[1]+TYBuff[2]+TYBuff[3]+TYBuff[4]+TYBuff[5]+TYBuff[6]+TYBuff[7]+TYBuff[8]+TYBuff[9])/10;
	p->x = (int16_t) ((1 - fclamp(adc_norm_x(x), 0.0f, 1.0f)) * TFTWIDTH);
	p->y = (int16_t) ((1 - fclamp(adc_norm_y(y), 0.0f, 1.0f)) * TFTHEIGHT);
	p->tick = HAL_GetTick();
	p->state = m_touch_state;

	m_last_point_ref = p;
	m_touch_state = LCD_TOUCH_MOVE;

	return LCD_TOUCH_READ_SUCCESS;
}

void LCD_Touch_OnDown() {
	if (m_touch_state == LCD_TOUCH_IDLE) {
		m_touch_state = LCD_TOUCH_DOWN;
	}
}

void LCD_Touch_OnUp() {
	m_touch_state = LCD_TOUCH_IDLE;
	m_last_point_ref->state = LCD_TOUCH_UP;
	LCD_Touch_Draw_OnUp();
}

LCD_TouchState LCD_Touch_GetState() {
	return m_touch_state;
}

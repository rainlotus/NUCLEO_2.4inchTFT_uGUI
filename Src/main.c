/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
该例程结合了2.4寸TFT屏（ILI9341）和μGUI（一种极简的GUI）库。
μGUI主要包含三个文件：ugui.c ,ugui.h, ugui_config.h
μGUI: https://github.com/achimdoebler/UGUI

*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "lcd_touch.h"
#include "math.h"
#include "ugui.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void drawSin(uint16_t color);
UG_GUI gui;   //定义UG_GUI结构型变量
#define MAX_OBJECTS 10
void window1callback(UG_MESSAGE* msg);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  
    UG_Init(&gui, UserPixelSetFunction, 240, 320);
 	LCD_Init();
    LCD_FillScreen(BLACK);
    
    UG_FillCircle(120, 160, 50, C_YELLOW_GREEN);
	UG_FillCircle(120, 160, 45, C_CADET_BLUE);
	UG_FontSelect(&FONT_12X20);
	UG_PutString(80, 220, "hello!");
    HAL_Delay(2000);
    UG_WINDOW window1;

	UG_BUTTON button1;
	UG_BUTTON button2;
	UG_BUTTON button3;
	UG_TEXTBOX textbox1;

	UG_OBJECT objbuffwnd1[MAX_OBJECTS];

	UG_WindowCreate(&window1, objbuffwnd1, MAX_OBJECTS, window1callback);

	UG_WindowSetTitleText(&window1, "uGUI Demo Window");
	UG_WindowSetTitleTextFont(&window1, &FONT_12X20);
    UG_ButtonCreate(&window1, &button1, BTN_ID_0, 10, 10, 160, 60);
	UG_ButtonCreate(&window1, &button2, BTN_ID_1, 10, 80, 160, 130);
	UG_ButtonCreate(&window1, &button3, BTN_ID_2, 10, 150,160, 200);
	UG_ButtonSetFont(&window1, BTN_ID_0, &FONT_12X20);
	UG_ButtonSetForeColor(&window1, BTN_ID_0, C_RED);
	UG_ButtonSetText(&window1, BTN_ID_0, "Button A");
	UG_ButtonSetFont(&window1, BTN_ID_1, &FONT_12X20);
	UG_ButtonSetText(&window1, BTN_ID_1, "Button B");
	UG_ButtonSetFont(&window1, BTN_ID_2, &FONT_12X20);
	UG_ButtonSetText(&window1, BTN_ID_2, "Button C");
    
    UG_TextboxCreate(&window1, &textbox1, TXB_ID_0, 10,220, 220, 260);
	UG_TextboxSetFont(&window1, TXB_ID_0, &FONT_12X20);
	UG_TextboxSetText(&window1, TXB_ID_0, "Test");
	UG_TextboxSetForeColor(&window1, TXB_ID_0, C_BLACK);
	UG_TextboxSetBackColor(&window1, TXB_ID_0, C_LIGHT_GRAY);
	UG_TextboxSetAlignment(&window1, TXB_ID_0, ALIGN_CENTER);

	UG_WindowShow(&window1);

    
    
//	LCD_FillScreen(BLACK);
//	LCD_SetTextSize(3);
//	LCD_SetTextColor(RED, BLACK);
//    LCD_SetRotation(1);//1，3横屏，0，2竖屏， 之间相差180度
//	//LCD_FillRoundRect(20, 40, 120, 120, 15, BLUE);
//    LCD_Printf("hello world.");
//    drawSin(RED);
//    HAL_Delay(5000);
//    LCD_FillScreen(BLACK); 
//	LCD_Touch_Init(&hadc2, ADC_CHANNEL_4, &hadc1, ADC_CHANNEL_1);
//	LCD_SetMode(LCD_MODE_TOUCH);
//	LCD_TouchPoint p;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//  if (LCD_Touch_Read(&p) == LCD_TOUCH_READ_SUCCESS) {
//		  LCD_Touch_Draw_ConnectLastPoint(&p);
//		  LCD_Touch_Draw_PrintInfo();
//	  }
//	  LCD_Touch_Draw_Update();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	// HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
	//  HAL_Delay(300);
      UG_Update();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void drawSin(uint16_t color)
{
  LCD_DrawFastHLine(0,120,320,WHITE);
    LCD_DrawFastVLine(0,0,240,WHITE);
  for(int i=0;i<360;i++) LCD_DrawLine(i,60*sin((i*6)*3.14/360)+120,i+1,60*sin(((i+1)*6)*3.14/360)+120,color);

}
//窗口回调函数
void window1callback(UG_MESSAGE* msg)
{
	if(msg->type == MSG_TYPE_OBJECT)
	{
		if(msg->id == OBJ_TYPE_BUTTON)
		{
			switch(msg->sub_id)
			{
				case BTN_ID_0 :
				{
					// . . .
					break;
				}
				case BTN_ID_1:
				{
					// . . .
					break;
				}
				case BTN_ID_2:
				{
					// . . .
					break;
				}
			}
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

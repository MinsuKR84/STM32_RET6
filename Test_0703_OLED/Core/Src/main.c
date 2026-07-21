/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_font.h"
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

#define SSD1306_I2C 

/* 버튼 핀 정의 */
#define BTN_1F_PORT      GPIOA
#define BTN_1F_PIN       GPIO_PIN_0

#define BTN_2F_PORT      GPIOA
#define BTN_2F_PIN       GPIO_PIN_1

#define BTN_3F_PORT      GPIOA
#define BTN_3F_PIN       GPIO_PIN_7

#define BTN_OPEN_PORT    GPIOA
#define BTN_OPEN_PIN     GPIO_PIN_8

#define BTN_CLOSE_PORT   GPIOA
#define BTN_CLOSE_PIN    GPIO_PIN_4

#define BTN_UP_PORT      GPIOA
#define BTN_UP_PIN       GPIO_PIN_5

#define BTN_DOWN_PORT    GPIOA
#define BTN_DOWN_PIN     GPIO_PIN_6

#define ARROW_DOWN       0
#define ARROW_UP         1

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t Button_IsPressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void OLED_ShowMessage(char *line1, char *line2);
void OLED_ShowArrowMessage(char *line1, char *line2, uint8_t direction);
void OLED_ShowArrowMessageAt(char *line1, char *line2, uint8_t direction, int8_t arrow_y);
void OLED_DrawArrow(uint8_t x, int8_t y, uint8_t direction);
void OLED_DrawClippedPixel(int16_t x, int16_t y);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t Button_IsPressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET)
    {
        HAL_Delay(20);   // 디바운싱

        if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET)
        {
            return 1;
        }
    }

    return 0;
}

void OLED_ShowMessage(char *line1, char *line2)
{
    ssd1306_black_screen();

    ssd1306_set_cursor(0, 0);
    ssd1306_write_string(font7x10, line1);

    ssd1306_set_cursor(0, 28);
    ssd1306_write_string(font11x18, line2);

    ssd1306_update_screen();
}

void OLED_ShowArrowMessage(char *line1, char *line2, uint8_t direction)
{
    OLED_ShowArrowMessageAt(line1, line2, direction, 20);
}

void OLED_ShowArrowMessageAt(char *line1, char *line2, uint8_t direction, int8_t arrow_y)
{
    ssd1306_black_screen();

    ssd1306_set_cursor(0, 0);
    ssd1306_write_string(font7x10, line1);

    ssd1306_set_cursor(0, 28);
    ssd1306_write_string(font11x18, line2);

    OLED_DrawArrow(96, arrow_y, direction);
    ssd1306_update_screen();
}

void OLED_DrawClippedPixel(int16_t x, int16_t y)
{
    if ((x >= 0) && (x < SSD1306_WIDTH) && (y >= 0) && (y < SSD1306_HEIGHT))
    {
        ssd1306_white_pixel((uint8_t)x, (uint8_t)y);
    }
}

void OLED_DrawArrow(uint8_t x, int8_t y, uint8_t direction)
{
    int16_t center = (int16_t)(x + 12);

    if (direction == ARROW_UP)
    {
        for (uint8_t row = 0; row < 12; row++)
        {
            uint8_t half_width = row;
            for (uint8_t col = 0; col <= (half_width * 2); col++)
            {
                OLED_DrawClippedPixel((int16_t)(center - half_width + col), (int16_t)(y + row));
            }
        }

        for (uint8_t row = 12; row < 36; row++)
        {
            for (uint8_t col = 0; col < 7; col++)
            {
                OLED_DrawClippedPixel((int16_t)(center - 3 + col), (int16_t)(y + row));
            }
        }
    }
    else
    {
        for (uint8_t row = 0; row < 24; row++)
        {
            for (uint8_t col = 0; col < 7; col++)
            {
                OLED_DrawClippedPixel((int16_t)(center - 3 + col), (int16_t)(y + row));
            }
        }

        for (uint8_t row = 0; row < 12; row++)
        {
            uint8_t half_width = (uint8_t)(11 - row);
            for (uint8_t col = 0; col <= (half_width * 2); col++)
            {
                OLED_DrawClippedPixel((int16_t)(center - half_width + col), (int16_t)(y + 24 + row));
            }
        }
    }
}

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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_init();

  OLED_ShowMessage("ELEVATOR", "READY");

  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (Button_IsPressed(BTN_1F_PORT, BTN_1F_PIN))
    {
        OLED_ShowMessage("BUTTON", "1 FLOOR");

        while (HAL_GPIO_ReadPin(BTN_1F_PORT, BTN_1F_PIN) == GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_2F_PORT, BTN_2F_PIN))
    {
        OLED_ShowMessage("BUTTON", "2 FLOOR");

        while (HAL_GPIO_ReadPin(BTN_2F_PORT, BTN_2F_PIN) == GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_3F_PORT, BTN_3F_PIN))
    {
        OLED_ShowMessage("BUTTON", "3 FLOOR");

        while (HAL_GPIO_ReadPin(BTN_3F_PORT, BTN_3F_PIN) == GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_OPEN_PORT, BTN_OPEN_PIN))
    {
        OLED_ShowMessage("DOOR", "OPEN");

        while (HAL_GPIO_ReadPin(BTN_OPEN_PORT, BTN_OPEN_PIN) == GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_CLOSE_PORT, BTN_CLOSE_PIN))
    {
        OLED_ShowMessage("DOOR", "CLOSE");

        while (HAL_GPIO_ReadPin(BTN_CLOSE_PORT, BTN_CLOSE_PIN) == GPIO_PIN_RESET);
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_UP_PORT, BTN_UP_PIN))
    {
        int8_t arrow_y[] = {44, 36, 28, 20, 12, 4, -4, -12, -20};
        uint8_t frame = 0;

        while (HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN) == GPIO_PIN_RESET)
        {
            OLED_ShowArrowMessageAt("MANUAL", "UP", ARROW_UP, arrow_y[frame]);
            frame = (uint8_t)((frame + 1) % 9);
            HAL_Delay(10);
        }
        HAL_Delay(100);
    }

    else if (Button_IsPressed(BTN_DOWN_PORT, BTN_DOWN_PIN))
    {
        int8_t arrow_y[] = {-20, -12, -4, 4, 12, 20, 28, 36, 44};
        uint8_t frame = 0;

        while (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET)
        {
            OLED_ShowArrowMessageAt("MANUAL", "DOWN", ARROW_DOWN, arrow_y[frame]);
            frame = (uint8_t)((frame + 1) % 9);
            HAL_Delay(10);
        }
        HAL_Delay(100);
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

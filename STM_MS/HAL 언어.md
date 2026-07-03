HAL 언어

<GPIO>
GPIO_PinState     HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void              HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void              HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void              HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin);
void              HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


<EXI>
// 인터럽트 사용 시 pin을 설정해야함 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  if(GPIO_Pin == GPIO_PIN_0)
  {
    flag = 1;
  }
  else if(GPIO_Pin == GPIO_PIN_1)  
  {
    flag = 2;
  }

PLL(Phase Locked Loop) = 기준 클럭을 배수시켜 더 빠른 클럭을 만드는 장치	

<타이머 주기 설정>
타이머 주기 = (PSC + 1) × (ARR + 1) / Timer Clock

PSC = 분주비( < 2^16)
ARR = 자동재장전 값, 끝 값
CNT = 현재 카운터 값

// 타이머 폴링방식 = 주기가 끝나면 우리가 자체적으로 끝나는 지점을 설정해야함
HAL_TIM_Base_Start(&htim2);

while (1)
{
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
    {
        if (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
}

// 타이머 인터럽트 방식 = 주기가 끝나면 자동적으로 인터럽트가 들어가 실행시켜줌
HAL_TIM_Base_Start_IT(&htim2); // &htim숫자

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) // TIM숫자
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}


<PWM>
Duty(%) = Pulse / (ARR + 1) × 100
cubemx 세팅 pulse값은 = 초기값 => 볼려면 맨앞에 딜레이 넣어줘야함(원래는 잘 안보임)
코드로 pulse값 조절 가능

// ARR = 99일경우
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 25);   // 25%
HAL_Delay(500);
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);   // 50%
HAL_Delay(500);
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 75);   // 75%
HAL_Delay(500);

<timer2>
HAL_GetTick() - 부팅 후 지난 시간을 ms 단위로 줌

	
<I2C 파형 순서>
(start) 101 0101 0 0(ACK) 0100 0101 0(ACK) (stop)
   0   (주소 7bit)(W)       (data - 8bit)           0 

<OLED 라이브러리>
https://github.com/mokhwasomssi/stm32_hal_ssd1306
- 주소

ssd1306.c
ssd1306.h
ssd1306_font.c
ssd1306_font.h

사용해야함


1.  UART 통신 (TX와 RX 교차)
- 클럭이 없는(별도의 클럭선을 연결하지 않는) 비동기 직렬 통신 

- 필요한 선 - TX(Transfer), RX(Receive), GND

<설정> 
- Baud rate - 115200 baud
- Data bits(데이터 비트 수) - 8 data bits
- Parity(패리티) - no parity
- Stop bits(정지 비트 수) - 1 stop bit
 
 -> 115200, 8-N-1

<보드 & PC전용> 
- PA2 = USART2_TX (선택)
- PA3 = USART2_RX (선택)

```
// 실제 배열 마지막에는 컴파일러가 `\0`을 자동으로 추가
uint8_t msg[] = "Hello UART\r\n";

while (1)
{
    HAL_UART_Transmit(
        &huart2, // USART2 설정 정보를 가진 구조체의 주소(UARTx의미)
        msg, // 전송할 데이터가 저장된 배열의 시작 주소
        sizeof(msg) - 1, // 배열 전체 크기에서 마지막 `\0` 한 바이트를 제외한 길이 , `sizeof()` 자체가 `\0`을 빼지 않음
        HAL_MAX_DELAY // 전송이 완료될 때까지 사실상 제한 없이 기다림, 통신 문제가 발생하면 오래 멈출 수 있어 timeout 값을 넣어줘도 됨
    );

    HAL_Delay(1000);
}
```

<보드 & 보드>
- 보드 A PB10 TX ──── 보드 B PB11 RX
- 보드 A PB11 RX ──── 보드 B PB10 TX
- 보드 A GND     ──── 보드 B GND

```
uint8_t tx_data = 'A'; // 송신할 바이트 하나를 저장할 변수

while (1)
{
    HAL_UART_Transmit(
        &huart3,
        &tx_data, // 보낼 데이터 저장할 주소
        1, // 한 바이트 수신
        HAL_MAX_DELAY
    );

    HAL_Delay(1000);
}
```
```
uint8_t rx_data ; // 수신한 바이트 하나를 저장할 변수

while (1)
{
    HAL_UART_Transmit(
        &huart3,
        &rx_data, // // 받을 데이터 저장할 주소
        1,
        HAL_MAX_DELAY
    );

    HAL_Delay(1000);
}
```

<파형>

![](../../STM_image/SCR03%201.png)

- 문자 `'A'`는 다음 값입니다.

```
'A' = 0x41 = 0100 0001
```

일반적인 UART는 데이터 비트를 **LSB부터** 보냅니다.

```
0x41의 비트
D7 D6 D5 D4 D3 D2 D1 D0
 0  1  0  0  0  0  0  1

실제 전송 순서
D0 D1 D2 D3 D4 D5 D6 D7
 1  0  0  0  0  0  1  0
```

전체 프레임은 다음과 같습니다.

```
Idle → Start → 데이터 8비트 → Stop → Idle
High    Low                     High
```
아래 처럼 보인다
```
Idle  Start   D0 D1 D2 D3 D4 D5 D6 D7  Stop
High    0      1  0  0  0  0  0  1  0    1

``` 
※ UART는 평상시에 TX가 High이고, Start bit에서 Low로 내려가면서 수신기에게 “지금부터 데이터가 시작된다”고 알림

 <비트 시간 계산>
 
- Baud기준 한 비트 시간
1 / 115200 ≈ 8.68µs
```
- 8-N-1 프레임은 총 10비트
- 문자 하나의 전송 시간은 약
```
10 × 8.68µs ≈ 86.8µs


<`strlen()`과 `sizeof()` 차이>
- `strlen(msg)` = 문자열의 `\0` 전까지 계산
- `sizeof(msg) -1` = 전체 배열 크기에서 문자열 종료문자 `\0` 한 바이트를 뺌
  
char msg[] = "ABC";

'A' 'B' 'C' '\0' 
 1   1   1    1 byte
 
 strlen(msg) = 3
 sizeof(msg) = 4 / sizeof(msg) -1 = 3

> TX/RX를 교차하고 설정만 동일하면 비교적 쉽게 통신된다. 오실로스코프에서는 Idle High, Start Low, LSB First 데이터를 확인

- polling interrupt 차이

  
  ## UART Prescaler

UART도 내부적으로 주변장치 클럭을 나누어 원하는 Baud rate를 생성합니다. 다만 보통 사용자는 CubeMX에 115200 같은 Baud rate를 입력하고 HAL이 관련 레지스터 값을 계산합니다.
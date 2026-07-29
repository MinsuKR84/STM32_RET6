3. SPI 통신 (MOSI끼리, MISO끼리, SCK끼리, NSS끼리)
-  SPI는 클럭을 사용하는 동기식 직렬통신이며, Master가 통신을 시작
-  필요한 선 - SCK(Master가 만드는 Clock) , MOSI(Master Out, Slave In) , MISO(Master In, Slave Out) , NSS/CS(통신할 Slave 선택)
- SPI는 송신과 수신이 동시에 진행(내부에 시프트 레지스터가 있음)

<보드 & 보드>

-  Master SCK  ───── Slave SCK
- Master MOSI ───── Slave MOSI
- Master MISO ───── Slave MISO
- Master NSS  ───── Slave NSS
- Master GND  ───── Slave GND


- Slave 혼자서는 SCK 파형이 생성되지 않고, SPI의 SCK는 **Master만 생성**하기 때문이다. 따라서 Slave 코드만 실행한 상태에서 SCK가 안 나오는 것은 정상. 상태는
```
Master가 NSS를 활성화하고 SCK를 공급할 때까지 대기
```

## MASTER
```
uint8_t tx_data = 'A'; // 보낼 데이터
uint8_t rx_data = 0; // 동시에 수신할 데이터

while (1)
{
    HAL_SPI_TransmitReceive(
        &hspi1, // SPI1 사용
        &tx_data, // 송신 버퍼
        &rx_data, // 수신 버퍼
        1, // 한 바이트 교환
        HAL_MAX_DELAY // 완료까지 대기
    );

    HAL_Delay(1000);
}
```
## SLAVE
```
uint8_t tx_data = 0;
uint8_t rx_data = 0;

while (1)
{
    if (HAL_SPI_TransmitReceive(
            &hspi1,
            &tx_data,
            &rx_data,
            1,
            HAL_MAX_DELAY) == HAL_OK)
    {
        if (rx_data == 'A')
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
    }
}
```

<파형>
1차: SCK + MOSI
2차: NSS + MOSI
3차: SCK + MISO

'A' = 0x41 = 0100 0001
SPI를 MSB First로 설정했다면 MOSI 상태는
```
0 → 1 → 0 → 0 → 0 → 0 → 0 → 1
```

- UART와 다르게 SPI는 설정에 따라 MSB 또는 LSB First를 선택할 수 있습니다.

## CS를 GPIO로 직접 제어하는 경우

<NSS 의미>
- Active Low를 뜻함
```
NSS High → Slave 선택 안 됨
NSS Low  → Slave 선택됨
```
-> 따라서 NSS가 계속 High인 상태라면 통신이 시작되지 않음

```
HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
```

- CS를 Low로 만들어 Slave를 선택합니다.

```
HAL_SPI_TransmitReceive(...);
```

- 실제 데이터 교환을 진행합니다.

```
HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
```

- CS를 High로 만들어 통신 종료를 알립니다.
- 
- Master가 전송할 때 정상 흐름
```
NSS Low
→ SCK 8펄스
→ MOSI/MISO 데이터 교환
→ NSS High
```

<CPOL 과 CPHA>

## CPOL

- 클럭이 전송이 없는 유휴 상태에서 SCK가 0(LOW)인지 1(HIGH)인지 정의 (클럭의 유휴 상태를 결정)
```
CPOL=0 → SCK idle Low
CPOL=1 → SCK idle High
```
## CPHA

- 데이터가 클럭의 첫 번째 에지에서 샘플링되는지(0) 두 번째 에지에서 샘플링되는지(1)를 정함 (몇 번째 엣지에서 데이터를 샘플링하는지 결정)
```
CPHA=0 → 첫 번째 엣지에서 샘플
CPHA=1 → 두 번째 엣지에서 샘플
```
※ Master와 Slave의 CPOL/CPHA가 다르면 데이터가 깨짐

| 모드  | CPOL | CPHA | 유휴 클럭 | 데이터 샘플링 |
| --- | ---- | ---- | ----- | ------- |
| 0   | 0    | 0    | LOW   | 상승 에지   |
| 1   | 0    | 1    | LOW   | 하강 에지   |
| 2   | 1    | 0    | HIGH  | 하강 에지   |
| 3   | 1    | 1    | HIGH  | 상승 에지   |
- Prescaler가 2일 때 수 MHz가 나오면 화면에서 보기 어려움(16,32로 맞춰야함)

> Master만 SCK를 생성한다. Slave 코드만 실행하면 파형이 나오지 않는다. NSS가 Low인 동안 CPOL/CPHA에 맞춰 MOSI와 MISO를 동시에 교환
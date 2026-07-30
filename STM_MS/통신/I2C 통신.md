2. I2C 통신 (SDA끼리, SCL끼리)

- Intergreted Two-circuit 으로 두개의 선을 사용하는 동기식 직렬 통신
- Controller가 통신을 시작하고 SCL을 생성하고, 통신 대상은 주소를 통해 선택함. I2C는 기본적으로 7비트 주소를 사용하며, 각 바이트 뒤에는 ACK/NACK 비트가 추가됨

- 필요한 선 - SCL(Serial Clock Line),SDA(Serial Data Line),GND

<특징> 

- 여러 장치를 하나의 버스에 연결할 수 있고, 각 Slave는 주소를 가짐
```
Master → START → 7비트 주소 → R/W 1비트 → ACK → 데이터 8비트 → ACK/NACK → STOP
```
 - Open-Drain 방식이므로 풀업저항이 필요
 - 장치는 선을 직접 High로 만들지 않고, Low로만 당길 수 있음
```
장치가 Low 출력 → 선이 0V
아무도 Low로 당기지 않음 → 풀업저항이 High로 올림
```

<보드 & LCD>

- STM32 PB8 SCL ───── LCD SCL
- STM32 PB9 SDA ───── LCD SDA
- STM32 GND     ───── LCD GND
- STM32/LCD 전원 연결

## 주소 찾기
```
char msg[64];

while (1)
{
    uint8_t found = 0;

	// I2C의 가능한 7비트 주소를 하나씩 검사(0x00~0x7F 사이)
    for (uint8_t address = 1; address < 128; address++)
    {
        if (HAL_I2C_IsDeviceReady(
                &hi2c1, // I2C1 주변 장치 사용
                address << 1, // 7비트 주소를 한 칸 왼쪽으로 이동(주소가 R/W 비트 자리까지 포함될 수 있도록 왼쪽 정렬된 형태를 요구)
                2, // 응답 확인할 재시도 횟수
                20 // Timeout 시간
				) == HAL_OK) // 해당 주소의 장치가 ACK를 반환했다는 뜻
        {
            found = 1;

            snprintf(
                msg,
                sizeof(msg),
                "Found: 0x%02X\r\n",
                address
            );

            HAL_UART_Transmit(
                &huart2,
                (uint8_t *)msg,
                strlen(msg),
                HAL_MAX_DELAY
            );
        }
    }

    if (!found)
    {
        HAL_UART_Transmit(
            &huart2,
            (uint8_t *)"No I2C device found\r\n",
            21,
            HAL_MAX_DELAY
        );
    }

    HAL_Delay(2000);
}
```
-> HAL에는 일반적으로 7비트 주소를 왼쪽으로 한 칸 이동해서 전달합니다.
## I2C 송신 코드

```
uint8_t data = 0x55;
```
100 01010
- `0x55` 한 바이트를 준비합니다.

```
HAL_I2C_Master_Transmit(&hi2c1,
                        0x27 << 1,
                        &data,
                        1,
                        100);
```

각 부분은 다음과 같습니다.

- `&hi2c1`: I2C1 사용
- `0x27 << 1`: 대상 장치의 7비트 주소
- `&data`: 전송할 데이터 주소
- `1`: 한 바이트 전송
- `100`: 최대 대기 시간

HAL이 내부적으로 다음 과정을 수행합니다.

```
START
→ 주소 0x27 + Write
→ 상대 ACK 확인
→ 0x55 전송
→ 상대 ACK 확인
→ STOP
```

주소가 0x12 이고 Write라면
```
7비트 주소: 0010010
R/W:        0
```
- 그다음 9번째 SCL 펄스에서 Slave가 ACK를 보냄

## ACK

- 9번째 클럭 동안 SDA가 Low:
```
ACK = 0
```

## NACK

- 9번째 클럭 동안 SDA가 High:
```
NACK = 1
```


 <주기 계산>
- 100kHz I2C라면 SCL 한 주기는 약:
```
1 / 100000 = 10µs
```

<I2C START와 STOP>

- START와 STOP은 별도의 SCL 한 주기가 아니다

## START

- SCL이 High인 동안 SDA가 High → Low 로 변하면 START

## STOP

- SCL이 High인 동안 SDA가 Low → High 로 변하면 STOP

일반 데이터 전송 중에는 SDA가 SCL Low 구간에서 변경되고, SCL High 구간에서는 안정되어 있어야 합니다.

즉 START/STOP은 **SCL이 High인 상태에서 SDA가 변하는 특별한 조건**

### I2C 풀업저항

신호를 High로 복귀시키기 위한 저항입니다.

<파형>
![](../../STM_image/SCR06.png)
## 첫 번째 바이트: 주소 `0x55 + Write`

일반적인 I2C 주소는 7비트입니다.

```
A6 A5 A4 A3 A2 A1 A0
```

7비트 주소:

```
0x55 = 1010101
```

Master가 Slave에 데이터를 쓰는 것이므로 R/W 비트는 `0`입니다.

```
일반적인 I2C 주소는 7비트입니다.

```

``` 
A6 A5 A4 A3 A2 A1 A0   R/W
1  0  1  0  1  0  1     0
```

따라서 버스에서 실제 전송되는 8비트는:

```
10101010 = 0xAA
```

입니다.

※ R/W 비트란 무엇인가?

`R/W`는 데이터 방향을 나타냅니다.

```
R/W = 0 → Write
Controller가 Target에 데이터를 보냄
(0x12 << 1) | '0' 로 표시 = 0xAA

R/W = 1 → Read
Controller가 Target에서 데이터를 읽음
(0x12 << 1) | '1' 로 표시 = 0xAB
```

그다음 9번째 클럭에서 Slave가 ACK를 보냅니다.

```
SCL 펄스:  1 2 3 4 5 6 7 8 | 9
SDA 데이터: 1 0 1 0 1 0 1 0 | 0
             주소 + Write     ACK
```

※ Controller가 주소를 보낸 경우

1. Controller가 주소 7비트와 R/W 비트를 보냅니다.
2. 총 8비트가 끝나면 Controller는 SDA를 놓습니다.
3. 주소가 일치하는 Target이 SDA를 Low로 당깁니다.
4. Controller가 9번째 SCL High에서 SDA를 읽습니다.

```
9번째 클럭에서 SDA Low → ACK
9번째 클럭에서 SDA High → NACK
```
## NACK가 나오는 경우

주소 전송 후 NACK:

- 해당 주소 장치가 없음
- 장치 전원이 꺼짐
- SDA/SCL 배선 문제
- 장치가 응답할 수 없는 상태

데이터 전송 중 NACK:

- Target이 더 이상 데이터를 받을 수 없음
- 전송 종료 요청
- 통신 오류


---

## 두 번째 바이트: 문자 `'E'`

문자 `'E'`의 ASCII 값은:

```
'E' = 0x45
```

이진수로는:

```
0x45 = 01000101
```

따라서 파형에서는 다음 순서로 보입니다.

```
SCL 펄스:  1 2 3 4 5 6 7 8 | 9
SDA 데이터: 0 1 0 0 0 1 0 1 | 0
                 0x45         ACK
```
```
SDA/SCL → 3.3V
```

        START      주소 0xAA        ACK      데이터 0x45      ACK    STOP
SCL  ─────┐   _-_-_-_-_-_-_-_-_   _-_   _-_-_-_-_-_-_-_-_   _-_   ─────
          │

SDA  ─────┘   1 0 1 0 1 0 1 0      0    0 1 0 0 0 1 0 1      0    └─────

## SCL 상승 에지와 하강 에지 중 어디를 봐야 하나?

데이터 판독은 기본적으로 SCL 상승 에지 기준으로 보면 됩니다.

I2C의 핵심 규칙은 다음과 같습니다.

```
SCL이 Low일 때  : SDA 변경 가능
SCL이 High일 때 : SDA 데이터가 안정적으로 유지되어야 함
```

따라서 수신 장치는 일반적으로 SCL 상승 에지에서 SDA 상태를 샘플링합니다.

```
SCL 상승 에지 ↑ : SDA 값을 읽음
SCL 하강 에지 ↓ : 송신 측이 다음 SDA 비트를 준비
```

파형을 직접 읽을 때는 각 SCL 상승 에지에서 SDA가 High인지 Low인지 확인하면 됩니다.


>SDA/SCL은 Open-Drain이므로 풀업저항이 중요하다. START/STOP은 SCL High에서 SDA가 변하는 조건이고, 매 8비트 다음 9번째 클럭에서 ACK/NACK가 나옴




<I2C가 Open-Drain인 이유와 동작 방식>

- Push-Pull 출력

  GPIO가 High와 Low를 모두 능동적으로 출력합니다.

```
High 출력 → 내부 상단 트랜지스터 ON
Low 출력  → 내부 하단 트랜지스터 ON
```

  문제는 두 장치가 같은 선을 공유할 때 발생합니다.

```
장치 A: High 출력
장치 B: Low 출력
```
- Open-Drain 출력

  Open-Drain은 장치가 할 수 있는 동작이 두 가지뿐입니다.

```
1. 선을 GND로 당김 → Low
2. 선에서 손을 뗌 → High-Z
```

장치가 직접 High를 출력하지 않습니다.

High는 외부 풀업저항이 만듭니다.

```
             3.3V
              │
            4.7kΩ
              │
SDA ──────────┼──── 여러 장치
              │
          Open-Drain
              │
             GND
```

## Low를 보낼 때

내부 트랜지스터를 켜서 선을 GND로 당깁니다.

```
SDA ≈ 0V
```

## High를 보낼 때

트랜지스터를 끄고 선에서 손을 뗍니다.

```
풀업저항이 SDA를 3.3V로 올림
```

## Open-Drain을 쓰는 이유 1: 여러 장치 공유

여러 장치가 동시에 선에 연결되어 있어도:

- 아무도 Low로 당기지 않으면 High
- 하나라도 Low로 당기면 Low

가 됩니다.

```
장치 A 해제 + 장치 B 해제 → High
장치 A Low + 장치 B 해제 → Low
장치 A 해제 + 장치 B Low → Low
장치 A Low + 장치 B Low → Low
```

이를 **wired-AND**라고 합니다.

---

## 이유 2: Arbitration

두 Controller가 동시에 통신을 시작했다고 가정해보겠습니다.

Controller A:

```
1을 보내려고 선에서 손을 뗌
```

Controller B:

```
0을 보내려고 SDA를 Low로 당김
```

실제 버스는 Low가 됩니다.

Controller A는 자신이 1을 보냈다고 생각했는데 실제 SDA를 읽어 보니 0입니다.

```
“다른 Controller가 더 우선인 데이터를 보내는 중이구나”
```

라고 판단하고 버스 제어를 포기합니다.

Push-Pull 방식이었다면 출력끼리 충돌하지만 Open-Drain에서는 안전하게 우선순위를 판별할 수 있습니다. I2C의 다중 Controller arbitration은 이러한 wired-AND 특성을 이용합니다.

---

## 이유 3: Clock Stretching

느린 Target이 아직 준비되지 않았을 때 SCL을 Low로 계속 잡을 수 있습니다.

Controller는 SCL을 High로 만들려고 선을 해제하지만 Target이 계속 Low로 당기므로 실제 SCL은 Low로 유지됩니다.

Target이 작업을 끝내고 SCL을 놓으면 풀업저항에 의해 High가 되고 통신이 계속됩니다.

---

## 보드 내부 풀업저항으로는 안 되나

STM32 GPIO에는 내부 Pull-up 기능이 있습니다. 하지만 보통 I2C의 정식 풀업으로는 외부 저항을 사용합니다.

### 이유

내부 풀업은 일반적으로 저항값이 크고 정확도가 낮습니다.

예를 들어 내부 풀업이 수십 kΩ 수준이라면 버스 커패시턴스와 결합해 상승 시간이 느려집니다.

I2C 선이 High로 올라가는 과정은 즉시 수직으로 올라가지 않고 RC 충전 형태입니다.

```
상승시간 ≈ 풀업저항 × 버스 커패시턴스
```

저항이 클수록 High로 천천히 올라갑니다.

```
강한 풀업, 작은 저항 → 빠른 상승
약한 풀업, 큰 저항 → 느린 상승
```

100kHz의 짧은 배선에서는 내부 풀업으로 우연히 동작할 수도 있지만 다음 조건에서 불안정해집니다.

- 배선이 길어짐
- 장치 수 증가
- 브레드보드 사용
- 속도 증가
- 노이즈 증가

따라서 실습에서는 SDA와 SCL 각각에 외부 `4.7kΩ` 정도를 3.3V로 연결하는 방법이 안전합니다.

```
PB8 SCL ── 4.7kΩ ── 3.3V
PB9 SDA ── 4.7kΩ ── 3.3V
```


## I2C의 SCL과 SPI의 SCK의미

둘 다 **Serial Clock** 역할을 하므로 기본 목적(데이터를 읽을 기준을 정함) / 규칙은 다르다

|구분|I2C SCL|SPI SCK|
|---|---|---|
|생성자|Controller|Master|
|출력 방식|Open-Drain이 일반적|Push-Pull|
|풀업저항|필요|일반적으로 불필요|
|여러 장치 공유|가능|가능하지만 CS 별도 필요|
|Target이 클럭을 멈출 수 있나|Clock stretching 가능|일반적으로 불가능|
|데이터 샘플 기준|SCL High 중 SDA 안정|CPOL/CPHA에 따라 결정|
|클럭선 유휴 상태|풀업으로 High|CPOL에 따라 Low 또는 High|
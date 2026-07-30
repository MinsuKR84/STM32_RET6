
4.  CAN 통신 (CANH끼리, CANL끼리)
- CAN은 여러 노드가 동일한 두 선을 공유하는 차동 통신(CANH,CANL)
- CAN 통신은 두 계층으로 나눠서 봐야함

## CAN Controller
- STM32F303RET6 내부에 있음
<기능>
- CAN 프레임 생성
- ID
- DLC
- Data
- CRC
- ACK 처리
- 필터
- FIFO
- 오류 관리

## CAN Transceiver
- 외부 VP230 모듈이 담당

```
MCU CAN_TX/CAN_RX
↕
CAN 트랜시버
↕
CANH/CANL 차동 버스
```
※ 두 보드 통신에는 각 보드마다 트랜시버 하나씩, 총 두 개가 필요

```
STM32 CAN Controller
       ↓ TX/RX 논리신호
CAN Transceiver
       ↓ 차동 신호
CANH / CANL
```

CAN에는 UART의 목적지 주소와 같은 개념보다 **메시지 ID** 개념이 중요

예를 들어:

```
TxHeader.StdId = 0x123;
```

이것은 “0x123번 보드로 보내라”라기보다:

> 이 메시지의 종류 또는 우선순위가 0x123이다.

라는 의미입니다.

수신 노드가 필터를 사용해 필요한 ID만 골라 받음

<CAN 배선>

- MCU CAN_TX → 모듈 CTX
- MCU CAN_RX ← 모듈 CRX

※ CAN_TX와 CTX, CAN_RX와 CRX는 **같은 역할끼리 연결**

- 모듈 A CANH ───── 모듈 B CANH 
- 모듈 A CANL ───── 모듈 B CANL 
- 모듈 A GND ───── 모듈 B GND

- CAN 종단저항 = CAN 버스 양끝에는 120Ω 종단 저항을 둠

## CAN 송신 헤더 해석

```
TxHeader.StdId = MY_CAN_ID;
```

- 11비트 Standard ID를 설정합니다.

```
TxHeader.IDE = CAN_ID_STD;
```

- Standard ID 형식을 사용합니다.
- Classical CAN에서는 Standard 11비트 ID와 Extended 29비트 ID를 사용할 수 있습니다.

```
TxHeader.RTR = CAN_RTR_DATA;
```

- 실제 데이터 프레임임을 뜻합니다.
- Remote frame이 아니라 데이터가 포함된 프레임입니다.

```
TxHeader.DLC = 8;
```

- Data Length Code입니다.
- 데이터가 8바이트임을 나타냅니다.

```
TxHeader.TransmitGlobalTime = DISABLE;
```

- 시간 트리거 관련 기능을 사용하지 않습니다.

---

## CAN 송신 코드 해석

```
uint32_t TxMailbox;
```

- CAN Controller 내부의 어느 송신 mailbox에 프레임이 들어갔는지 저장합니다.

```
HAL_CAN_AddTxMessage(&hcan,
                     &TxHeader,
                     TxData,
                     &TxMailbox);
```

각 부분은:

- `&hcan`: 사용할 CAN
- `&TxHeader`: ID, DLC 등의 헤더 정보
- `TxData`: 전송 데이터 배열
- `&TxMailbox`: 선택된 mailbox 번호 저장

중요한 점은 이 함수가 `HAL_OK`를 반환하는 것이 **상대방 수신 완료를 의미하지 않는다는 것**입니다.

정확히는:

> 전송 요청을 STM32 CAN 송신 mailbox에 정상적으로 등록했다.

는 뜻입니다.




## CAN 수신 필터 코드

```
CAN_FilterTypeDef canFilter;
```

- CAN 필터 설정 구조체입니다.

```
canFilter.FilterBank = 0;
```

- 사용할 필터 bank 번호입니다.

```
canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
```

- ID와 Mask를 이용해 필터링합니다.

```
canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
```

- 32비트 필터 크기를 사용합니다.

```
canFilter.FilterIdHigh = 0;
canFilter.FilterIdLow = 0;
canFilter.FilterMaskIdHigh = 0;
canFilter.FilterMaskIdLow = 0;
```

ID와 Mask가 모두 0이면 사실상 모든 메시지를 수신하도록 설정한 형태입니다.

```
canFilter.FilterFIFOAssignment = CAN_FILTER_FIFO1;
```

- 필터를 통과한 메시지를 FIFO1에 저장합니다.

```
canFilter.FilterActivation = ENABLE;
```

- 해당 필터를 활성화합니다.

```
HAL_CAN_ConfigFilter(&hcan, &canFilter);
```

- 설정값을 실제 CAN 주변장치에 적용

## CAN 시작과 알림

```
HAL_CAN_Start(&hcan);
```

- CAN Controller를 초기화 상태에서 실제 통신 가능한 상태로 전환합니다.

```
HAL_CAN_ActivateNotification(
    &hcan,
    CAN_IT_RX_FIFO1_MSG_PENDING
);
```

- FIFO1에 메시지가 들어오면 인터럽트를 발생시키도록 합니다.

---

## CAN IRQ Handler

```
void CAN_RX1_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan);
}
```

하드웨어에서 CAN RX1 인터럽트가 발생하면 이 함수가 실행됩니다.

```
HAL_CAN_IRQHandler(&hcan);
```

HAL 드라이버가 다음을 확인합니다.

- 어떤 CAN 이벤트가 발생했는지
- FIFO0인지 FIFO1인지
- 송신 완료인지
- 오류인지

그 후 상황에 맞는 callback을 호출합니다.

---

## FIFO1 수신 callback

```
void HAL_CAN_RxFifo1MsgPendingCallback(
    CAN_HandleTypeDef *hcan_ptr)
{
    HAL_CAN_GetRxMessage(
        hcan_ptr,
        CAN_RX_FIFO1,
        &RxHeader,
        RxData
    );
}
```

`HAL_CAN_GetRxMessage()`가 FIFO1에 들어 있는 메시지를 꺼내서:

- 헤더를 `RxHeader`
- 데이터를 `RxData`

에 저장합니다.

FIFO1을 사용한다면 다음이 모두 FIFO1로 통일되어야 합니다.

```
FilterFIFOAssignment = FIFO1
Notification = FIFO1
Callback = Fifo1
GetRxMessage = FIFO1
IRQ = RX1
```

STM32 HAL에서도 FIFO1 메시지 수신은 FIFO1 callback에서 `HAL_CAN_GetRxMessage(...CAN_RX_FIFO1...)`를 호출하는 구조



## CAN 비트 타이밍

- CAN은 별도 클럭선을 사용하지 않음
- 각 노드는 자신 내부 클럭으로 bit 시간을 만들고, CANH/CANL의 edge를 보면서 서로 동기화
- 모든 노드들은 동일 bitrate, 적절한 sample point, 허용 가능한 클럭 오차, 적절한 SJW 맞아야함
```
hcan.Init.Prescaler = 4;
hcan.Init.Mode = CAN_MODE_NORMAL;
hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
hcan.Init.TimeSeg1 = CAN_BS1_13TQ; 
hcan.Init.TimeSeg2 = CAN_BS2_2TQ; 

Sync Segment : 수신 edge를 기준으로 각 노드의 bit 타이밍을 동기화하는 구간 (항상 1)
= 한 번의 재동기화에서 최대 1TQ까지 조정가능
BS1 : 신호가 케이블과 트랜시버를 왕복하는 지연을 보상하고 sample point 위치를 조절
= Propagation Segment + Phase Segment 1
BS2 : Sample point 이후부터 다음 bit까지의 구간
```

- 한 비트는 다음 구간으로 구성(STM32 bxCAN에서)
```
1 Sync Segment(항상 1) + TimeSeg1(BS1) + TimeSeg2(BS2)
```
- 설정을 아래처럼 하면

- TQ(Time Quantum - CAN 한 비트 시간을 잘게 나눈 최소 시간 단위) 
	총 TQ(1bit)= 1 + 13 + 2 = 16TQ
	TQ는 CAN 주변장치 클럭(fCAN)과 Prescaler로 결정됩니다.

$$
tTQ=\frac{Prescaler}{f_{CAN}}
$$

또는 TQ 주파수로 보면:

$$
fTQ=\frac{f_{CAN}}{Prescaler}
$$

CAN bitrate 공식
$$
Bitrate=\frac{f_{CAN}}{Prescaler×(1+BS1+BS2)}
$$

※ **CAN 주변장치에 실제로 들어가는 APB 클럭**을 같이 확인해야 함(CubeMX에서 **APB1 peripheral clock 값**을 확인)

## Sample Point

- Sample Point는 수신 노드가 CAN 버스 값을 실제 0 또는 1로 판정하는 시점

위치는 BS1 끝입니다.

```
Sync |──────── BS1 ────────|─ BS2 ─|
                           ↑
                      Sample Point
```
- 공식
$$
SamplePoint=\frac{1+BS1}{1+BS1+BS2}×100
$$
ex) Sample Point가 87.5% 일때, 한 bit 시간의 87.5% 지점에서 버스 상태를 읽음

※ Sample point를 bit 뒤쪽에 두는 것은 케이블과 트랜시버 전파 지연 후에도 신호가 안정될 시간을 확보하기 위한 것

## SJW(**Synchronization Jump Width**)

- 노드들의 클럭이 완전히 똑같지 않거나 edge가 예상 위치보다 조금 빠르거나 늦을 때, bit timing을 얼마나 보정할 수 있는지를 나타냄
- 역할 - 클럭 오차 보정 / edge 위치 오차 보정 / 노드 간 동기 유지

※ 위 처럼 두 보드 설정을 맞추지 않으면 "같은 CANH/CANL 파형을 서로 다른 시간 간격으로 읽게 됨"
# CAN 송신 성공 메시지의 정확한 의미

## `HAL_CAN_AddTxMessage() == HAL_OK`

```
if (HAL_CAN_AddTxMessage(...) == HAL_OK)
{
    printf("sent");
}
```

이때 출력한 `sent`는 표현상 오해를 만들 수 있습니다.

실제 의미는:

```
CAN Controller의 비어 있는 Tx mailbox에
전송할 프레임을 정상 등록했다.
```

다음 단계는:

1. mailbox 등록 
2. 버스가 비기를 기다림 
3. Arbitration 참여 
4. 전체 프레임 송신 
5. 다른 노드의 ACK 확인 
6. 오류 없이 송신 완료

입니다.

그래서 출력 문구는 다음처럼 표현하는 것이 더 정확합니다.

```
printf("[TX QUEUED]\r\n");
```

실제 버스 전송 완료와 ACK 여부는 추가 상태를 확인해야 합니다.

## 실제 전송 완료 확인

### 방법 1: Pending 상태 확인

```
while (HAL_CAN_IsTxMessagePending(&hcan, TxMailbox))
{
}
```

Pending이 해제되면 mailbox의 요청 처리가 끝났다는 뜻입니다.

다만 오류 여부도 함께 확인하는 것이 좋습니다.

```
uint32_t error = HAL_CAN_GetError(&hcan);
```

---

### 방법 2: Tx complete callback

활성화:

```
HAL_CAN_ActivateNotification(
    &hcan,
    CAN_IT_TX_MAILBOX_EMPTY
);
```

Callback:

```
void HAL_CAN_TxMailbox0CompleteCallback(
    CAN_HandleTypeDef *hcan)
{
    printf("[TX COMPLETE]\r\n");
}
```

사용된 mailbox에 따라 0, 1, 2 callback이 있습니다.

---

### 방법 3: 상대방 수신 확인

가장 확실한 응용 수준 확인은 상대 노드가 메시지를 받은 뒤 응답 프레임을 보내는 것입니다.

```
Board A → ID 0x123 전송
Board B → 수신 후 ID 0x456으로 응답
Board A → 응답 수신
```

CAN의 ACK는 프레임이 물리적으로 정상 수신됐다는 뜻이지, 상대방 애플리케이션이 그 데이터를 처리했다는 뜻까지 보장하지는 않습니다.


## CAN ACK의 특징

- CAN은 송신 노드 자신이 ACK를 넣지 않음

- 프레임을 정상적으로 받은 다른 노드가 ACK 슬롯을 dominant로 만들어 줍니다. 
- 따라서 CAN 버스에 송신 노드 하나만 있다면:
-> 데이터는 내보냄, ACK를 받지 못함, 오류 발생 또는 재전송, 정상 전송으로 완료되지 않을 수 있음

실제 통신 테스트는 두 노드가 있어야 하는 이유임

## ACK 슬롯을 다른 노드가 Dominant로 만든다는 의미

CAN 프레임에는 데이터와 CRC 뒤에 ACK 영역이 있습니다.

간단히 나타내면:

```
SOF → ID → Control → Data → CRC → ACK → EOF
```

ACK 영역은 다음으로 나뉩니다.

```
ACK Slot
ACK Delimiter
```

---

## 송신 노드의 동작

송신 노드는 ACK Slot에서 **Recessive bit**를 내보냅니다.

쉽게 말하면 버스를 강제로 dominant로 만들지 않고 확인합니다.

```
송신 노드:
“내 메시지를 누군가 정상적으로 받았다면
이 시간에 dominant를 넣어 줘.”
```

---

## 수신 노드의 동작

프레임을 정상적으로 받은 수신 노드는 다음을 검사합니다.

- bit 형식
- CRC
- frame 형식

정상이라면 ACK Slot 동안 버스를 Dominant로 당깁니다.

```
수신 노드:
“정상적으로 받았어.”
```

---

## 송신 노드가 보는 버스

송신 노드는 자신이 Recessive를 내보냈는데 실제 버스에서 Dominant를 읽습니다.

```
자기가 출력한 값: Recessive
실제 버스 값:     Dominant
```

그러면:

```
다른 노드가 ACK를 넣었다
→ 최소 한 노드가 프레임을 정상 수신했다
```

라고 판단합니다.

---

## 수신 노드가 없으면

송신 노드만 연결되어 있으면 ACK Slot에서 아무도 Dominant로 당기지 않습니다.

```
송신 노드: Recessive
다른 노드: 없음
버스: Recessive
```

송신 노드는 ACK를 받지 못했으므로 ACK Error로 판단합니다.

Auto Retransmission이 켜져 있으면 같은 프레임을 다시 전송할 수 있고, 오류가 반복되면 error counter가 증가합니다.

---

## 중요한 점

ACK는 다음을 뜻합니다.

> 적어도 하나의 다른 CAN Controller가 프레임을 오류 없이 수신했다.

하지만 다음까지 뜻하지는 않습니다.

- 목표 애플리케이션이 데이터를 사용함
- 해당 ID의 보드가 원하는 동작을 완료함
- 프로그램 로직이 정상 처리됨

그래서 응용 단계에서는 별도의 응답 메시지를 설계하기도 합니다.

## CAN FIFO0/FIFO1 구조

수신 메시지는 필터를 거쳐 FIFO0 또는 FIFO1로 들어갑니다.

예를 들어 FIFO1을 사용한다면 다음 네 부분이 모두 FIFO1로 맞아야함

## 필터

```
canFilter.FilterFIFOAssignment = CAN_FILTER_FIFO1;
```

## 알림

```
HAL_CAN_ActivateNotification(
    &hcan,
    CAN_IT_RX_FIFO1_MSG_PENDING
);
```

## 콜백

```
void HAL_CAN_RxFifo1MsgPendingCallback(
    CAN_HandleTypeDef *hcan_ptr)
{
    HAL_CAN_GetRxMessage(
        hcan_ptr,
        CAN_RX_FIFO1,
        &RxHeader,
        RxData
    );
}
```

## IRQ 처리

```
void CAN_RX1_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan);
}
```
※ 이 중 하나라도 FIFO0로 남아 있으면 FIFO1 수신이 정상적으로 처리되지 않을 수 있음

<CAN 인터럽트가 동작한 흐름>

- 우리 실습에서 다음 IRQ handler가 추가된 뒤 수신이 된 이유는:

```
void CAN_RX1_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan);
}
```

- 인터럽트 처리 경로가 완성됐기 때문입니다.

```
CAN 프레임 수신
→ 필터 통과
→ FIFO1 저장
→ CAN_RX1 인터럽트 발생
→ CAN_RX1_IRQHandler()
→ HAL_CAN_IRQHandler()
→ HAL_CAN_RxFifo1MsgPendingCallback()
→ HAL_CAN_GetRxMessage()
```

이 경로에서 `HAL_CAN_IRQHandler()`가 빠지면 HAL callback이 호출되지 않음

<CAN 폴링 방식>

- 인터럽트 문제인지 확인할 때는 FIFO를 직접 확인할 수 있습니다.

```
if (HAL_CAN_GetRxFifoFillLevel(
        &hcan,
        CAN_RX_FIFO1) > 0)
{
    if (HAL_CAN_GetRxMessage(
            &hcan,
            CAN_RX_FIFO1,
            &RxHeader,
            RxData) == HAL_OK)
    {
        printf(
            "[RX] ID=0x%03lX\r\n",
            RxHeader.StdId
        );
    }
}
```

폴링으로는 되는데 인터럽트로 안 된다면 다음 문제를 의심합니다.

- NVIC 미설정
- IRQ handler 누락
- FIFO notification 불일치
- callback 이름 불일치

<파형>
CAN은 차동 신호이므로 CANH와 CANL을 함께 측정하는 것이 좋습니다.

```
CH1 → CANH
CH2 → CANL
GND → CAN GND
```

## 정상 상태

### Recessive = 버스가 지배적으로 당겨지지 않은 상태

CANH와 CANL이 서로 비슷한 전압(차동전압 ≈ 0)

논리적으로는 보통 `1`에 해당합니다.

버스 유휴 상태도 Recessive입니다.

```
아무도 송신하지 않음
→ CANH/CANL 차이가 작음
→ Recessive
```
### Dominant

```
CANH 상승
CANL 하강
차동 전압 증가
```

논리적으로는 보통 `0`에 해당합니다.

- 두 파형이 반대 방향으로 움직입니다.

한 노드가 Recessive를 보내고 다른 노드가 Dominant를 보내면 실제 버스는 Dominant가 됩니다.

```
노드 A: Recessive
노드 B: Dominant
결과:   Dominant
```

Dominant가 Recessive를 덮어쓰기 때문에 Dominant라고 부릅니다.

I2C Open-Drain과 비슷한 우선 동작 개념이 있지만, CAN은 CANH/CANL 차동 물리계층을 사용한다는 차이가 있다.

정확한 공통모드 전압은 트랜시버와 회로 조건에 따라 달라질 수 있으므로, 핵심은 **CANH와 CANL의 차이**입니다.

```
Vdiff = CANH - CANL
```

Dominant 상태에서 차동전압이 커지고, Recessive 상태에서 작아짐

- 예를 들어 500kbps라면 한 비트 시간은:

```
1 / 500000 = 2µs
```

# CAN Arbitration에서의 사용

두 노드가 동시에 송신했다고 가정합니다.

```
노드 A ID = 0x123
노드 B ID = 0x456
```

ID를 MSB부터 한 bit씩 내보내면서 동시에 실제 bus 상태를 읽습니다.

어떤 위치에서:

```
노드 A는 Dominant(0)
노드 B는 Recessive(1)
```

를 보냈다면 실제 bus는 Dominant가 됩니다.

노드 B는:

```
나는 Recessive를 보냈는데 bus는 Dominant다
→ 더 높은 우선순위 메시지가 있다
→ 송신 중단
```

이라고 판단합니다.

CAN에서는 ID 숫자가 작을수록 앞부분에서 dominant 0이 나올 가능성이 커 우선순위가 높다.

## 측정 순서

1. MCU의 CAN_TX/CTX 측정
2. 트랜시버의 CANH/CANL 측정
3. 상대 트랜시버의 CRX 측정

이렇게 보면 어느 구간에서 신호가 끊기는지 알 수 있습니다.

```
MCU CAN_TX 정상
→ CANH/CANL 비정상
= 트랜시버 송신부 문제 가능

CANH/CANL 정상
→ CRX 비정상
= 트랜시버 수신부 문제 가능

CRX 정상
→ MCU에서 수신 안 됨
= CAN 핀/필터/FIFO/IRQ 문제 가능
```

### CAN 종단저항 다는 이유

- 전기 신호의 반사(Reflection) 현상을 억제하여 신호 왜곡과 통신 에러를 막기 위해서
- 케이블 끝에서 신호 에너지를 흡수하여 이러한 반사를 줄임
## 왜 양 끝에 하나씩인가

CAN 버스는 하나의 신호선에 여러 노드가 병렬로 붙는 **버스(Bus) 구조**입니다. 신호는 특정 노드에서 출발해 **양쪽 방향**으로 퍼진다.

```
120Ω ───── CAN 버스 ───── 120Ω
```

양쪽 끝을 모두 종단해야 양 방향의 반사를 줄일 수 있습니다.

중간 노드마다 120Ω을 달면 전체 부하가 너무 낮아져 트랜시버가 과도한 전류를 공급해야 합니다.

※ 120Ω 이유 - can 통신에 쓰이는 꼬임선(Twisted Pair Cable)의 특성 임피던스(Characteristic Impedance)가 물리적으로 연결되어 있음

## CAN Prescaler

CAN에서는 Prescaler가 CAN 주변장치 클럭을 나누어 **Time Quantum, TQ**를 만듭니다.

```
CAN 입력 클럭
÷ Prescaler
= TQ 기준 클럭
```

CAN에서는 Prescaler만으로 bitrate를 결정하지 않고 BS1, BS2와 함께 결정


> MCU 내부 CAN 컨트롤러와 외부 트랜시버를 구분해야 한다. 두 노드 모두 트랜시버가 필요하며 CANH/CANL 양 끝에 120Ω 종단저항을 둔다. 수신 시 필터, FIFO, notification, IRQ, callback이 모두 같은 FIFO 기준으로 맞아야함

---

**UART, I2C, SPI, CAN 모두 전압과 전류로 이루어진 전기신호를 사용**

- **UART와 SPI**는 MCU가 High와 Low를 직접 출력하므로 짧은 배선에서는 저항이 없어도 됩니다.
- **I2C**는 Open-Drain 방식이므로 High를 만들기 위한 풀업저항이 반드시 필요합니다.
- **CAN**은 긴 차동 버스에서 발생하는 신호 반사를 줄이기 위해 양 끝에 120Ω 종단저항을 사용합니다.
- UART와 SPI도 고속 또는 장거리에서는 저항이나 전용 트랜시버가 필요할 수 있습니다.


UART - TX 핀은 일반적으로 **Push-Pull 출력**

-  신호가 왕복하는 시간보다 한 비트 시간이 훨씬 길기 때문에 전송선 반사의 영향이 작다.

115200bps의 한 비트 시간은 약:

1115200≈8.68μs\frac{1}{115200}\approx8.68\mu s1152001≈8.68μs

점퍼선 수십 cm에서 신호가 이동하는 시간은 이보다 훨씬 짧습니다. 그래서 일반적인 짧은 UART 실습에서는 별도의 종단저항 없이도 잘 동작

UART라고 항상 저항이 필요 없는 것은 아닙니다.

다음 조건에서는 파형이 깨질 수 있습니다.

- 배선이 길다
- baud rate가 높다
- 케이블 품질이 나쁘다
- GND 차이가 크다
- 주변 노이즈가 심하다
- 여러 갈래로 배선했다

이때는 TX 근처에 작은 직렬저항을 넣기도 합니다.

```
UART TX ── 22~100Ω ── 수신 RX
```

이 직렬저항은 CAN의 120Ω 종단저항과 역할이 조금 다릅니다.

- 출력 엣지를 조금 완만하게 함
- 링잉 감소
- 순간 전류 감소
- 신호 반사 완화

장거리 UART가 필요하면 보통 TTL UART를 그대로 멀리 보내지 않고 RS-232나 RS-485 트랜시버를 사용


SPI에도 다음 모든 선에서 전기신호가 발생합니다.

- SCK
- MOSI
- MISO
- NSS

SPI 역시 일반적으로 Push-Pull 출력입니다.

```
Master SCK  → 능동 High/Low 출력
Master MOSI → 능동 High/Low 출력
Slave MISO  → 선택된 동안 능동 High/Low 출력
Master NSS  → 능동 High/Low 출력
```

따라서 신호를 High로 올리기 위한 풀업저항은 기본적으로 필요하지 않습니다.

SPI는 원래 PCB 내부나 짧은 보드 간 연결을 주로 고려한 통신입니다.

```
같은 PCB 내부
또는
매우 짧은 보드 간 연결
```

## SPI는 오히려 고속에서 저항이 필요할 때가 많음

SPI는 MHz 단위로 매우 빠르게 동작할 수 있습니다.

예를 들어 STM32 SPI가 9MHz라면 클럭 한 주기는:

19MHz≈111ns\frac{1}{9MHz}\approx111ns9MHz1≈111ns

이 정도로 빨라지면 짧은 점퍼선에서도 링잉이나 반사가 나타날 수 있습니다.

그래서 실제 회로에서는 SCK나 MOSI에 작은 직렬저항을 넣는 경우가 많습니다.

```
STM32 SCK  ── 22~47Ω ── Slave SCK
STM32 MOSI ── 22~47Ω ── Slave MOSI
```

특히 SCK는 모든 비트의 기준이 되는 클럭이므로 파형이 깨지면 이중 클럭처럼 인식될 수 있습니다.


I2C에도 SDA와 SCL에서 전기신호가 발생합니다.

하지만 UART와 SPI와는 출력회로가 다릅니다.

UART와 SPI는 일반적으로 Push-Pull 출력입니다.

```
Push-Pull
→ High도 직접 출력
→ Low도 직접 출력
```

I2C는 Open-Drain 방식입니다.

```
Open-Drain
→ Low는 직접 출력
→ High는 직접 출력하지 않음
→ High일 때는 핀을 전기적으로 놓음
```

핀을 놓기만 하면 스스로 3.3V가 되지 않습니다. 그래서 외부 풀업저항이 필요합니다.

```
             3.3V
              │
            4.7kΩ
              │
SDA/SCL ──────┼──── I2C 장치
```

아무 장치도 Low로 당기지 않으면 풀업저항을 통해 High가 됩니다.

장치 하나라도 선을 GND로 당기면 Low가 됩니다.

|구분|I2C 풀업저항|CAN 종단저항|
|---|---|---|
|연결 위치|SDA/SCL에서 3.3V로|CANH와 CANL 사이|
|대표값|2.2kΩ~10kΩ|120Ω|
|주요 목적|High 상태 생성|전송선 반사 방지|
|없으면|High로 제대로 못 올라감|짧은 선에서는 될 수 있지만 반사·오류 가능|
|설치 수|버스 전체 기준 각 선에 풀업|버스 양 끝에 하나씩|

CAN은 다음 구조로 사용되기 때문입니다.

## 10-1. 배선이 길 수 있음

자동차나 산업장비에서는 수 m에서 수십 m까지 갈 수 있습니다.

## 10-2. 여러 노드가 연결됨

```
노드 A ─ 노드 B ─ 노드 C ─ 노드 D
```

배선 분기와 각 노드 입력 커패시턴스가 신호 품질에 영향을 줍니다.

## 10-3. 모든 노드가 같은 버스를 동시에 읽음

CAN에서는 송신 중인 노드도 버스 상태를 계속 읽습니다.

이 기능을 통해 다음을 수행합니다.

- Arbitration
- ACK 확인
- Bit error 검출

따라서 반사 때문에 순간적으로 전압이 잘못 보이면 단순히 한 바이트만 깨지는 것이 아니라 CAN Controller가 오류를 검출하고 Error Frame을 발생시킬 수 있습니다.

## 10-4. 차동신호 사용

CAN 트랜시버는 CANH와 CANL의 절대전압 하나만 보는 것이 아니라 두 선의 차이를 판단합니다.

```
차동전압 = CANH - CANL
```

Dominant일 때 두 선의 차이가 커지고, Recessive일 때 차이가 작아집니다.

종단저항은 이 차동신호가 케이블 끝에서 안정적으로 유지되도록 돕습니다


|통신|기본 출력 방식|기본적으로 필요한 저항|이유|
|---|---|---|---|
|UART|Push-Pull|보통 없음|짧은 일대일 연결에서 직접 High/Low 출력|
|SPI|Push-Pull|보통 없음|짧은 PCB 내부 연결을 주로 사용|
|I2C|Open-Drain|SDA/SCL 풀업저항|장치가 High를 직접 출력하지 않음|
|CAN|차동 트랜시버|양 끝 120Ω 종단저항|전송선 반사 억제 및 임피던스 정합|

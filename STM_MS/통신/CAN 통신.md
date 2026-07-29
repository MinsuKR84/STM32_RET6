
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
```
hcan.Init.Prescaler = 4;
hcan.Init.Mode = CAN_MODE_NORMAL;
hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
```
- 한 비트는 다음 구간으로 구성
```
1 Sync Segment
+ TimeSeg1
+ TimeSeg2
```
- 설정을 아래처럼 하면
```
총 TQ = 1 + 13 + 2 = 16TQ
```

- CAN 주변장치 클럭을 `fCAN`이라고 하면
```
CAN bitrate = fCAN / [Prescaler × 16]
```

<CAN 송신 성공 메시지의 정확한 의미>

다음 함수가 `HAL_OK`를 반환했다고 해서 상대가 수신했다는 뜻은 아닙니다.

```
HAL_CAN_AddTxMessage(...)
```

정확한 뜻은:

> 송신 프레임을 CAN 송신 메일 박스에 넣는 데 성공했다.

입니다.

그래서 출력은 다음처럼 표현하는 것이 더 정확합니다.

```
[TX QUEUED]
```

실제 버스 전송 완료와 ACK 여부는 추가 상태를 확인해야 합니다.

<CAN ACK의 특징>

- CAN은 송신 노드 자신이 ACK를 넣지 않음

- 프레임을 정상적으로 받은 다른 노드가 ACK 슬롯을 dominant로 만들어 줍니다. 따라서 CAN 버스에 송신 노드 하나만 있다면:
- 데이터는 내보냄
- ACK를 받지 못함
- 오류 발생 또는 재전송
- 정상 전송으로 완료되지 않을 수 있음

실제 통신 테스트는 두 노드가 있어야 하는 이유임

<CAN FIFO0/FIFO1 구조>
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

### Recessive

CANH와 CANL이 서로 비슷한 전압입니다.

### Dominant

```
CANH 상승
CANL 하강
```

두 파형이 반대 방향으로 움직입니다.

정확한 공통모드 전압은 트랜시버와 회로 조건에 따라 달라질 수 있으므로, 핵심은 **CANH와 CANL의 차이**입니다.

```
Vdiff = CANH - CANL
```

Dominant 상태에서 차동전압이 커지고, Recessive 상태에서 작아짐

- 예를 들어 500kbps라면 한 비트 시간은:

```
1 / 500000 = 2µs
```

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

### CAN 종단저항

전송선 반사와 임피던스 정합을 위한 저항입니다.

```
CANH ↔ CANL 사이에 120Ω
```

> MCU 내부 CAN 컨트롤러와 외부 트랜시버를 구분해야 한다. 두 노드 모두 트랜시버가 필요하며 CANH/CANL 양 끝에 120Ω 종단저항을 둔다. 수신 시 필터, FIFO, notification, IRQ, callback이 모두 같은 FIFO 기준으로 맞아야함
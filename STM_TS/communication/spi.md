Serial Peripheral Interface의 약자
Controller가 생성한 클럭에 맞추어 데이터를 송수신하는 동기식 직렬 통신 방식임

SPI는 일반적으로 I2C보다 높은 속도를 지원하며, 센서, 디스플레이, ADC, DAC, SD 카드, Ethernet 모듈 등에 사용됨



기본적인 SPI 통신에는 다음 네 개의 신호선이 사용됨
SCK : Serial Clock
MOSI : Controller에서 Peripheral로 데이터 전송
MISO : Peripheral에서 Controller로 데이터 전송 
CS 또는 NSS : 통신 대상 장치 선택

STM32 SCK --- Peripheral SCK 
STM32 MOSI --- Peripheral MOSI 
STM32 MISO --- Peripheral MISO 
STM32 CS --- Peripheral CS 
STM32 GND --- Peripheral GND



동기식 통신 
SPI에서는 Controller가 SCK 클럭을 생성함
송신 측은 클럭의 특정 에지에서 데이터를 출력하고, 수신 측은 반대 또는 지정된 에지에서 데이터를 읽음
따라서 UART와 달리 송수신 장치가 Baud Rate를 별도로 맞출 필요가 없고, 같은 클록을 기준으로 동작함



전이중 통신 
SPI는 MOSI와 MISO 선이 분리되어 있어 데이터를 송신하면서 동시에 수신할 수 있음
예를 들어 STM32가 1바이트를 전송하면 동시에 Peripheral 장치에서도 1바이트가 STM32로 들어옴
SPI는 클록을 생성해야 데이터가 이동하므로, 데이터를 읽기 위해서도 Controller가 더미 데이터를 전송해야 하는 경우가 있음



CS 신호
CS는 통신할 Peripheral을 선택하는 신호임
대부분의 장치는 Active Low 방식으로 동작함
CS = Low : 장치 선택
CS = High : 장치 선택 해제

통신 과정은 일반적으로 다음과 같음
CS Low → 명령 전송 → 주소 전송 → 데이터 송수신 → CS High

여러 장치를 연결하면 SCK, MOSI, MISO는 공유할 수 있지만, 일반적으로 각 장치마다 별도의 CS 신호가 필요함

```
              ┌─ CS1 → Sensor
STM32 SPI ────┼─ CS2 → Display
              └─ CS3 → W5500
```



CPOL과 CPHA
SPI는 클럭 극성과 데이터 샘플링 시점을 설정해야 함

CPOL : 클럭이 대기 상태일 때의 논리 레벨
CPOL = 0 : 대기 상태 Low
CPOL = 1 : 대기 상태 High

CPHA : 첫 번째 또는 두 번째 클럭 에지 중 어느 에지에서 데이터를 읽을지 결정함
CPHA = 0: 첫 번째 에지에서 샘플링
CPHA = 1: 두 번째 에지에서 샘플링
이 조합으로 네 개의 SPI Mode가 만들어짐


| SPI Mode | CPOL | CPHA |
| :------: | :--: | :--: |
|  Mode 0  |  0   |  0   |
|  Mode 1  |  0   |  1   |
|  Mode 2  |  1   |  0   |
|  Mode 3  |  1   |  1   |
STM32와 Peripheral의 SPI Mode가 다르면 데이터가 정상적으로 전달되지 않음



비트 순서
SPI는 장치에 따라 최상위 비트 또는 최하위 비트를 먼저 전송함
MSB First: 최상위 비트부터 전송
LSB First: 최하위 비트부터 전송
대부분의 SPI 장치는 MSB First를 사용하지만 데이터시트를 확인해야 함



SPI 장단점
장점
통신 속도가 빠름
전이중 통신이 가능함
프로토콜 구조가 단순함
연속된 대용량 데이터 전송에 적합함

단점
I2C보다 많은 신호선이 필요함
장치가 늘어날수록 CS 핀이 추가로 필요함
표준화된 장치 주소 체계가 없음
기본적으로 ACK와 같은 수신 확인 기능이 없음
장치별 명령 형식을 데이터시트에서 확인해야 함
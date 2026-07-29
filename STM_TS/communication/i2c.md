Inter-Intergrated Circuit의 약자 
두 개의 신호선만으로 여러 장치를 연결할 수 있는 동기식 직렬 통신 방식임 

I2C에서는 통신을 시작하고 클럭을 생성하는 장치를 Controller 또는 Master라고 함
명령에 응답하는 장치를 Target 또는 Slave라고 함

각 Target 장치는 고유한 주소를 가지고 있어 하나의 버스에 여러 장치를 연결할 수 있음



I2C는 두 개의 신호선을 사용함
SCL : Serial Clock, Controller가 생성하는 클럭
SDA : Serial Data, 주소와 데이터를 전송하는 양방향 데이터선

SCL과 SDA에는 일반적으로 외부 풀업 저항이 필요함
보통 4.7K옴 정도가 자주 사용되지만, 적절한 값은 전압, 배선 길이, 통신 속도 및 버스 정전용량에 따라 달라짐



Open-Drain 방식
I2C의 SCL과 SDA 출력은 일반적으로 Open-Drain 구조를 사용함

장치는 신호선을 직접 High로 출력하지 않고 Low 출력 또는 출력 해제 상태를 만듦
모든 장치가 출력을 해제하면 풀업 저항에 의해 신호선이 High가 됨
이 구조를 사용하면 여러 장치가 같은 버스를 공유할 때 출력 충돌을 방지할 수 있음



I2C 통신 속도
대표적인 I2C 속도는 다음과 같음
Standard Mode | 100 kbit/s
Fast Mode | 400 kbit/s
Fast Mode Plus | 1 Mbit/s
High-Speed Mode | 3.4 Mbit/s
실제 STM32와 연결 장치가 지원하는 최대 속도를 확인해야 함



START와 STOP 조건
I2C에서는 SDA와 SCL의 변화로 통신 시작과 종료를 나타냄

START 조건 : SCL이 High인 상태에서 SDA가 High에서 Low로 변경됨 
STOP 조건 : SCL이 High인 상태에서 SDA가 Low에서 High로 변경됨

통신은 일반적으로 다음 순서로 진행됨
START → Address → R/W → ACK → Data → ACK → STOP



장치 주소
일반적으로 I2C 장치는 7비트 주소를 사용함 
7비트 주소 뒤에 읽기 또는 쓰기 방향을 나타내는 1비트가 추가됨, 7-bit Address + R/W bit

예를 들어 장치 주소가 0x68이라면 HAL 함수에서는 보통 다음처럼 왼쪽으로 1비트 이동시켜 전달함
```
#define DEVICE_ADDRESS (0x68 << 1)
```
STM32 HAL 함수가 주소와 R/W 비트를 포함하는 형식을 사용하기 때문임



ACK와 NACK
송신 측이 8비트를 전송하면 수신 측은 9번째 클럭에서 응답함
ACK : 정상적으로 수신함
NACK : 데이터를 수신하지 못했거나 더 이상 받지 않음 
8-bit Data → ACK/NACK
ACK는 수신 장치가 SDA를 Low로 당겨 표현함



I2C 레지스터 읽기 과정 
센서 내부의 특정 레지스터를 읽는 경우 일반적으로 다음 과정을 사용함
START → 장치 주소 + Write → 레지스터 주소 → Repeated START → 장치 주소 + Read → 데이터 수신 → NACK → STOP

먼저 읽으려는 레지스터 번호를 Write 방향으로 전송한 후 통신을 다시 시작하여 Read 방향으로 데이터를 가져옴



I2C의 장단점
두 개의 신호선으로 여러 장치를 연결할 수 있음
장치 주소를 사용하여 대상을 구분함
센서 및 EEPROM 연결에 적합함
ACK를 통해 수신 여부를 확인할 수 있음

단점
SPI보다 일반적으로 느림 
풀업 저항이 필요함
배선이 길어지면 신호 품질이 나빠질 수 있음
주소가 동일한 장치를 여러 개 연결하기 어려울 수 있음
반이중 방식이므로 동시에 송수신할 수 없음 
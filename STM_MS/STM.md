<MX경로(win버전)>
C:\Users\User\AppData\Local\Programs\STM32CubeMX

<ST-linker(STSW-LINK009)>
https://www.st.com/en/development-tools/stsw-link009.html?utm_source=chatgpt.com#

<CLT 경로(win버전)>
https://www.st.com/en/development-tools/stm32cubeclt.html?icmp=tt38569_gl_lnkon_apr2024

<STM 데이터 시트>
https://os.mbed.com/platforms/ST-Nucleo-F303RE/

<cmake 설치 / ninza>
winget install Kitware.CMake
winget install Ninja-build.Ninja

<build하는법>
cmake --preset Debug (디버그 build 만들어줌)
cmake --build --preset Debug (build)

<처음 업로드> = 일반 SWD 연결(Normal mode
STM32_Programmer_CLI -c port=SWD -w build\Debug\<파일이름>.elf -v -rst
<다음 업로드 시 - 복구> = Under Reset 모드(=NRST 핀 = 이미 안에 저장된 펌웨어가 있어 리셋 후 업로드 해야 에러가 안뜸)
STM32_Programmer_CLI -c port=SWD mode=UR -w build\Debug\<파일이름>.elf -v -rst

<파일명 확인하기>
Get-ChildItem .\build\Debug\*.elf

Default comfiler/linker => GCC로

<빨간 줄 없애기>
CMake: Delete Cache and Reconfigure (ctrl + .)

<vscode 새로고침>
Developer: Reload Window (ctrl + ,)



다음주에 할거
1. 버튼 누르면 LED 점멸 시작, 다시 누르면 점멸 정지(timer)
2. 저항 가져오기, 모터, 스피커
3. UART
4. I2c


나중에 할거
1. 달력만들어서 LCD확인

0x41

1000 0010


STM32F303RET6 프로젝트에서 3층 엘리베이터 내부 OLED 표시 및 제어 코드를 만들고 싶습니다.

현재 프로젝트에는 SSD1306 OLED 드라이버가 있고, OLED에는 층 표시와 이동 방향 화살표 애니메이션을 표시하고 싶습니다.

요구사항:
1. 내부 버튼 3개로 목표 층을 선택합니다.
   - 1층 버튼
   - 2층 버튼
   - 3층 버튼

2. 각 층에는 리미트 스위치가 있습니다.
   - 1층 리미트 스위치가 눌리면 현재 층 = 1층
   - 2층 리미트 스위치가 눌리면 현재 층 = 2층
   - 3층 리미트 스위치가 눌리면 현재 층 = 3층

3. 버튼을 누르면 현재 층과 목표 층을 비교합니다.
   - 목표 층이 현재 층보다 높으면 모터를 위로 이동
   - 목표 층이 현재 층보다 낮으면 모터를 아래로 이동
   - 목표 층이 현재 층과 같으면 모터 정지

4. 이동 중 OLED 표시:
   - 올라갈 때: 위쪽 화살표가 아래에서 나타나 위로 이동하고, 화면 위로 사라진 뒤 다시 아래에서 나타나는 애니메이션
   - 내려갈 때: 아래쪽 화살표가 위에서 나타나 아래로 이동하고, 화면 아래로 사라진 뒤 다시 위에서 나타나는 애니메이션

5. 목표 층에 도착하면:
   - 모터 정지
   - OLED에 ARRIVED / 1 FLOOR, ARRIVED / 2 FLOOR, ARRIVED / 3 FLOOR 표시

6. 코드는 blocking while 구조를 피하고, 아래 구조처럼 계속 반복 검사하는 방식으로 작성해주세요.
   - Read_Buttons()
   - Read_LimitSwitches()
   - Update_ElevatorState()
   - Update_Motor()
   - Update_OLED()

7. 버튼과 리미트 스위치는 GPIO_PULLUP 입력이고, 눌리면 GPIO_PIN_RESET으로 읽히는 구조입니다.

8. 모터 제어 함수는 일단 아래 함수 형태로 만들어주세요.
   - Motor_Up()
   - Motor_Down()
   - Motor_Stop()

9. 기존 SSD1306 함수들을 사용해주세요.
   - ssd1306_black_screen()
   - ssd1306_set_cursor()
   - ssd1306_write_string()
   - ssd1306_white_pixel()
   - ssd1306_update_screen()

10. 현재 main.c 구조를 크게 유지하면서 USER CODE 영역 안에 작성해주세요.

원하는 결과:
- 버튼을 누르면 목표 층이 저장됨
- 리미트 스위치로 현재 층을 인식함
- 이동 중에는 OLED 화살표 애니메이션이 계속 움직임
- 목표 층 도착 시 모터가 멈추고 OLED 층 표시가 바뀜
- 코드가 나중에 모터 드라이버 핀만 연결하면 바로 확장 가능하도록 깔끔하게 작성됨
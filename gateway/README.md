# NexusMeshX

**Langueges:** ![C](https://img.shields.io/badge/c-00599C?style=flat-square&logo=C%2B%2B&logoColor=white)
![C](https://img.shields.io/badge/C-Supported-green?logo=C)
![C](https://img.shields.io/badge/C-Tested-red?logo=C)
![C](https://img.shields.io/badge/C-Compatible-blue?logo=C)

**Controller's MCU:** ![Raspberry Pi](https://img.shields.io/badge/Rasberry_Pi-A22846?style=flat-square&logo=raspberrypi&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-Supported-green?logo=raspberrypi)
![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-Compatible-blue?logo=raspberrypi)
![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-Tested-red?logo=raspberrypi)
![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-4B-purple?logo=raspberrypi)

**Operating Systems:** ![Raspberry Pi OS](https://img.shields.io/badge/Rasberry_Pi_OS-A22846?style=flat-square&logo=linux&logoColor=white)
![Raspberry Pi OS](https://img.shields.io/badge/Rasberry_Pi_OS-Supported-green?logo=linux)
![Raspberry Pi OS](https://img.shields.io/badge/Rasberry_Pi_OS-Compatible-blue?logo=linux)
![Raspberry Pi OS](https://img.shields.io/badge/Rasberry_Pi_OS-Tested-red?logo=linux)

**Networks:** ![Wifi5](https://img.shields.io/badge/Wifi5-000000?style=flat-square&logo=rss&logoColor=white)
![802.11ac](https://img.shields.io/badge/802.11ac-000000?style=flat-square&logo=rss&logoColor=white)
![Wifi5](https://img.shields.io/badge/Wifi5-Supported-green?logo=rss)
![Wifi5](https://img.shields.io/badge/Wifi5-Compatible-blue?logo=rss)
![Wifi5](https://img.shields.io/badge/Wifi5-Tested-red?logo=rss)

**Robot's MCU:** ![STM32](https://img.shields.io/badge/STM32-0099B0?style=flat-square&logo=opensourcehardware&logoColor=white)
![STM32](https://img.shields.io/badge/STM32-Supported-green?logo=opensourcehardware)
![STM32](https://img.shields.io/badge/STM32-Compatible-blue?logo=opensourcehardware)
![STM32](https://img.shields.io/badge/STM32-Tested-red?logo=opensourcehardware)
![STM32](https://img.shields.io/badge/STM32-NUCLEO_F103RB-purple?logo=opensourcehardware)

**Conveyer's MCU:** ![Arduino](https://img.shields.io/badge/Arduino-00599C?style=flat-square&logo=opensourcehardware&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue?logo=opensourcehardware)
![Arduino](https://img.shields.io/badge/Arduino-Tested-red?logo=opensourcehardware)
![Arduino](https://img.shields.io/badge/Arduino-Uno_R3-purple?logo=opensourcehardware)

**Releases:** ![release](https://img.shields.io/badge/release-v1.0-purple?logo=release)

<div align="center">
  <img src="../img/NexusMesh-X.png" alt="NexusMesh-X">
</div>

<br>
<br>

# What is NexusMesh-X?
> NexusMesh-X의 소프트웨어 구조 및 동작 영상이 궁금하시면, 본 프로젝트에 첨부된 NexusMesh-X.pdf 파일을 참고하시길 바랍니다.

NexusMeshX는 **스마트 팩토리를 위한 고성능 로봇 제어 게이트웨이(AP)** 로, 로봇과 컨베이어 시스템 간의 안정적이고 효율적인 통신을 지원합니다.

본 게이트웨이는 **WiFi5**를 기반으로 설계되었으며, 자체 프로토콜인 **RCP(Robot Control Protocol)** 가 적용되어 비동기 통신 기반의 네트워크 성능 최적화와 통신 제어 기능을 제공합니다.

<br>

# Main Features

1. **서버-클라이언트 기반의 로봇 제어 프로토콜(RCP, Robot Control Protocol) 적용**
  + 멀티 스레드 기반의 채널 실행을 통해, 로봇과 컨베이어 시스템 간 통신에서 병목 현상을 줄이고, 데이터 흐름의 효율성을 극대화
  + 저지연 비동기 통신 지원으로 실시간 데이터 전송 지원
  + 흐름제어, 오류제어, 인증제어를 통해 데이터를 체계적이고 안정적인 통신 채널 지원

<br>

2. **Wi-Fi 네트워크 커널 모듈**
  + 현재 접속되어 있는 Wi-Fi 네트워크의 상태를 게이트웨이의 LED 점등을 통해 확인
  + 터미널 접속이나 별도의 모니터링 소프트웨어 실행 필요 없이, 하드웨어에서 직관적이고 편리하게 상태 확인 가능
  + 와이파이 신호 품질 체크
    + 통신 신뢰도를 높이기 위한 신호 강도 및 품질 상태 확인
  + 네트워크 트래픽 부하도 체크
    + 트래픽 과부하 감지 및 네트워크 효율 유지

<br>

3. **Wi-Fi5 기반의 무선 네트워크 지원**
+ IEEE 802.11ac 표준을 따르는 Cypress CYW43455 Wifi 칩 셋 내장
+ 20Mhz 대역폭 지원
+ 평균 72.2 Mb/s의 대역폭 지원

<br>

4. **실시간 네트워크 상태 모니터링**
+ Wi-Fi의 현재 네트워크 상태 정보를 터미널 환경에서도 실시간으로 모니터링 가능
+ 와이파이 네트워크 인터페이스 명 체크
  + 현재 사용 중인 Wi-Fi 칩의 네트워크 인터페이스 명 확인
+ 와이파이 네트워크 신호 강도 체크
  + 현재 접속 중인 Wi-Fi 네트워크의 신호 강도를 데시벨(dB) 단위로 확인
+ 와이파이 송수신 대역폭 체크
  + 현재 접속 중인 Wi-Fi 네트워크의 대역폭 확인
+ 와이파이 송수신 패킷 수 체크
  + 현재 접속 중인 Wi-Fi 네트워크에서 송수신 되는 패킷 수 확인


<br>

# Application Technologies

> 좀 더 세부적인 기술 내용이 궁금하시면 연결된 하이퍼 링크 또는 문서 하단의 Links 항목을 참고하시길 바랍니다.

**게이트 웨이**
+ 임베디드 시스템 최적화를 목적으로, 라즈베리파이 4 및 Rasberry Pi OS 기반으로 설계
+ 802.11ac에 준수한 WiFi5기반의 와이파이 모듈을 이용하여 유연한 게이트웨이(AP) 설계.
+ [hostapd와 wpa_suplicant](https://w1.fi/wpa_supplicant/devel/)를 통해, 필요에 따라 STA(Station) 모드와 AP(Access Point)모드를 자유롭게 설정 가능

<br>

**로봇 제어 프로토콜**
+ 비동기 통신 지원
+ 헤더, 데이터 정보 제공
+ 로그인 기반의 인증 제어
+ 체크섬 기반의 오류 제어
+ 메시지 송수신을 통한 흐름 제어
+ 메시지 코드 제공


<br>

**네트워크 데몬**
+ 서버: 라즈베리 파이에서의 소켓 통신
+ 클라이언트: STM32에서의 USART 통신
+ Epoll: 클라이언트 소켓 연결 요청에 대한 이벤트 기반 비동기 처리
+ POSIX 멀티스레드: 작업의 병렬 처리
+ POSIX 메시지 큐: 스레드간 비동기 통신(IPC)

<br>

**Wi-Fi 네트워크 커널 모듈**
+ [cfg80211 커널 드라이버](https://www.kernel.org/doc/html/v4.12/driver-api/80211/index.html) 연동
  + Wi-Fi5 기반 무선 네트워크 드라이버와의 연동 모듈
+ [nl80211 라이브러리](https://git.kernel.org/pub/scm/linux/kernel/git/linville/wireless.git/tree/include/uapi/linux/nl80211.h?id=HEAD) 연동
  + Wi-Fi 정보를 시스템 콜을 통해 cfg80211 모듈로 부터 추출
+ ~~[iperf_api 라이브러리](https://github.com/esnet/iperf/blob/master/src/iperf_api.h) 연동~~ (nl80211 라이브러리로 대체)
  + ~~iperf3의 라이브러리를 통해 디지털 대역폭 측정~~
  + ~~iperf3의 라이브러리를 이용하여 현재 송수신 트래픽 비율(부하도) 확인~~
+ LED를 통한 현재 네트워크 상태 확인
  + 커널 타이머와 게이트웨이 연결된 LED를 통해, 현재 와이파이 네트워크의 신호 품질, 트래픽 비율을 실시간으로 확인

<br>
<br>

IEEE 802.11에 대해서는 아래 링크를 참고 하시길 바랍니다.

+ https://standards.ieee.org/beyond-standards/the-evolution-of-wi-fi-technology-and-standards/

<br>

# Guide

아래 절차를 통해 설정 및 설치를 진행하시길 바랍니다.

## Step 01. 와이파이 모드 설정

본 게이트웨이는 라즈베리파이 4 기반으로 설계되었으므로, 라즈베리파이에 기본적으로 내장되어 있는 와이파이 모듈에 대한 설정을 진행하여 네트워크를 제어합니다.

와이파이 네트워크 설정에는, 기본적으로 `STA(Station) 모드`와 `AP(Access Point)모드`의 총 **두 가지 모드**가 지원되며, 상황 및 기호에 따라 알맞게 설정하실 수 있습니다.

### STA Mode
STA 모드는, 외부의 와이파이 게이트웨이에 클라이언트로써 접속하여 연결하는 방식을 의미합니다.

따라서 라즈베리파이 4에서는, `wpa_supplicant.conf` 파일에서 접속하고자 하는 게이트웨이의 `SSID`와 `패스워드` 정보를 입력하여 줍니다.

```bash
$ sudo vi /etc/wpa_supplicant/wpa_supplicant.conf
```

```bash
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
# 추가
network={
    ssid="Your-AP-SSID"
    scan_ssid=1
    psk="Your-AP-Password"
}
```

**선택사항:** 위 설정을 마쳤으면, 네트워크 혼용을 방지하기 위해 블루투스 설정을 비활성화 할 수 있습니다.
```bash
$ sudo vi /boot/firmware/config.txt
```
```bash
...
# 파일의 가장 하단에 추가
[all]
dtoverlay=disable-bt
```

위 설정을 마쳤으면, 시스템 재부팅을 진행한 후, 와이파이 모듈의 인터페이스가 활성화 되었는지 확인합니다.
```bash
$ sudo reboot
```
```bash
$ iwconfig

wlan0     IEEE 802.11  ESSID:"Your-AP-SSID"
          Mode:Managed  Frequency:2.432 GHz  Access Point: Your-AP-MAC-ADDRESS
          Bit Rate=72.2 Mb/s   Tx-Power=31 dBm
          Retry short limit:7   RTS thr:off   Fragment thr:off
          Power Management:on
          Link Quality=68/70  Signal level=-42 dBm
          Rx invalid nwid:0  Rx invalid crypt:0  Rx invalid frag:0
          Tx excessive retries:640  Invalid misc:0   Missed beacon:0

```

STA 모드 해제를 원하는 경우, 아래와 같이 설정을 추가하여 비활성화하실 수 있습니다.
```bash
...
# 파일의 가장 하단에 추가
[all]
dtoverlay=disable-wifi
```
```bash
$ sudo reboot
```

### AP Mode
AP모드는, 와이파이 네트워크를 직접 제공하여 클라이언트의 연결을 수립하는 게이트웨이로써의 역할을 수행하는 모드입니다.

라즈베리파이 4의 WiFi는 802.11ac를 준수하는 WiFi5로 동작합니다.

따라서 WiFi를 802.11ac로 설정할 수 있으며, 리눅스에서 WiFi 
AP 관련 기능 지원 및 인증 서비스를 제공하는 데몬인 `hostapd`를 통해 AP모드로 설정하실 수 있습니다.

우선, hostapd 프로그램을 다운로드합니다.
```bash
$ sudo apt install hostapd -y
```
hostapd는 터미널에서 작업을 수행할 수 있게 hostapd 커맨드를 제공하며, 해당 커맨드를 통해 와이파이를 AP 모드로 실행시킬 수 있습니다.
```bash
# -dd 옵션을 이용하여 디버깅 모드로 작업 수행 가능
$ sudo hostapd -dd hostapd.conf
```

시스템이 재부팅 되더라도 영구적으로 AP모드 유지를 원한다면, 커맨드가 아닌 hostapd 시스템 설정 파일의 편집을 통해 데몬을 제어할 수 있습니다.
```bash
$ sudo vi /etc/default/hostapd
```
```bash
...
DAEMON_CONF="/etc/hostapd/hostapd.conf"
...
```

이후, AP 설정 파일을 새롭게 생성하여 설정 내용을 편집합니다.
```bash
$ sudo vi /etc/hostapd/hostapd.conf
```
이때, AP 설정 파일에 들어갈 내용은 WIFI 버전에 따라 준수하는 기술 표준이 다르므로, 각 버전에 맞게 다르게 작성되어야 합니다.

아래는 WiFi5(802.11ac)일 때의 설정 내용입니다.
```bash
interface=wlan0
driver=nl80211
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

ssid=Your-AP-SSID
wpa_passphrase=YOUR-AP-PASSWORD

country_code=US

hw_mode=a
channel=165

# 아래 모든 항목을 Enable 해야 데이터 송수신 속도가 상승 됨
ieee80211n=1
require_ht=1
ieee80211ac=1
require_vht=1
```

위 설정을 모두 마쳤으면, hostapd 데몬을 재시작한 후 데몬의 상태를 확인합니다.
```bash
$ sudo systemctl unmask hostapd
$ sudo systemctl enable hostapd
$ sudo systemctl start hostapd

$ sudo systemctl status hostapd
```

<br>

## Step 02. 로봇 제어 채널 실행
데몬은 아래와 같이 실행 합니다.
```bash
$ make
$ ./nmx_channel_run
```
위와 같이 실행을 하면, 아래와 같이 워커 접속 및 데이터 전송을 실시간으로 확인하실 수 있습니다.

> NexusMesh-X의 채널은 Pub-Sub 패턴의 메시지 큐를 원리로 메시지 기반의 데이터 송수신을 지원합니다.

> Publisher 및 Subsciber는 워커, 채널 및 토픽을 관리하는 Broker는 매니저라는 이름의 오브젝트로 설계되었습니다.

단, RCP의 인증제어에 의해, 각 워커의 채널 접속을 위해서는 워커의 **디바이스 인증**을 거쳐야 하며, 

인증 방식은 `[PJW_SQL:PASSWD]` 또는 `[PJW_ROBOT:PASSWD]`와 같은 문자열 형식으로,

아이디의 `접미사에 컨베이어 측은 SQL, 로봇 측에는 ROBOT`을 반드시 포함하여야 합니다.

그렇지 않으면, 서로 다른 워커에 대한 스레드를 구분하여 생성할 수가 없어 런타임에서 버그가 발생합니다.

```C
...
// 컨베이어 워커에서는 블루투스를 통한 소켓 통신 수행
if (strstr(name_msg, "DETECT") != NULL) {
    write(dev_fd->btfd, "[SCM_SQL]DETECT\n", strlen("[SCM__SQL]DETECT\n"));
} else if (strstr(name_msg, "END_DETECT") != NULL) {
    write(dev_fd->btfd, "[SCM__SQL]END_DETECT\n", strlen("[SCM__SQL]END_DETECT\n"));
} else {
    write(dev_fd->btfd, name_msg, strlen(name_msg));
}

...
```

```c
...

#define SSID "your-ap-ssid"
#define PASS "your-ap-password"
#define LOGID "KEG_ROBOT"
#define PASSWD "PASSWD"
#define DST_IP "your-ap-ip"
#define DST_PORT 8080

...
// 로봇 워커에서는 ESP-8266 와이파이 모듈을 통한 UART기반의 시리얼 통신 수행
int esp_client_conn() {
	char at_cmd[MAX_ESP_COMMAND_LEN] = {0, };
  uint16_t length = 0;
	sprintf(at_cmd,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",DST_IP,DST_PORT);
	esp_at_command((uint8_t *)at_cmd,(uint8_t *)response, &length, 1000);

	esp_send_data("["LOGID":"PASSWD"]");
	return 0;
}
```

```bash
pi@pi14:~/rcp $ ./nmx_channel_run
Server is running on port 8080...
New connection accepted: FD = 6
Tokenized data: pArray[0] = SCM_SQL, pArray[1] = PASSWD
[SCM] New connected! (ip:10.10.14.54, fd:6, sockcnt:1)
[SCM] New connected! (ip:10.10.14.54, fd:6, sockcnt:1)
Authentication successful
1
Conveyer 1 is connected: FD = 6
New connection accepted: FD = 8
Tokenized data: pArray[0] = KEG_ROBOT, pArray[1] = PASSWD
[KEG] New connected! (ip:10.10.14.54, fd:8, sockcnt:2)
[KEG] New connected! (ip:10.10.14.54, fd:8, sockcnt:2)
Authentication successful
2
Robot 1 is connected: FD = 8
Received from Conveyer: 1 1
Received from conveyer: Detect flag=1, Color=1
6, 2, 1, 1
Conveyer thread is done sucessfully.
Robot thread is received sucessfully.
6, 2, 1, 1
Robot thread' machine type is robot.
detect_flag: 1
Write to robot thread Success
Sent PDU to client B
It is first.
Received from Conveyer: 0 2
Received from conveyer: Detect flag=0, Color=2
6, 2, 0, 2
Conveyer thread is done sucessfully.
Robot thread is received sucessfully.
6, 2, 0, 2
Robot thread' machine type is robot.
detect_flag: 0
Flag is 0 or client B is not connected
Write to robot thread Success
...

```

로봇 워커 측에서는 컨베이어로 부터의 물체 탐지 플래그 및 색깔 플래그를 동시에 수신하면, 아래와 같이 로봇을 작동시킵니다.
```bash
Client B connected to server
Enter username: KEG_ROBOT
Enter password: PASSWD
Server response: Authentication successful

# 테스트 출력
Received from server:
Proto Ver: rcp/1.1
Msg Type: COMMAND
Error: 0
Payload Size: 122
Check: 45103
Timestamp: 0
Data Length: 48
Color: 1
Data: ControlMsg
Yaw: 1.00
Pitch: 2.00
Roll: 3.00
X Position: 4.00
Y Position: 5.00
Z Position: 6.00
...
```

<br>

## Step 03. 워커 실행
> 컨베이어 워커와 로봇 워커 실행에 대한 가이드는 본 프로젝트의 [conveyer 디렉토리](https://github.com/pjw1702/Project_SmartFactory/tree/main/conveyor)와 [Robot_arm 디렉토리](https://github.com/pjw1702/Project_SmartFactory/tree/main/Robot_arm)의 README.md 문서를 참고하시길 바랍니다.

<br>

## Step 04. 네트워크 커널 모듈 적재
NexusMesh-X의 커널 모듈은, `ARM-32Bits(Armv7)` CPU 아키텍처와 커널 버전 `6.6.70-v7l+` 에서 빌드되었습니다.

따라서, 본인의 개별 게이트웨이의 OS에서 모듈 적재 시, 반드시 아래와 같이 커널과 CPU 아키텍처 정보를 확인을 해주시길 바랍니다.

```bash
$ uname -a
Linux pi14 6.6.70-v7l+ #1 SMP Sun Jan 12 21:19:39 KST 2025 armv7l GNU/Linux
```

만일 지원하는 버전과 다른 버전의 커널에서 모듈을 적재하려 하면, 아래와 같은 에러가 발생합니다.
```bash
$ cd wifi_health/module
$ sudo insmod nexusmeshx.ko
insmod: ERROR: could not insert module net_dev.ko: Invalid module format
```

적재하기 전, nl80211 및 cfg80211과의 IPC에 필요한 `Nelink` 및 `Generic-Netlikc` 개발자 라이브러리를 먼저 다운로드 받습니다.
```bash 
sudo apt update
sudo apt install libnl-3-dev libnl-genl-3-dev
```

NexusMesh-X의 GPIO 드라이버는 캐릭터 디바이스 파일에 대해 ioctl를 이용한 접근을 기반으로 실행되므로, 아래와 같이 캐릭터 디바이스 파일을 정의된 주 번호 및 부 번호와 함께 생성하여 줍니다.
```bash
$ sudo mknod /dev/nexusmeshx c 230 0
$ sudo chmod 666 /dev/nexusmeshx
```

이후, 모듈을 적재합니다.

```bash
$ cd wifi_health/module
$ sudo insmod nexusmeshx.ko
```

모듈을 잭재한 후, 아래와 같이 애플리케이션을 실행합니다.
+ 커널 타이머 주기 공식: `f=100HZ, T=1/100 = 10ms, 100*10ms = 1Sec`
```bash
$ cd ../
# parameter: [초기화 할 GPIO 점등 수] [커널 타이머 주기 시간, 100=1s] [헬스 체크 주기 시간]
$ cd ./nexusmesh_run 0xff 100 3
```

<br>
<br>

# RCP (Robot Control Protocol) 프로토콜 규격서

## 1. 개요
RCP (Robot Control Protocol)는 로봇과 워커 디바이스 간의 통신을 관리하고 제어하는 프로토콜입니다. 이 프로토콜은 다양한 메시지 유형과 응답 코드, 명령을 정의하여 로봇 제어 시스템과 워커 디바이스 간의 효율적인 데이터 전송을 보장합니다.

<br>

## 2. 메시지 타입
RCP에서 사용하는 메시지 타입은 다음과 같습니다:

| 메시지 타입    | 설명                              |
|----------------|-----------------------------------|
| READY          | 준비 완료 상태                    |
| COMMAND        | 명령 전달                         |
| DONE           | 작업 완료                         |
| CONFIRM        | 확인 요청                         |
| REQUEST        | 요청                              |
| ACK            | 응답 확인                         |

<br>

## 3. 응답 코드

### 3.1. 2xx - 성공 응답 코드

| 응답 코드 | 설명                             |
|-----------|----------------------------------|
| 200       | 준비 완료                        |
| 201       | 작업 완료                        |
| 202       | 준비 완료 확인                  |
| 211       | 매니저 연결됨                    |
| 212       | 워커 연결됨                      |
| 230       | 워커 인증 성공                   |
| 240       | 명령 처리 완료                   |
| 262       | 매니저 전송 성공                 |
| 263       | 매니저 수신 성공                 |
| 262       | 워커 전송 성공                   |
| 263       | 워커 수신 성공                   |

### 3.2. 4xx - 오류 응답 코드

| 응답 코드 | 설명                                      |
|-----------|-------------------------------------------|
| 410       | 준비되지 않음                            |
| 411       | 매니저 연결 끊김                         |
| 412       | 워커 연결 끊김                           |
| 413       | 워커 명령 중단                           |
| 414       | 워커 명령 미실행                         |
| 430       | 일반 오류                               |
| 451       | 매니저 응답 없음                         |
| 452       | 워커 응답 없음                          |
| 462       | 매니저 전송 실패                        |
| 463       | 매니저 수신 실패                        |
| 462       | 워커 전송 실패                          |
| 463       | 워커 수신 실패                          |

### 3.3. 5xx - 명령 관련 오류 응답 코드

| 응답 코드 | 설명                      |
|-----------|---------------------------|
| 500       | 잘못된 명령                |
| 501       | 명령 없음                  |
| 510       | 인증 실패                  |

<br>

## 4. 명령 정의
RCP에서 사용하는 주요 명령은 다음과 같습니다:

| 명령        | 설명                       |
|-------------|----------------------------|
| RCT GREP    | 물체 집기 명령                |
| RCT RELEASE | 집은 물체 놓기 명령            |

## 5. 메시지 구조

### 5.2. 헤더 (Rcphdr)
헤더는 패킷의 메타데이터를 담고 있습니다.
  + 크기: **42 bytes**

| 필드 이름     | 타입           | 설명                          |
|---------------|----------------|-------------------------------|
| proto_ver     | char[8]        | 프로토콜 버전                  |
| msg_type      | char[16]       | 메시지 타입                    |
| err           | uint16_t       | 오류 코드                      |
| p_size        | uint16_t       | 페이로드 크기                  |
| check         | uint16_t       | 체크섬                         |
| timestamp     | uint64_t       | 타임스탬프                     |
| d_len         | uint32_t       | 데이터 길이                    |

<br>

### 5.3. 페이로드 (Rcdata)
페이로드는 로봇 제어 데이터를 담고 있습니다.
  + 크기: **48 bytes**

| 필드 이름     | 타입       | 설명                               |
|---------------|------------|------------------------------------|
| yaw           | float      | 로봇의 요각                         |
| pitch         | float      | 로봇의 피치각                       |
| roll          | float      | 로봇의 롤각                         |
| x_pos         | float      | 로봇의 X 위치                       |
| y_pos         | float      | 로봇의 Y 위치                       |
| z_pos         | float      | 로봇의 Z 위치                       |
| robot_id      | uint8_t    | 로봇 ID                            |
| conveyer_id   | uint8_t    | 컨베이어 ID                        |
| detect_flag   | uint8_t    | 감지 플래그                         |
| color         | uint8_t    | 색상                               |
| data          | char[20]   | 기타 데이터                         |


<br>

### 5.4. PDU (Protocol Data Unit)
PDU는 헤더와 페이로드를 결합한 전체 패킷을 의미합니다.
  + 크기: **90 bytes**

| 필드 이름    | 타입         | 설명                        |
|--------------|--------------|-----------------------------|
| rcp_header   | Rcphdr       | 패킷 헤더                   |
| rc_data      | Rcdata       | 패킷 페이로드               |


<br>

# Links
+ [hostapd Github](https://github.com/latelee/hostapd)
+ [About cfg80211](https://wireless.docs.kernel.org/en/latest/en/developers/documentation/cfg80211.html)
+ [About nl80211](https://wireless.docs.kernel.org/en/latest/en/developers/documentation/nl80211.html)
+ [iperf Github](https://github.com/esnet/iperf)






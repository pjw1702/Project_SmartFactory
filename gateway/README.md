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

![alt text](../img/NexusMeshX.png)

---

<br>

# What is NexusMeshX?

NexusMeshX는 **스마트 팩토리를 위한 고성능 로봇 제어 게이트웨이(AP)** 로, 로봇과 컨베이어 시스템 간의 안정적이고 효율적인 통신을 지원합니다.

본 게이트웨이는 **WiFi5**를 기반으로 설계되었으며, 자체 프로토콜인 **RCP(Robot Control Protocol)** 가 적용되어 비동기 통신 기반의 네트워크 성능 최적화와 통신 제어 기능을 제공합니다.

<br>

# Main Features

1. **네트워크 커널 드라이버**
  + 효율적인 데이터 처리를 위해 최적화된 네트워크 커널 드라이버 탑재.
  + 저지연 비동기 통신 지원으로 실시간 데이터 전송 보장

<br>

2. **서버-클라이언트 기반의 로봇 제어 프로토콜(RCP, Robot Control Protocol) 적용**
  + 비동기 통신 지원
  + 로봇과 컨베이어 시스템 간 통신에서 병목 현상을 줄이고, 데이터 흐름의 효율성을 극대화

<br>

3. **WiFi5 기반의 무선 네트워크 지원**
+ 802.11ac
+ 20Mhz 대역폭 지원
+ 평균 6.8 ~ 7.1 Gbits 대역폭 지원

<br>

4. **실시간 네트워크 상태 모니터링**
  + 와이파이 대역폭 체크
    + 현재 사용 중인 네트워크 대역폭 확인
  + 와이파이 신호 품질 체크
    + 통신 신뢰도를 높이기 위한 신호 강도 및 품질 상태 확인
  + 네트워크 트래픽 부하도 체크
    + 트래픽 과부하 감지 및 네트워크 효율 유지

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

**네트워크 커널 드라이버**
+ [cfg80211 커널 드라이버](https://www.kernel.org/doc/html/v4.12/driver-api/80211/index.html) 연동
  + WIFI5 기반 무선 네트워크 지원
+ [nl80211 라이브러리](https://git.kernel.org/pub/scm/linux/kernel/git/linville/wireless.git/tree/include/uapi/linux/nl80211.h?id=HEAD) 연동
  + 와이파이 모듈의 인터페이스 정보 확인
  + 주파수 기반의 아날로그 대역폭 및 신호 품질 측정
+ [iperf_api 라이브러리](https://github.com/esnet/iperf/blob/master/src/iperf_api.h) 연동
  + iperf3의 라이브러리를 통해 디지털 대역폭 측정
  + iperf3의 라이브러리를 이용하여 현재 송수신 트래픽 비율(부하도) 확인
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

## Step 02. 로봇 제어 네트워크 데몬 실행
데몬은 아래와 같이 실행 합니다.
```bash
$ make
$ ./rcp_manager
```
위와 같이 실행을 하면, 아래와 같이 클라이언트 접속 및 데이터 전송을 실시간으로 확인하실 수 있습니다.

단, 데이터 송수신을 위해서는 인증을 거쳐야 하며, 인증 방식은 `[PJW_SQL:PASSWD]` 또는 `[PJW_ROBOT:PASSWD]`와 같이, 아이디의 `접두사에 컨베이어 측은 SQL, 로봇 측에는 ROBOT`을 반드시 포함하여야 합니다.

그렇지 않으면, 서로 다른 클라이언트에 대한 스레드를 생성할 수가 없어 에러가 발생합니다.

```bash
pi@pi14:~/rcp $ ./rcp_manager
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

로봇 측에서는 컨베이어로 부터의 물체 탐지 플래그 및 색깔 플래그를 동시에 수신하면, 아래와 같이 로봇을 작동시킵니다.
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

## Step 03. 네트워크 커널 드라이버 적재
NexusMeshX의 네트워크 커널 드라이버는, 커널 버전 `6.6.63-v7l+` 에서 빌드되었습니다.

따라서, 본인의 개별 게이트웨이의 OS에서 드라이버 적재 시, 반드시 아래와 같이 커널 버전을 확인을 해주시길 바랍니다.

```bash
$ uname -r
6.6.63-v7l+
```

만일 지원하는 버전과 다른 버전의 커널에서 드라이버를 적재하려 하면, 아래와 같은 에러가 발생합니다.
```bash
$ sudo intsmod net_dev.ko
insmod: ERROR: could not insert module net_dev.ko: Invalid module format
```

적재하기 전, 프로그램 실행에 필요한 라이브러리를 먼저 다운로드 받습니다.
```bash
sudo apt update
sudo apt install iperf3 libiperf-dev
sudo apt install libnl-3-dev libnl-genl-3-dev
```

이후, 드라이버를 적재합니다.

```bash
$ sudo insmod net_dev.ko
```

드라이버를 잭재한 후 아래와 같이 애플리케이션을 실행합니다.
```bash
$ ./net_app ledVal=0xff timeVal=100
```

```bash
gcc -o load_average_test_3 load_average_test_3.c -ljson-c -liperf -L/usr/local/lib -lnl-3 -lnl-genl-3 -I/usr/include/libnl3
```


<br>

# Links
+ [hostapd Github](https://github.com/latelee/hostapd)
+ [About cfg80211](https://wireless.docs.kernel.org/en/latest/en/developers/documentation/cfg80211.html)
+ [About nl80211](https://wireless.docs.kernel.org/en/latest/en/developers/documentation/nl80211.html)
+ [iperf Github](https://github.com/esnet/iperf)






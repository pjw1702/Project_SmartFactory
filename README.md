# 컨베이어 벨트 & 내부통신코드

1. **Color Detector**: 카메라를 이용해 물체 색상을 감지하고 데이터베이스(MariaDB)에 저장.
2. **IR Detection & Bluetooth Communication**: IR 센서 상태와 감지된 색상을 블루투스로 전송.
3. **Server & Database Integration**: 서버에서 데이터베이스를 관리하며, 추가로 STM32 로봇 팔에 작업 명령을 전달.
4. **STM32 Control**: 로봇 팔이 색상 및 상태 데이터를 기반으로 물체를 집어 이동.

---

### 주요 구성 요소 및 동작 원리

#### 1. **Color Detection** (`color-detector.py`)
- **역할**: OpenCV를 사용해 중앙 사각형 영역에서 색상을 분석.
- **세부 동작**:
  - 카메라의 중앙 영역에서 ROI(Region of Interest) 정의.
  - HSV 색 공간을 사용하여 Red, Green, Blue를 감지.
  - 색상 감지 시, 감지 시간을 기록하고 `ObjectDetection` 테이블에 저장.

```python
# 예시: 색상 감지 및 데이터베이스 삽입
color = detect_color(frame)
if color:
    insert_to_db(color)
```

- **데이터베이스 저장 형식**:
  - 컬럼: `Color`, `CameraTimestamp`
  - 예: `Red, 2025-01-01 10:20:30`

---

#### 2. **Bluetooth Communication** (`iot_client_bluetooth.c`)
- **역할**: IR 센서 데이터를 블루투스로 서버에 전송.
- **세부 동작**:
  - IR 감지 시 메시지를 `[SCM_SQL]DETECT` 형식으로 블루투스를 통해 전송.
  - IR 상태가 변경되면 `[SCM_SQL]END_DETECT` 메시지를 전송.
  - 블루투스 연결 상태를 지속적으로 확인하며 서버와 소통.

```c
if (strstr(msg, "DETECT") != NULL) {
    write(dev_fd->btfd, "[SCM_SQL]DETECT\n", strlen("[SCM_SQL]DETECT\n"));
}
```
### 클라이언트를 통해 전송되는 메시지
![alt text](img/iot-client-bluetooth.png)

---

#### 3. **Server & SQL Management** (`iot_client_sql.c`, `iot_server.c`)
- **역할**: 데이터베이스 관리 및 STM32로 명령 전송.
- **SQL 관리**:
  - `ObjectDetection` 테이블에서 IR 상태(`IRStatus`)가 1인 데이터를 조회.
  - 감지된 색상 데이터를 기반으로 `[KEG_STM32]1@<color_code>` 형식의 명령 전송.
  - 작업 완료 메시지(`DONE`) 수신 후 새로운 데이터를 처리.

```c
snprintf(name_msg, sizeof(name_msg), "[KEG_STM32]%d@%d\n", current_status, col);
write(primary_sock, name_msg, strlen(name_msg));
```
- **서버 동작**:
  - 클라이언트 연결 관리 (최대 32개 클라이언트).
  - 메시지 수신 후 각 클라이언트에 전달.

### SQL 클라이언트가 BT에게 수신됨과 동시에 STM에게 송신
![alt text](img/iot-client-sql.png)

### 내부 통신 서버, 모든 메시지가 거쳐간다.
![alt text](img/iot-server.png)
#### 4. **STM32 Control** (`conveyor.ino`)
- **역할**: 색상 및 IR 상태 데이터를 기반으로 로봇 팔 제어.
- **세부 동작**:
  - `[KEG_STM32]<status>@<color_code>` 형식으로 데이터 수신.
  - 색상별로 동작을 구분하며 물체를 집어 지정된 위치로 이동.
  - 작업 완료 시 `[SCM_SQL]DONE` 메시지 송신.

### IR 감지결과 메시지를 실시간으로 송신
![alt text](video/IR센서.gif)
---

### 데이터 흐름

1. **Color Detection**: 색상 감지 후 MariaDB에 데이터 저장.
2. **IR Detection & Bluetooth**: IR 감지 시 데이터베이스 업데이트 및 블루투스 전송.
3. **Server**: 블루투스 메시지를 수신 후 STM32에 전달.
4. **STM32**: 로봇 팔이 동작하고 완료 시 메시지를 다시 서버로 전송.

---

### 실행 방법

1. **Color Detector**:
   ```bash
   python3 color-detector.py
   ```

2. **Bluetooth Client**:
   ```bash
   ./iot_client_bluetooth <Server_IP> <Port> <Device_Name>
   ```

3. **SQL Client**:
   ```bash
   ./iot_client_sql <Server_IP> <Port> <Device_Name>
   ```

4. **Server**:
   ```bash
   ./iot_server <Port>
   ```

5. **STM32**:
   - `conveyor.ino` 코드를 Arduino IDE를 통해 업로드.


### 주요 테이블 스키마

```sql
CREATE TABLE ObjectDetection (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    Color VARCHAR(10),
    CameraTimestamp DATETIME,
    IRStatus TINYINT(1),
    IRTimestamp DATETIME
);
```
![alt text](img/sql테이블.png)

### 오류 처리

1. **Camera Error**:
   - 카메라가 감지되지 않을 경우 OpenCV에서 에러 메시지 출력.
   - 해결: 카메라 연결 상태 확인 후 재실행.

2. **Bluetooth Connection Error**:
   - 블루투스 소켓 생성 실패 시 에러 메시지 출력.
   - 해결: 블루투스 장치 및 설정 확인.

3. **Database Error**:
   - SQL 쿼리 실패 시 상세 에러 메시지 출력.
   - 해결: MariaDB 연결 및 테이블 상태 확인.
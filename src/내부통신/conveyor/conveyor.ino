#include <SoftwareSerial.h>

#define ENA 5  // 모터 속도 제어 핀
#define IN1 6  // 모터 방향 제어 핀 1
#define IN2 7  // 모터 방향 제어 핀 2
#define IR 8   // IR 센서 입력 핀
#define ARR_CNT 5
#define CMD_SIZE 60

SoftwareSerial BTSerial(10, 11); // RX | TX

char sendBuf[CMD_SIZE];
bool prevIRState = false;
char recvId[10] = "SCM_SQL";  

void setup() {
    Serial.begin(115200);
    BTSerial.begin(9600);
    pinMode(IR, INPUT);  
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT); 
    pinMode(IN2, OUTPUT);
}

void loop() {
    bool currentIRState = digitalRead(IR); 

    if (currentIRState) { 
        digitalWrite(IN1, HIGH);    // 모터 전진
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 90);       // 모터 속도 설정

        if (prevIRState != currentIRState) {  // 상태가 변경된 경우
            sprintf(sendBuf, "[%s]END_DETECT\n", recvId);
            BTSerial.write(sendBuf);         // 블루투스로 전송
            Serial.println(sendBuf);         // 디버깅용 출력
            prevIRState = currentIRState;    // 상태 업데이트
        }
    } else {  // IR 센서가 물체를 감지한 경우
        digitalWrite(IN1, LOW);     // 모터 정지
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);

        if (prevIRState != currentIRState) {  // 상태가 변경된 경우
            sprintf(sendBuf, "[%s]DETECT\n", recvId);
            BTSerial.write(sendBuf);         // 블루투스로 전송
            Serial.println(sendBuf);         // 디버깅용 출력
            prevIRState = currentIRState;    // 상태 업데이트
        }
    }
}
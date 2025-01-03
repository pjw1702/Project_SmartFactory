import cv2
import time
from ultralytics import YOLO
import pymysql
from datetime import datetime

model = YOLO("best.pt")

TARGET_CLASSES = [0, 1, 2] 
CONFIDENCE_THRESHOLD = 0.5 

# MariaDB 연결 설정
conn = pymysql.connect(
    host='localhost',
    port=3306,  # 기본 포트
    user='iot',
    password='pwiot',
    database='iotdb'
)

cursor = conn.cursor()

last_detection_time = 0  # 마지막 감지 시간 초기화
last_detection = None  # 마지막 감지된 객체 초기화

# DB 삽입 함수
def insert_to_db(color):
    timestamp = datetime.now()
    query = "INSERT INTO ObjectDetection (Color, CameraTimestamp) VALUES (%s, %s)"
    cursor.execute(query, (color, timestamp))
    conn.commit()
    print(f"Inserted {color} at {timestamp}")

# 웹캠 시작
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

print("Press 'q' to quit.")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        break

    # 프레임 크기 가져오기
    height, width, _ = frame.shape

    # 중앙 바운딩 박스 좌표 계산
    BOX_WIDTH = 200
    BOX_HEIGHT = 200
    x1 = width // 2 - BOX_WIDTH // 2
    y1 = height // 2 - BOX_HEIGHT // 2
    x2 = width // 2 + BOX_WIDTH // 2
    y2 = height // 2 + BOX_HEIGHT // 2

    # 원본 프레임에 중앙 바운딩 박스 그리기
    cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 255, 255), 2)

    # 바운딩 박스 내부 ROI 추출
    roi = frame[y1:y2, x1:x2]

    # 현재 시간 가져오기
    current_time = time.time()

    # 추론 수행 (0.5초 간격)
    if current_time - last_detection_time > 0.5:
        results = model.predict(
            roi,
            verbose=False,
            imgsz=(640, 480),  # 고정된 이미지 크기
            device="cuda"  # CUDA 사용
        )

        for result in results:
            for box in result.boxes.data.tolist():
                # YOLO 결과에서 박스 정보 추출
                x1_pred, y1_pred, x2_pred, y2_pred, confidence, class_id = map(float, box[:6])
                class_id = int(class_id)

                # 관심 있는 클래스와 Confidence 필터링
                if class_id in TARGET_CLASSES and confidence >= CONFIDENCE_THRESHOLD:
                    detected_class = model.names[class_id]
                    print(f"Detected: {detected_class} with confidence {confidence:.2f}")

                    # DB에 감지 결과 삽입
                    insert_to_db(detected_class)

                    # 라벨을 화면에 표시
                    label = f"{detected_class}: {confidence:.2f}"
                    cv2.putText(frame, label, (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

                    # 마지막 감지 시간 업데이트
                    last_detection_time = current_time

    # 화면에 표시
    cv2.imshow("YOLO Inference", frame)

    # 'q' 키를 누르면 종료
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 리소스 해제
cap.release()
cv2.destroyAllWindows()
cursor.close()
conn.close()

import cv2
import numpy as np
import pymysql
from datetime import datetime
import time

# MariaDB 연결 설정
conn = pymysql.connect(
    host='localhost',
    port=3306,  # 기본 포트
    user='iot',
    password='pwiot',
    database='iotdb'
)

# Cursor 생성
cursor = conn.cursor()

# Global variables to track the detection state of each color
last_detection_time = 0  # 마지막 감지 시간 초기화

# K-Means 클러스터링 함수
def detect_dominant_color(roi, k=3):
    pixels = roi.reshape(-1, 3)
    pixels = np.float32(pixels)

    # K-Means 클러스터링 설정
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.2)
    _, labels, centers = cv2.kmeans(pixels, k, None, criteria, 10, cv2.KMEANS_RANDOM_CENTERS)

    # 가장 큰 클러스터의 색상 반환
    dominant_color = centers[np.argmax(np.bincount(labels.flatten()))]
    return dominant_color

# 색상 감지 함수
def detect_color(frame):
    global last_detection_time

    # Get frame dimensions
    height, width, _ = frame.shape

    # Define the size of the square (e.g., 200x200 pixels)
    square_size = 120

    # Calculate the coordinates of the square's top-left and bottom-right corners
    x1 = width // 2 - square_size // 2
    y1 = height // 2 - square_size // 2
    x2 = width // 2 + square_size // 2
    y2 = height // 2 + square_size // 2

    # Draw the square on the frame for visualization
    cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 255, 255), 2)

    # Extract the ROI (Region of Interest)
    roi = frame[y1:y2, x1:x2]

    # Apply Gaussian Blur to reduce noise
    roi = cv2.GaussianBlur(roi, (5, 5), 0)

    # Detect dominant color using K-Means
    dominant_color = detect_dominant_color(roi)

    # Convert BGR to HSV for better color range handling
    dominant_hsv = cv2.cvtColor(np.uint8([[dominant_color]]), cv2.COLOR_BGR2HSV)[0][0]

    # Define color thresholds
    if 0 <= dominant_hsv[0] <= 10 and dominant_hsv[1] > 150 and dominant_hsv[2] > 150:
        return "red"
    elif 50 <= dominant_hsv[0] <= 70 and dominant_hsv[1] > 100 and dominant_hsv[2] > 100:
        return "green"
    elif 110 <= dominant_hsv[0] <= 130 and dominant_hsv[1] > 100 and dominant_hsv[2] > 100:
        return "blue"

    return None

# DB에 색상 데이터 삽입 함수
def insert_to_db(color):
    timestamp = datetime.now()
    query = "INSERT INTO ObjectDetection (Color, CameraTimestamp) VALUES (%s, %s)"
    cursor.execute(query, (color, timestamp))
    conn.commit()
    print(f"Inserted {color} at {timestamp}")

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

    # 현재 시간 가져오기
    current_time = time.time()

    # Detect colors in the frame's center square
    if current_time - last_detection_time > 0.5:  # 마지막 감지 시간과 0.5초 조건 확인
        color = detect_color(frame)
        if color:
            insert_to_db(color)
            cv2.putText(frame, f"Detected: {color}", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            last_detection_time = current_time  # 마지막 감지 시간 업데이트

    # Display the webcam feed
    cv2.imshow("Webcam Feed", frame)

    # Break the loop if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the webcam and close the OpenCV windows
cap.release()
cv2.destroyAllWindows()
cursor.close()
conn.close()


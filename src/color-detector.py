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
red_detected = False
green_detected = False
blue_detected = False
last_detection_time = 0  # 마지막 감지 시간 초기화

def detect_color(frame):
    global red_detected, green_detected, blue_detected

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

    # Convert the ROI to HSV
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)

    # Define HSV color ranges for Red, Green, and Blue
    red_lower = np.array([0, 150, 150])  # Pure red (lower range)
    red_upper = np.array([5, 255, 255])

    green_lower = np.array([55, 110, 110])  # Pure green
    green_upper = np.array([65, 255, 255])

    blue_lower = np.array([115, 170, 170])  # Pure blue
    blue_upper = np.array([125, 255, 255])

    # Create masks for each color within the ROI
    red_mask = cv2.inRange(hsv, red_lower, red_upper)
    green_mask = cv2.inRange(hsv, green_lower, green_upper)
    blue_mask = cv2.inRange(hsv, blue_lower, blue_upper)

    # Check if colors are detected in the ROI
    if cv2.countNonZero(red_mask) > 0:
        if not red_detected:  # If red is detected for the first time
            color = 'red'
            print("Red detected in the square!")
            red_detected = True
            return color
    else:
        red_detected = False  # Reset if red is no longer detected

    if cv2.countNonZero(green_mask) > 0:
        if not green_detected:  # If green is detected for the first time
            color = 'green'
            print("Green detected in the square!")
            green_detected = True
            return color
    else:
        green_detected = False  # Reset if green is no longer detected

    if cv2.countNonZero(blue_mask) > 0:
        if not blue_detected:  # If blue is detected for the first time
            color = 'blue'
            print("Blue detected in the square!")
            blue_detected = True
            return color
    else:
        blue_detected = False  # Reset if blue is no longer detected

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


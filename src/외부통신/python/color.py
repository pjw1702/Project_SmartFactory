import cv2
from ultralytics import YOLO

# YOLO 모델 로드
model = YOLO("color.pt")

# 웹캠 열기
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("웹캠을 열 수 없습니다.")
    exit()

roi_x, roi_y, roi_width, roi_height = 265, 115, 200, 200

already_detected = []

def is_new_object(obj_bbox, detected_list, threshold=50):
    x, y, w, h = obj_bbox
    cx, cy = x + w // 2, y + h // 2
    for detected in detected_list:
        dx, dy = detected
        if abs(cx - dx) < threshold and abs(cy - dy) < threshold:
            return False
    return True

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        print("프레임을 읽을 수 없습니다.")
        break

    cv2.rectangle(frame, (roi_x, roi_y), (roi_x + roi_width, roi_y + roi_height), (0, 255, 0), 2)

    roi_frame = frame[roi_y:roi_y + roi_height, roi_x:roi_x + roi_width]
    results = model.predict(roi_frame, imgsz=640, verbose=False)

    for result in results[0].boxes:
        x1, y1, x2, y2 = map(int, result.xyxy[0])
        label = result.cls
        confidence = result.conf

        bbox = (x1, y1, x2 - x1, y2 - y1)
        if is_new_object(bbox, already_detected):
            cx, cy = x1 + (x2 - x1) // 2, y1 + (y2 - y1) // 2
            already_detected.append((cx, cy))

            print(f"Detected: {label}, Confidence: {float(confidence):.2f}")

    cv2.imshow("YOLO Detection", frame)

    if cv2.waitKey(1) == 27:
        break

cap.release()
cv2.destroyAllWindows()


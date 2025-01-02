from ultralytics import YOLO

# 모델 로드
model = YOLO("color.pt")

model.export(
        format="engine", half=True, data="data.yaml", imgsz=(640, 480), workspace=1024
)

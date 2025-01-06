CC = gcc
# 컴파일 최적화
CFLAGS = -Wall -Wextra -g -O0 -I/usr/include/libnl3 -L/usr/local/lib
TARGET = wifi_check

RM = rm -rf

# 소스 파일 목록 - 여러 폴더에 있는 소스 파일을 포함
SRC = main.c \
      wifi_check.c \

# 오브젝트 파일 목록 (.c -> .o)
OBJ = $(SRC:.c=.o)

# 링커 플래그
LDFLAGS = -ljson-c -liperf -lnl-3 -lnl-genl-3

# 기본 목표: 실행 파일 생성
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# 각 .o 파일 생성 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 청소: 생성된 .o 파일 및 실행 파일 제거
clean:
	rm -rf $(OBJ) $(TARGET)

# 실행 목표 (clean 후 실행)
run: $(TARGET)
	./$(TARGET)

.PHONY: clean run

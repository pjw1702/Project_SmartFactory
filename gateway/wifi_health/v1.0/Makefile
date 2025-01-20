# 사용자 애플리케이션 명
APP := nexusmeshx_run
# 모니터링 애플리케이션 명
MON := nexusmeshx_mon
# 최종 모듈명
MOD := nexusmeshx
# 최종 모듈 오브젝트 파일
obj-m := $(MOD).o
# 크로스 컴파일 옵션
CROSS = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
# 크로스 컴파일에 빌요한 빌드 툴
CC := arm-linux-gnueabihf-gcc
# 모듈 빌드 시 참조할 커널 경로
KDIR := /home/ubuntu/pi_bsp/kernel/linux

# 소스 경로 설정
APP_PATH := $(PWD)/app
MON_PATH := $(PWD)/app/monitoring
MOD_PATH := $(PWD)/module
UTIL_PATH := $(PWD)/utils

# 최종 모듈 빌드에 필요한 의존성 소스 파일
MOD_SRC = \
    $(MOD_PATH)/nexusmeshx_module.c \
    $(MOD_PATH)/driver/gpio/led_switch.c \
    $(MOD_PATH)/driver/ioctl/ioctl.c \
    $(MOD_PATH)/driver/irq/irq_switch.c \
    $(MOD_PATH)/driver/timer/timer.c \
    $(MOD_PATH)/model/module_data.c

# 최종 모듈 빌드에 필요한 의존성 오브젝트 파일
$(MOD)-y := \
    $(MOD_PATH)/nexusmeshx_module.o \
    $(MOD_PATH)/driver/gpio/led_switch.o \
    $(MOD_PATH)/driver/ioctl/ioctl.o \
    $(MOD_PATH)/driver/irq/irq_switch.o \
    $(MOD_PATH)/driver/timer/timer.o \
    $(MOD_PATH)/model/module_data.o

# 모든 .c 파일을 .o 파일로 변환
OBJ := $(MOD_SRC:.c=.o)

# 사용자 애플리케이션 빌드에 필요한 의존성 소스 파일
APP_SRC := \
   $(APP_PATH)/nexusmeshx_run.c \
   $(APP_PATH)/nexusmeshx_app.c \
   $(UTIL_PATH)/wifi/wifi_check.c \
   $(UTIL_PATH)/wifi/wifi_init.c \

MON_SRC := \
   $(MON_PATH)/nexusmeshx_mon.c \
   $(UTIL_PATH)/wifi/wifi_init.c \
   $(UTIL_PATH)/wifi/wifi_check.c \

#EXTRA_CFLAGS += -I$(KDIR)/include -I$(DRV_SRC_DIR) -I$(UTIL_SRC_DIR)

# 해당 모듈에 대한 커널 라이브러리 경로 설정
EXTRA_CFLAGS += -I$(KDIR)/include

# 컴파일에 필요한 라이브러리 플래그 설정
CFLAGS = -Wall -Wextra -g -O0 -I/usr/include/libnl3 -I$(APP_PATH) -I$(UTIL_PATH) -I/usr/include/lpthread
# 컴파일에 필요한 링커 플래그 설정
LDFLAGS = -L/usr/lib/arm-linux-gnueabihf/ldscripts -ljson-c -liperf -lnl-3 -lnl-genl-3 -lpthread -lssl -lcrypto -lm -lsctp -latomic -ldl -lgcc

# 기본 타겟 정책 설정: 앱과 모듈 모두 빌드
default: $(APP) $(MON) modules

# 사용자 앱 빌드
$(APP):
	$(CC) $(CFLAGS) $(APP_SRC) -o $(APP) $(LDFLAGS)
	sudo cp $(APP) /srv/nfs/drivers

# 모니터링 앱 빌드
$(MON):
	$(CC) $(CFLAGS) $(MON_SRC) -o $(MON) $(LDFLAGS)
	sudo cp $(MON) /srv/nfs/drivers

# 모듈 빌드
modules:
	$(MAKE) -C $(KDIR) M=$(MOD_PATH) modules $(CROSS)
	sudo cp $(MOD_PATH)/$(MOD).ko /srv/nfs/drivers

# 클린
clean:
	rm -rf $(MOD_PATH)/*.ko
	rm -rf $(MOD_PATH)/*.mod.*
	rm -rf $(MOD_PATH)/.*.cmd
	rm -rf $(MOD_PATH)/*.o
	find . -type f -name "*.o*" -exec rm -f {} \;
	rm -rf $(MOD_PATH)/modules.order
	rm -rf $(MOD_PATH)/Module.symvers
	rm -rf $(MOD_PATH)/$(MOD).mod
	rm -rf $(MOD_PATH)/.tmp_versions
	rm -rf $(APP)
	rm -rf $(MON)
	rm -rf $(MOD_PATH)/*.o 

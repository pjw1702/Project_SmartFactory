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

# 최종 모듈 빌드에 필요한 의존성 소스 파일
DRV_SRC = nexusmeshx_module.c \
    driver/gpio/led_switch.c \
    driver/ioctl/ioctl.c \
    irq/irq_switch.c \
    driver/timer/timer.c \
    model/module_data.c

# 최종 모듈 빌드에 필요한 의존성 오브젝트 파일
$(MOD)-y := nexusmeshx_module.o \
    driver/gpio/led_switch.o \
    driver/ioctl/ioctl.o \
    irq/irq_switch.o \
    driver/timer/timer.o \
    model/module_data.o

# .c 파일을 .o 파일로 변환
OBJ := $(DRV_SRC:.c=.o)

# 해당 모듈에 대한 커널 라이브러리 경로 설정
EXTRA_CFLAGS += -I$(KDIR)/include

# 해당 모듈 빌드에 필요한 링커 플래그 설정
LDFLAGS = -L/usr/lib/arm-linux-gnueabihf/ldscripts -ljson-c -liperf -lnl-3 -lnl-genl-3 -lpthread -lssl -lcrypto -lm -lsctp -latomic -ldl -lgcc

# 모듈 빌드
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules $(CROSS)
	sudo cp $(PWD)/$(MOD).ko /srv/nfs/drivers

# 클린 타겟
clean:
	rm -f $(OBJ)
	rm -f $(obj-m:.o=.ko)
	rm -f $(MOD).o
	find . -type f -name "*.o.cmd" -exec rm -f {} \;
	find . -type f -name "*.o.d" -exec rm -f {} \;
	rm -f *.mod *.mod.* 
	rm -f .*.cmd *.o.d
	rm -f modules.order
	rm -f Module.symvers
	rm -rf .tmp_versions
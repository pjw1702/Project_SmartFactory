APP := net_app
MOD := net_dev
SRC := $(APP).c
obj-m := $(MOD).o

CROSS = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
CC := arm-linux-gnueabihf-gcc
CFLAGS = -Wall -Wextra -g -O0 -I/usr/include/libnl3 -L/usr/local/lib
KDIR := /lib/modules/6.6.67-v7l+/
PWD := $(shell pwd)
SRC := $(APP).c wifi/wifi_check.c

# 링커 플래그
LDFLAGS = -ljson-c -liperf -lnl-3 -lnl-genl-3

default:$(APP)
	$(MAKE) -C $(KDIR) M=$(PWD) modules $(CROSS)
	# sudo cp $(MOD).ko /srv/nfs/drivers

$(APP):
	$(CC) $(CFLAGS) $(SRC) -o $(APP) $(LDFLAGS)
	# sudo cp $(APP) /srv/nfs/drivers

clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf modules.order
	rm -rf Module.symvers
	rm -rf $(MOD).mod
	rm -rf .tmp_versions
	rm -rf $(APP)

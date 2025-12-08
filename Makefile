# Указание kernel build system файла с кодом
obj-m += wireless_usb.o
# obj-m := module.o
# module-objs := file1.o file2.o

PWD := $(CURDIR)
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

# make -C lib/modules/fedora/build M=$pwd modules - 
# перейти в kernel build, вызвать у Makefile оттуда таргет
# "modules", который выполнится в папке по пути M=<path>, т.е. здесь
# 
# $(MAKE) - параметр для вызова make во второй раз из kernel
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

# Поместить модуль в kernel. Для этого требуется отключить Secure Boot
# sudo mokutil --disable-validation, а затем перезагрузиться. Но это 
# первый попавшийся дурацкий метод, и я еще огребу из-за этого
install: all
	sudo insmod wireless_usb.ko

remove:
	sudo rmmod wireless_usb

reload: remove install

test: install
	dmesg | tail -3
	sleep 2
	sudo rmmod wireless_usb
	dmesg | tail -3

.PHONY: all clean install remove reload test
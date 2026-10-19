obj-m += os04c10.o

KDIR ?= $(HOME)/luckfox-pico/sysdrv/source/kernel

ccflags-y += -I$(KDIR)/drivers/phy/rockchip
ccflags-y += -I$(KDIR)/drivers/media/platform/rockchip/cif

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

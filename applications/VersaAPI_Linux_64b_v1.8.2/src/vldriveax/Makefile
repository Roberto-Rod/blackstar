
KERNEL_DIR	:= `uname -r`

# Should be a link to the appropriate kernel source files.
KDIR=/lib/modules/$(KERNEL_DIR)/build

DRIVER_DIR	:= /lib/modules/$(KERNEL_DIR)
DRIVER_SYSD	:= /etc/modules-load.d/modules.conf
DRIVER_SYSV := /etc/modules

obj-m	:= vldriveax.o

PWD	:= $(shell pwd)

default: 
	$(MAKE) -C $(KDIR) M=$(PWD)

install:
	@echo '$@: Installing VersaLogic PCI A1/A2 driver vldriveax ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldriveax.ko
	cp vldriveax.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSD}'
	touch ${DRIVER_SYSD}
	sed -i '/^vldriveax/d' ${DRIVER_SYSD}
	echo "vldriveax" >> "${DRIVER_SYSD}"
	depmod -a
	@echo '$@: ... Done.'

install5:
	@echo '$@: Installing VersaLogic PCI A1/A2 driver vldriveax ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldriveax.ko
	cp vldriveax.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSV}'
	touch ${DRIVER_SYSV}
	sed -i '/^vldriveax/d' ${DRIVER_SYSV}
	echo "vldriveax" >> "${DRIVER_SYSV}"
	depmod -a
	@echo '$@: ... Done.'

clean:
	-$(RM) -r *.ko *.o *.mod.* .*.cmd .tmp* .cache.mk modules.order Module.symvers .vldriveax.ko.cmd vldriveax.mod.c .vldriveax.o.cmd

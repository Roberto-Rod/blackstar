
KERNEL_DIR	:= `uname -r`

# Should be a link to the appropriate kernel source files.
KDIR=/lib/modules/$(KERNEL_DIR)/build

DRIVER_DIR	:= /lib/modules/$(KERNEL_DIR)
DRIVER_SYSV	:= /etc/modules
DRIVER_SYSD := /etc/modules-load.d/modules.conf

obj-m	:= vldrivep.o

PWD	:= $(shell pwd)

default: 
	$(MAKE) -C $(KDIR) M=$(PWD)

install:
	@echo '$@: Installing VersaLogic PCI driver vldrivep ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldrivep.ko
	cp vldrivep.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSD}'
	sed -i '/^vldrivep/d' ${DRIVER_SYSD}
	echo "vldrivep" >> "${DRIVER_SYSD}"
	depmod -a
	@echo '$@: ... Done.'
	
install5:
	@echo '$@: Installing VersaLogic PCI driver vldrivep ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldrivep.ko
	cp vldrivep.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSV}'
	sed -i '/^vldrivep/d' ${DRIVER_SYSV}
	echo "vldrivep" >> "${DRIVER_SYSV}"
	depmod -a
	@echo '$@: ... Done.'

clean:
	-$(RM) -r *.ko *.o *.mod.* .*.cmd .tmp* .cache.mk modules.order Module.symvers .vldrive.ko.cmd vldrive.mod.c .vldrive.o.cmd

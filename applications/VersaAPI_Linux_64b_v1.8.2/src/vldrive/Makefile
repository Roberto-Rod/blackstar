
KERNEL_DIR	:= `uname -r`

# Should be a link to the appropriate kernel header files.
KDIR=/lib/modules/$(KERNEL_DIR)/build

DRIVER_DIR	:= /lib/modules/$(KERNEL_DIR)
DRIVER_SYSD	:= /etc/modules-load.d/modules.conf
DRIVER_SYSV	:= /etc/modules

obj-m	:= vldrive.o

PWD	:= $(shell pwd)


default: 
	$(MAKE) -C $(KDIR) M=$(PWD)

# Build targets.
#if [ -d "/usr/src/kernels" ]; then 
build_0xC80:
	@echo '$@: Building VersaLogic base driver vldrive ...'
	@echo '$@: Creating vldrive.h from vldrive.h.template ...'
	sed 's/VL_FPGA_ADDR/0xC80/g' vldrive.h.template > vldrive.h
	$(MAKE) -C $(KDIR) M=$(PWD)
	@echo '$@: ... Done.'

build_0x1C80:
	@echo '$@: Building VersaLogic base driver vldrive ...'
	@echo '$@: Creating vldrive.h from vldrive.h.template ...'
	sed 's/VL_FPGA_ADDR/0x1C80/g' vldrive.h.template > vldrive.h
	$(MAKE) -C $(KDIR) M=$(PWD)
	@echo '$@: ... Done.'

build_0xD80:
	@echo '$@: Building VersaLogic base driver vldrive ...'
	@echo '$@: Creating vldrive.h from vldrive.h.template ...'
	sed 's/VL_FPGA_ADDR/0xD80/g' vldrive.h.template > vldrive.h
	$(MAKE) -C $(KDIR) M=$(PWD)
	@echo '$@: ... Done.'


# Install targets.
install5: 
	@echo '$@: Installing VersaLogic base driver vldrive ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldrive.ko
	cp vldrive.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSV}'
	touch ${DRIVER_SYSV}
	sed -i '/^vldrive/d' ${DRIVER_SYSV}
	echo "vldrive" >> "${DRIVER_SYSV}"
	depmod -a
	@echo '$@: ... Done.'

install:
	@echo '$@: Installing VersaLogic base driver vldrive ...'
	@echo '$@: Copying driver to ${DRIVER_DIR}'
	chmod a+x vldrive.ko
	cp vldrive.ko ${DRIVER_DIR}
	@echo '$@: Adding the driver name to ${DRIVER_SYSD}'
	touch ${DRIVER_SYSD}
	sed -i '/^vldrive/d' ${DRIVER_SYSD}
	echo "vldrive" >> "${DRIVER_SYSD}"
	depmod -a
	@echo '$@: ... Done.'

clean:
	-$(RM) -r *.ko *.o *.mod.* .*.cmd .tmp* .cache.mk modules.order Module.symvers .vldrive.ko.cmd vldrive.h vldrive.mod.c .vldrive.o.cmd

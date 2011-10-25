###############################################################################
# for kernel modeule level driver:

# Kernel directory
KDIR		:= /lib/modules/$(shell uname -r)/build
# Module directory
MODDIR		:= /lib/modules/$(shell uname -r)/kernel/drivers/sensoray

# System values
PWD		:= $(shell pwd)
KERNEL_24	:= $(if $(wildcard $(KDIR)/Rules.make),1,0)

# Target file
obj-m		:= s626.o


# Source files
ifeq	($(KERNEL_24),0) # > 2.4
s626-objs	:= s626drv.o 
else # <= 2.4
s626-objs	:= s626drv.o
endif

.PHONY:		all clean modules_install

ifeq	($(KERNEL_24),0) # > 2.4
ifeq	($(KERNELRELEASE),)
all:
		$(MAKE) -C $(KDIR) M=$(PWD) SUBDIRS=$(PWD)
clean modules_install:
		$(MAKE) -C $(KDIR) M=$(PWD) SUBDIRS=$(PWD) $@
endif	# KERNELRELEASE

else 	# <= 2.4

ifneq	($(KERNELRELEASE),)

include	$(KDIR)/Rules.make

s626.o:	$(s626-objs)
		$(Q)$(LD) $(LD_RFLAG) -r -o $@ $(s626-objs)
else

all:
		$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
		rm -f *.ko *.o .*.cmd .*.o.flags *.mod.c

endif 	# KERNELRELEASE

endif 	# KERNEL_24

ifeq	($(KERNEL_24),1) # <= 2.4

install:		s626.o
	@if [ -d /lib/modules/$(shell uname -r)/kernel/drivers/sensoray/ ];\
	then rm -f /lib/modules/$(shell uname -r)/kernel/drivers/sensoray/s626.*;\
	fi
	@if [ -d /lib/modules/$(shell uname -r)/extra/ ];\
	then rm -f /lib/modules/$(shell uname -r)/extra/s626.*;\
	fi
				su -c "set -x;./MAKEDEV;mkdir -p $(MODDIR);cp -v s626.o $(MODDIR);depmod -a"

else
install:		s626.ko
	@if [ -d /lib/modules/$(shell uname -r)/kernel/drivers/sensoray/ ];\
	then rm -f /lib/modules/$(shell uname -r)/kernel/drivers/sensoray/s626.*; \
	fi
	@if [ -d /lib/modules/$(shell uname -r)/extra/ ];\
	then rm -f /lib/modules/$(shell uname -r)/extra/s626.*;\
	fi
			su -c "set -x;./MAKEDEV;mkdir -p $(MODDIR);cp -v s626.ko $(MODDIR);install -m 444 s626.ko $(MODDIR);depmod -a"
endif  # KERNEL > 2.4


###############################################################################
# for library and application level:
SRC=			# module source tree
CC=			# compiler
AR=			# library manager

# source path
ifeq ($(strip $(SRC)),)
	SRC=.
endif

# the compiler
ifeq ($(strip $(CC)),)
	CC=gcc
endif

# the library manager
ifeq ($(strip $(AR)),)
	AR=ar
endif

# build the distribution's library
lib:		s626core.o s626mod.o
		$(AR) cr libs626.a $(SRC)/s626core.o $(SRC)/s626mod.o

# compile the 626 universal core
core:		s626core.o
s626core.o:	$(SRC)/s626core.c $(SRC)/s626core.h $(SRC)/s626mod.h
		$(CC) $(CFLAGS) -c $(SRC)/s626core.c

# build the 626 OS-dependent module
mod:		s626mod.o
s626mod.o:	$(SRC)/s626mod.c $(SRC)/s626core.h $(SRC)/s626mod.h $(SRC)/s626drv.h $(SRC)/s626.h $(SRC)/s626api.h
		$(CC) $(CFLAGS) -c $(SRC)/s626mod.c

#----------------------------------------------------------------
# Demonstration programs
# type "make demo" to compile, link, and create demo application
demo:		libs626.a s626demo.o s626dm2b.o
		$(CC) -o s626demo s626demo.o -L$(SRC) -ls626 -lpthread
		$(CC) -o s626dm2b s626dm2b.o -L$(SRC) -ls626 -lpthread

s626demo.o:	$(SRC)/s626demo.c $(SRC)/s626core.h $(SRC)/s626mod.h $(SRC)/s626drv.h $(SRC)/s626api.h $(SRC)/App626.h
		$(CC) $(CFLAGS) -c $(SRC)/s626demo.c

s626dm2b.o:	$(SRC)/s626dm2b.c $(SRC)/s626core.h $(SRC)/s626mod.h $(SRC)/s626drv.h $(SRC)/s626api.h $(SRC)/App626.h
		$(CC) $(CFLAGS) -c $(SRC)/s626dm2b.c

# for debugging:
#     gcc -g -o s626demo s626core.c s626mod.c s626demo.c -lpthread
#     gcc -g -o s626dm2b s626core.c s626mod.c s626dm2b.c -lpthread

clnall:
		rm -f *.ko *.o .*.cmd .*.o.flags *.mod.c libs626.a s626demo s626dm2b *.tar.gz


###############################################################################
# for internal develoment only
#----------------------------------------------------------------
# select files for the distribution tarball
T =  $(SRC)/README
T += $(SRC)/COPYING
T += $(SRC)/MAKEDEV
T += $(SRC)/Modules.conf
T += $(SRC)/Makefile
T += $(SRC)/s626drv.h
T += $(SRC)/s626drv.c
T += $(SRC)/s626.h
T += $(SRC)/s626mod.h
T += $(SRC)/s626mod.c
T += $(SRC)/s626core.h
T += $(SRC)/s626core.c
T += $(SRC)/s626api.h
T += $(SRC)/App626.h
T += $(SRC)/libs626.a
T += $(SRC)/s626demo.c
T += $(SRC)/s626dm2b.c

#----------------------------------------------------------------
# build distribution tarball (only for development)
tar:
#		tar -Pzcf s626-0.3.tar.gz $(T)
		tar -czvf s626-1.0.tar.gz $(T)

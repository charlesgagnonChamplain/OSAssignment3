####
#
# Makefile
#
# Main makefile to build: - ASCII character device driver
#                         - main() to test the driver
#
# Serguei Mokhov
#
##

CC=gcc
DEBUG=-g -D_DEBUG
DEFINE=-DMODULE -D__KERNEL__ -DLINUX
WARNINGS=-Wall -Wmissing-prototypes -Wmissing-declarations
#ISO=-ansi -pedantic
CC_OPTIONS=-O1 $(WARNINGS) $(ISO) $(DEBUG) $(DEFINE)

# Where to look for header files
INC=-I. -I/usr/include -I/usr/src/kernels/`uname -r`/include

DRIVER=ascii.o
MODULE=ascii.ko
EXE=ascii-test
OBJ=mapdriver.o $(DRIVER)

obj-m += $(DRIVER)

all: $(EXE)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	@echo ""
	@echo "ASCII Character Device Driver and the Test Program have been built."
	@echo "Type 'make register' to register ASCII device module."
	@echo "Then follow module's suggestion to mknod /dev/asciimap with the correct major number!"
	@echo "EXTREME CAUTION IS HIGHLY RECOMMENDED. NOT TO USE ON YOUR OWN"
	@echo "MACHINE OR NUCLEAR POWER PLANTS :) THERE MAY BE CONSEQUENCES"
	@echo ""

clean:
	rm -f $(EXE) $(OBJ)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

compile: $(EXE) $(OBJ)

register: $(DRIVER)
	insmod ./$(MODULE)
	modinfo $(MODULE)
	lsmod | grep ascii
	@echo ""
	@echo "ASCII Character Device Driver has been built and registered."
	@echo ""

$(EXE): mapdriver.o
	$(CC) mapdriver.o -o $(EXE)

mapdriver.o: mapdriver.c common.h
	$(CC) $(CC_OPTIONS) $(INC) -c mapdriver.c

$(DRIVER): types.h ascii.h ascii.c
	$(CC) $(CC_OPTIONS) $(INC) -c ascii.c

clean-all:
	make clean
	rmmod ascii
	lsmod

# EOF

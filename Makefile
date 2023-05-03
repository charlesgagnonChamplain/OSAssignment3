
CC=gcc
DEBUG=-g -D_DEBUG
DEFINE=-DMODULE -D__KERNEL__ -DLINUX
WARNINGS=-Wall -Wmissing-prototypes -Wmissing-declarations
#ISO=-ansi -pedantic
CC_OPTIONS=-O1 $(WARNINGS) $(ISO) $(DEBUG) $(DEFINE)

# Where to look for header files
INC=-I. -I/usr/include -I/usr/src/kernels/`uname -r`/include

FT=forkTest.c
DRIVER=ascii.o
MODULE=ascii.ko

obj-m += $(DRIVER)

# rm -f "/dev/asciimap"
# lsmod | grep "ascii"
# make build

all:
	sudo mknod -m 666 /dev/asciimap c 236 1	
	$(CC) $(CC_OPTIONS) $(FT) -o forkTest
	DIR=$(PWD)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cd $(DIR)

build:
	make all
	make register

clean:
	rm -f $(DRIVER)
	DIR=$(PWD)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	cd $(DIR)

ascii.o: ascii.c
	$(CC) $(WARNINGS) ascii.c -c -o ascii.o

test:
	$(CC) $(WARNINGS) forkTest.c -o forkTest
	./forkTest

main:
	$(CC) $(WARNINGS) main.c -o main
	./main

test-driver:
	$(CC) $(WARNINGS) testDriver.c -o testDriver
	./testDriver

register: $(DRIVER)
	sudo insmod ./$(MODULE)
	modinfo $(MODULE)

clean-all:
	make clean
	sudo rmmod ascii
	lsmod
	
# EOF

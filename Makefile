obj-m := setpriority.o
BUILDROOT_DIR := ../..
COMPILER := $(BUILDROOT_DIR)/output/host/bin/i686-buildroot-linux-uclibc-gcc

all:
	#$(COMPILER) -o setpriority setpriority.c -static -pthread -lpthread -Wall
	$(COMPILER) -o setpriority setpriority.c -lpthread -Wall
	cp setpriority $(BUILDROOT_DIR)/output/target/bin
	
clean:
	rm -f *.o *.ko .*.cmd
	rm -f setpriority

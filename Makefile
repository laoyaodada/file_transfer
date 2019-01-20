.PHONY: build test clean

MODULE_NAME :=netlink
obj-m :=$(MODULE_NAME).o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

build: libaplogger.so main client netlink netlink_app

libaplogger.so:
	g++ -fPIC -shared -o libaplogger.so aplogger.cpp

main:
	g++ ./main.cpp -lpthread -o main

client: libaplogger.so
	g++ client.cpp -L. -laplogger -o client
	export LD_LIBRARY_PATH=.

netlink:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

netlink_app:
	g++ ./netlink_app.cpp -o netlink_app

clean:
	rm -f *.o *.so a.out main client netlink_app
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

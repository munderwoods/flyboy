#
# KallistiGL Direct Rendering test program
# (c)2002 Dan Potter
#   

SHELL := /bin/bash

TARGET = test.elf
OBJS = test.o
KOS_ROMDISK_DIR = romdisk
CDI = test.cdi
CHEAT_ARG := $(shell source /opt/toolchains/dc/kos/environ.sh)

all: build cdi run-cdi

init:
	docker build -t kos .

source:
	source /opt/toolchains/dc/kos/environ.sh

rm-cdi:
	rm -f built-cdis/$(CDI)

rm-elf:
	rm -f elfs/*.elf

clean: rm-cdi rm-elf

run-cdi:
	~/redream built-cdis/$(CDI)

build: clean
	docker run -ti -v $(shell pwd)/src:/src kos $(MAKE)
	mv src/*.elf elfs/test.elf

cdi: rm-cdi source
	/opt/toolchains/dc/kos/utils/mkdcdisc -d src/ -e elfs/test.elf -o built-cdis/$(CDI) -s /opt/toolchains/dc/kos/utils/scramble/scramble -N -O /opt/toolchains/dc/sh-elf/bin/sh-elf-objcopy

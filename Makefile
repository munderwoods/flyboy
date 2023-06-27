SHELL := /bin/bash

TARGET = flyboy.elf
OBJS = flyboy.o
KOS_ROMDISK_DIR = romdisk
CDI = flyboy.cdi
CHEAT_ARG := $(shell source /opt/toolchains/dc/kos/environ.sh)

all: build cdi run-cdi
local: build-local cdi-local run-cdi

init: #run this first
	mkdir -p tools
	mkdir -p elfs
	mkdir -p built-cdis
	docker build -f Dockerfile.kos -t kos .
	docker build -f Dockerfile.extras -t dc-extras .
	docker run -ti -v $(shell pwd):/cdi kos cp /opt/toolchains/dc/kos/utils/scramble/scramble /cdi/tools/
	docker run -ti -v $(shell pwd):/cdi kos cp /opt/toolchains/dc/sh-elf/bin/sh-elf-objcopy /cdi/tools/
	wget -O ~/.local/bin/redream.tar.gz https://redream.io/download/redream.x86_64-linux-v1.5.0-1045-g9f00768.tar.gz
	cd ~/.local/bin && tar -xf redream.tar.gz

source:
	source /opt/toolchains/dc/kos/environ.sh

rm-cdi:
	rm -f built-cdis/$(CDI)

rm-elf:
	rm -f elfs/*.elf

clean: rm-cdi rm-elf

run-cdi:
	redream built-cdis/$(CDI)

docker-bash:
	docker run -ti -v $(shell pwd)/src:/src kos $(SHELL)

build: clean
	docker run -ti -v $(shell pwd)/src:/src kos $(MAKE)
	mv src/*.elf elfs/flyboy.elf

cdi: rm-cdi
	docker run -ti -v $(shell pwd):/cdi dc-extras mkdcdisc -d /cdi/src/ -e /cdi/elfs/flyboy.elf -o /cdi/built-cdis/$(CDI) -s /cdi/tools/scramble -N -O /cdi/tools/sh-elf-objcopy

build-local: clean
	cd src && $(MAKE)
	mv src/*.elf elfs/test.elf

cdi-local: rm-cdi source
	/opt/toolchains/dc/kos/utils/mkdcdisc -d src/ -e elfs/test.elf -o built-cdis/$(CDI) -s /opt/toolchains/dc/kos/utils/scramble/scramble -N -O /opt/toolchains/dc/sh-elf/bin/sh-elf-objcopy

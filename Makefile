# 
# Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
# Copyright (c) 2017 Texas Instruments - Jason Kridner <jdk@ti.com>
# 

# TARGET must be defined as the file to be compiled without the .c.
TARGET=adc-cape

# PRUN must be defined as the PRU number (0 or 1) to compile for.
PRUN=0

# PRU_CGT environment variable points to the TI PRU compiler directory.
# PRU_SUPPORT points to pru-software-support-package.
# PRU0_GEN_DIR points to where to put the generated files.
PRU_CGT:=/usr/share/ti/cgt-pru
PRU_SUPPORT:=/usr/lib/ti/pru-software-support-package
PRU0_GEN_DIR:=/tmp/pru$(PRUN)-gen

LINKER_COMMAND_FILE=AM335x_PRU.cmd
LIBS=--library=$(PRU_SUPPORT)/lib/rpmsg_lib.lib
INCLUDE=--include_path=$(PRU_SUPPORT)/include --include_path=$(PRU_SUPPORT)/include/am335x

STACK_SIZE=0x100
HEAP_SIZE=0x100

PRU_CFLAGS=-v3 -O2 --printf_support=minimal --display_error_number --endian=little --hardware_mac=on --obj_directory=$(PRU0_GEN_DIR) --pp_directory=$(PRU0_GEN_DIR) --asm_directory=$(PRU0_GEN_DIR) -ppd -ppa --asm_listing --c_src_interlist # --absolute_listing
PRU_LFLAGS=--reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE) -m $(PRU0_GEN_DIR)/$(TARGET).map

# Lookup PRU by address
ifeq ($(PRUN),0)
PRU_ADDR=4a334000
endif
ifeq ($(PRUN),1)
PRU_ADDR=4a338000
endif

PRU_DIR=$(wildcard /sys/devices/platform/ocp/4a32600*.pruss-soc-bus/4a300000.pruss/$(PRU_ADDR).*/remoteproc/remoteproc*)


CPU_GEN_DIR:=/tmp/adc-cape-gen
CPU_CFLAGS:=-c -Wall -O3
CPU_LFLAGS:=

all: stop install start adccape

stop:
	@echo "-    Stopping PRU $(PRUN)"
	@sudo sh -c "echo stop > $(PRU_DIR)/state" || echo Cannot stop $(PRUN)

start:
	@echo "-    Starting PRU $(PRUN)"
	@sudo sh -c "echo start > $(PRU_DIR)/state" || ((echo Cannot start $(PRUN); false))

install: $(PRU0_GEN_DIR)/$(TARGET).out configPins
	@echo '-    Copying firmware file $(PRU0_GEN_DIR)/$(TARGET).out to /lib/firmware/am335x-pru$(PRUN)-fw'
	@sudo cp $(PRU0_GEN_DIR)/$(TARGET).out /lib/firmware/am335x-pru$(PRUN)-fw
	
configPins:
	@echo "-    Configuring pins"
	@config-pin P9_29 pruin  # DRDY
	@config-pin P9_31 pruout # SCLK
	@config-pin P9_30 pruin  # MISO
	@config-pin P9_27 pruout # SYNC
	@config-pin P9_25 pruout # DEBUG

$(PRU0_GEN_DIR)/$(TARGET).out: $(PRU0_GEN_DIR)/$(TARGET).obj $(PRU0_GEN_DIR)/main.obj
	@echo 'LD	$^' 
	@lnkpru -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(PRU_LFLAGS) -o $@ $^ $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS) $^

$(PRU0_GEN_DIR)/main.obj: main.c
	@echo 'CC	$<'
	@clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(PRU_CFLAGS) -fe $@ $<

$(PRU0_GEN_DIR)/$(TARGET).obj: $(TARGET).asm
	@mkdir -p $(PRU0_GEN_DIR)
	@echo 'CC	$<'
	@clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(PRU_CFLAGS) -D=PRUN=$(PRUN) -fe $@ $<

adccape: $(CPU_GEN_DIR)/adccape.obj
	gcc $(CPU_LFLAGS) -o $@ $^

$(CPU_GEN_DIR)/%.obj: %.c
	@mkdir -p $(CPU_GEN_DIR)
	gcc $(CPU_CFLAGS) -o $@ $^

clean:
	@echo 'CLEAN	.    PRU $(PRUN)'
	@rm -rf $(PRU0_GEN_DIR)
	@echo 'CLEAN	.    CPU'
	@rm -rf $(CPU_GEN_DIR)
	@rm adccape


CFLAGS:=-c -Wall -O3 -Iinclude
LFLAGS:=

GEN_DIR:=bin

all: pru-run memory_test

include src-cpp/Makefrag
include src-pru/Makefrag
include examples/Makefrag


examples: $(GEN_DIR)/libadc-cape.so $(EXAMPLES)

pru-run: pru-stop pru-install pru-configPins pru-start

GEN_DIR:
	mkdir $@
	
clean:
	@echo "CLEAN	$(GEN_DIR)/ "
	@rm -rf $(GEN_DIR)
	
.PHONY: clean pru-run

CFLAGS:=-c -Wall -O3 -Iinclude
LFLAGS:=

GEN_DIR:=bin

all: pru-run memory_test

include src-cpp/Makefrag
include src-pru/Makefrag
include examples/Makefrag


examples: $(GEN_DIR)/libadc-cape.so moving_average

pru-run: pru-stop pru-install pru-configPins pru-start

GEN_DIR:
	mkdir $@
	
clean:
	@echo "CLEAN	$(GEN_DIR)/ $(EXAMPLES)"
	@rm -rf $(GEN_DIR) $(EXAMPLES) memory_test
	
.PHONY: clean pru-run
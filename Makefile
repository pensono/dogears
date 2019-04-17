
CFLAGS:=-c -Wall -O3 -Iinclude
LFLAGS:=

GEN_DIR:=bin

all: pru-run memory_test

include src-cpp/Makefrag
include src-pru/Makefrag
include examples/Makefrag


examples: libadc-cape.so $(EXAMPLES)

pru-run: $(GEN_DIR)/pru/adc-cape.out pru-stop pru-install pru-configPins pru-start

GEN_DIR:
	mkdir $@
	
clean:
	@echo "CLEAN	$(GEN_DIR)/ $(EXAMPLES)"
	@rm -rf $(GEN_DIR) $(EXAMPLES)
	
.PHONY: clean pru-run
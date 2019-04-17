
CPU_CFLAGS:=-c -Wall -O3
CPU_LFLAGS:=

GEN_DIR:=obj

all: pru-run adccape

include src-cpp/Makefrag
include src-pru/Makefrag
	
clean:
	@echo "CLEAN $(GEN_DIR)"
	@rm -rf $(GEN_DIR)

.PHONY: clean pru-run
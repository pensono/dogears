# --std=c++17 -lstdc++fs needed for <filesystem>
CFLAGS:=-c -Wall -O3 -Iinclude -DDEBUG --std=c++17 -lstdc++fs
LFLAGS:=

GEN_DIR:=bin

all: pru-run memory_test

include src-cpp/Makefrag
include src-pru/Makefrag
include examples/Makefrag
include test/Makefrag


GEN_DIR:
	mkdir $@
	
clean:
	@echo "CLEAN	$(GEN_DIR)/ $(EXAMPLES)"
	@rm -rf $(GEN_DIR) $(EXAMPLES)
	
.PHONY: clean pru-run
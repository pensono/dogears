# --std=c++17 -lstdc++fs needed for <filesystem>
CFLAGS:=-c -Wall -O3 -Iinclude -DDEBUG -g --std=c++17 -lstdc++fs -I/home/debian/am335x_pru_package/pru_sw/app_loader/include -lprussdrv
LFLAGS:=-lprussdrv

GEN_DIR:=bin

all: pru-run test_rpmsg

include src/cpp/Makefrag
include src/pru/Makefrag
include examples/Makefrag
include test/Makefrag


GEN_DIR:
	mkdir $@

clean:
	@echo "CLEAN	$(GEN_DIR)/ $(EXAMPLES)"
	@rm -rf $(GEN_DIR) $(EXAMPLES)

.PHONY: clean pru-run
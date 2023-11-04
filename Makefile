CC := gcc
LD := ld
OBJCOPY := objcopy
PYTHON := python3

CFLAGS := -Wall -Werror -Wl,--oformat=binary -no-pie -m32 -mno-mmx -mno-sse -mno-sse2 -mno-sse3 \
					-s -falign-functions=4 -ffreestanding -fno-asynchronous-unwind-tables

LFLAGS := -melf_i386 --build-id=none

MOS_ELF := ./mOS/mOS.elf
OUT := ./bin/game.bin

.PHONY: clean

all: $(MOS_ELF) $(OUT)

$(OUT): ./bin/game.o test_entry.o
	$(LD) -o $@.elf $(LFLAGS) -T link.ld $^ --just-symbols=$(MOS_ELF)
	$(OBJCOPY) -O binary $@.elf $@
	rm $@.elf

./bin/%.o: ./src/%.c
	$(CC) -c $^ -o $@ $(CFLAGS) -I./mOS/src/lib/ -I./mOS/src/lib/stdlib/

$(MOS_ELF):
	cd ./mOS/ && $(MAKE)

test_entry.o: ./mOS/tests/test_entry.asm
	nasm $^ -f elf32 -o $@

clean:
	rm -f test_entry.o
	rm -rf ./bin/*
	cd ./mOS/ && $(MAKE) clean

run: all
	python3 ./src/run.py


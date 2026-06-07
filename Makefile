CXX = clang++
CC  = clang

CFLAGS = \
    -target arm-none-eabi \
    -mcpu=cortex-a15 \
    -ffreestanding \
    -nostdlib

CXXFLAGS = \
    $(CFLAGS) \
    -fno-exceptions \
    -fno-rtti

all:
	$(CC) $(CFLAGS) \
	-c boot/start.S \
	-o start.o

	$(CXX) \
	$(CXXFLAGS) \
	-c kernel/kernel.cpp \
	-o kernel.o

	$(CXX) \
	$(CXXFLAGS) \
	-c kernel/uart.cpp \
	-o uart.o

	$(CC) \
	-T linker.ld \
	start.o kernel.o uart.o \
	-o spacejet.elf \
	-nostdlib

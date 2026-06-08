# SpaceJet OS — STM32F405 Makefile
#
# Flash to board:
#   make flash        (requires st-flash from stlink tools)
#   make flash_dfu    (DFU via USB)
# QEMU (limited STM32 support):
#   make qemu

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# Cortex-M4 with FPU — hard-float ABI
CFLAGS  := -mcpu=cortex-m4 -mthumb \
            -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
            -O2 -g -Wall -Wextra -Wno-unused-parameter \
            -ffreestanding -fno-builtin -nostdlib \
            -I. -I../include

BUILD   := build

BOOT    := startup_m4.s
SRCS    := clock.c uart_stm32.c systick.c spi_stm32.c i2c_stm32.c \
           scheduler_m4.c main_stm32.c \
           ../src/kstring.c ../src/kprintf.c ../src/crc.c ../src/ccsds.c \
           ../src/sjfs.c ../src/satkey.c ../src/logger.c ../src/watchdog.c \
           ../src/eps.c ../src/adcs.c ../src/telemetry.c ../src/command.c

OBJS    := $(BUILD)/startup_m4.o \
           $(patsubst %.c,$(BUILD)/%.o,$(filter %.c,$(SRCS)))

TARGET  := $(BUILD)/spacejet_stm32

.PHONY: all flash flash_dfu qemu clean

all: $(TARGET).elf $(TARGET).bin $(TARGET).hex
	@echo ""
	@echo "  STM32 build complete."
	@echo "  Flash: make flash  |  QEMU: make qemu"

$(BUILD)/startup_m4.o: startup_m4.s | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: ../src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -T linker_stm32f405.ld -nostdlib -o $@ $^ -lgcc
	@$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD):
	mkdir -p $@

# Requires: brew install stlink  OR  apt install stlink-tools
flash: $(TARGET).bin
	st-flash write $< 0x08000000

# DFU mode (hold BOOT0 while resetting)
flash_dfu: $(TARGET).bin
	dfu-util -a 0 -s 0x08000000:leave -D $<

# QEMU: netduino2 is STM32F405-based (limited peripheral emulation)
qemu: $(TARGET).elf
	@echo "  QEMU STM32 (limited) — Ctrl+A X to exit"
	qemu-system-arm -M netduino2 -nographic \
	    -serial stdio -kernel $<

clean:
	rm -rf $(BUILD)

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

CFLAGS  := -mcpu=arm926ej-s -marm -O1 -g \
            -Wall -Wextra -Wno-unused-parameter \
            -ffreestanding -fno-builtin \
            -nostdlib -Iinclude

BUILD   := build

BOOT_S  := boot/startup.s boot/task_switch.s
SRCS_C  := src/kstring.c src/crc.c src/ccsds.c \
            src/uart.c src/kprintf.c src/timer.c src/vic.c \
            src/scheduler.c \
            src/sjfs.c src/satkey.c src/logger.c src/watchdog.c \
            src/eps.c src/adcs.c src/telemetry.c \
            src/command.c src/main.c

BOOT_O  := $(patsubst boot/%.s,$(BUILD)/%.o,$(BOOT_S))
SRCS_O  := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS_C))
OBJS    := $(BOOT_O) $(SRCS_O)

TARGET  := $(BUILD)/spacejet

.PHONY: all qemu clean

all: $(TARGET).elf $(TARGET).bin
	@echo ""
	@echo "  SpaceJet OS v2.0 built — run: make qemu"

$(BUILD)/%.o: boot/%.s | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -T linker.ld -nostdlib -o $@ $^ -lgcc
	@$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD):
	mkdir -p $@

qemu: $(TARGET).elf
	@echo "  QEMU — Ctrl+A then X to exit"
	@echo "  --------------------------------"
	qemu-system-arm -M versatilepb -nographic -kernel $(TARGET).elf

clean:
	rm -rf $(BUILD)

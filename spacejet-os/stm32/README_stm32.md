# SpaceJet OS — STM32F405 Port

## Key differences from QEMU/versatilepb

| Feature | QEMU (ARM926) | STM32F405 (Cortex-M4) |
|---|---|---|
| ISA | ARMv5, ARM+Thumb | ARMv7-M, Thumb-2 only |
| Context switch | `stmfd`/`ldmfd` in IRQ mode | PendSV + PSP/MSP split |
| Preemption | Soft (flag) | **True** (every SysTick → PendSV) |
| Timer | SP804 @ 1 MHz | SysTick @ 168 MHz |
| UART | PL011 @ 0x101F1000 | USART1 PA9/PA10 |
| Clock | QEMU ignores | HSI → PLL → 168 MHz |
| Flash storage | SJFS in RAM | SJFS in SRAM (→ add SPI-NOR) |

## Why PendSV is the right way

Cortex-M designed PendSV specifically for RTOS context switches:
- Lowest priority — only fires when no other exception is running
- SysTick (higher priority) sets `PENDSVSET`, returns immediately
- PendSV fires, switches task — no priority inversion
- Hardware auto-saves r0-r3, r12, lr, pc, xpsr — we only save r4-r11

## Building

```bash
cd stm32
make              # build .elf + .bin + .hex
make flash        # flash via ST-Link (st-flash tool)
make flash_dfu    # flash via DFU (USB, BOOT0 held high)
make qemu         # test in QEMU (limited STM32 support)
```

## Wiring (STM32F4-Discovery or Nucleo)

| Signal | STM32 Pin | Peripheral |
|---|---|---|
| UART TX | PA9  | USART1 → USB-serial adapter |
| UART RX | PA10 | USART1 → USB-serial adapter |
| SPI SCK | PA5  | SPI1 → sensor / SPI flash |
| SPI MISO| PA6  | SPI1 |
| SPI MOSI| PA7  | SPI1 |
| SPI CS  | PA4  | SPI1 (GPIO) |
| I2C SCL | PB6  | I2C1 → RTC / IMU |
| I2C SDA | PB7  | I2C1 |

## Next steps for real satellite hardware

1. **SPI-NOR Flash** (e.g. W25Q128) — back SJFS with real non-volatile storage
2. **DS3231 RTC over I2C** — replace simulated date/time with real clock
3. **MPU-6050 IMU over I2C** — real attitude data into ADCS
4. **RF transceiver** (e.g. RFM96 LoRa over SPI) — real downlink for CCSDS packets
5. **Hardware watchdog** — `IWDG` peripheral (independent from CPU, survives lockups)
6. **Power management** — STM32 STOP/STANDBY modes for eclipse battery saving

# SpaceJet Satellite OS v1.0.0

Bare-metal ARM satellite OS — runs on real hardware or QEMU.

## Architecture

| Layer        | Component            | Details                          |
|--------------|----------------------|----------------------------------|
| Boot         | `boot/startup.s`     | ARM vector table, mode stacks, BSS clear |
| Context switch | `boot/task_switch.s` | stmfd/ldmfd {r4-r11, lr/pc}  |
| UART driver  | `src/uart.c`         | PL011 @ 0x101F1000, 115200 baud  |
| Timer driver | `src/timer.c`        | SP804 @ 0x101E2000, 100 Hz tick  |
| Interrupt ctrl | `src/vic.c`        | PL190 VIC @ 0x10140000           |
| Scheduler    | `src/scheduler.c`    | Cooperative round-robin          |
| EPS          | `src/eps.c`          | Power system simulation          |
| ADCS         | `src/adcs.c`         | Attitude control simulation      |
| Telemetry    | `src/telemetry.c`    | 5-second beacon                  |
| Shell        | `src/command.c`      | UART command interface           |

## Build

```bash
# In Termux:
pkg install arm-none-eabi-binutils
# Or get QEMU from somewhere else depending what platform your on:

make          # compile everything
make qemu     # run in QEMU (Ctrl+A then X to exit)
make clean    # remove build artefacts
```

## Shell Commands

| Command    | Description                         |
|------------|-------------------------------------|
| `help`     | List all commands                   |
| `status`   | Quick system overview               |
| `tlm`      | Full telemetry beacon               |
| `power`    | EPS battery / solar data            |
| `attitude` | ADCS roll / pitch / yaw             |
| `orbit`    | Position and orbit number           |
| `tasks`    | Scheduler task list and states      |
| `ticks`    | Raw timer tick count                |
| `version`  | Firmware version string             |
| `reboot`   | Soft-reset (jumps to 0x00000000)    |

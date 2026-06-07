#!/data/data/com.termux/files/usr/bin/bash

qemu-system-arm \
    -M virt \
    -cpu cortex-a15 \
    -kernel spacejet.elf \
    -nographic

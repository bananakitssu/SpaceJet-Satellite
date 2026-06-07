#include "uart.hpp"

constexpr unsigned long UART0 =
    0x09000000;

void UART::Put(char c)
{
    *(volatile unsigned int*)UART0 = c;
}

void UART::Write(const char* str)
{
    while (*str)
    {
        Put(*str++);
    }
}

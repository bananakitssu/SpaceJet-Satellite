#include "uart.hpp"

extern "C"
void kernel_main()
{
    UART::Write(
        "\n"
        "====================\n"
        " SpaceJet Satellite\n"
        " SJOS Booting...\n"
        "====================\n"
    );

    while (1)
    {
    }
}

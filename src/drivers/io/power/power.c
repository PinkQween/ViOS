#include "power.h"

// Global variables to be set by ACPI parser
uint16_t pm1a_cnt_port = 0x1004; // example fallback default
uint16_t pm1b_cnt_port = 0x0000;

void power_shutdown(uint16_t slp_typ)
{
    const uint16_t SLP_EN = 1 << 13;

    disable_interrupts();

    // Send shutdown command to PM1a
    if (pm1a_cnt_port)
        outw(pm1a_cnt_port, slp_typ | SLP_EN);

    // Send shutdown command to PM1b (optional)
    if (pm1b_cnt_port)
        outw(pm1b_cnt_port, slp_typ | SLP_EN);
}

void power_restart(void)
{
    disable_interrupts();
    outb(0xCF9, 0x06); // Reset via Reset Control Register
}

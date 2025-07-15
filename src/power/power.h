#ifndef POWER_H
#define POWER_H

#include <stdint.h>
#include "io/io.h"
#include "idt/idt.h"

// ACPI sleep state types (SLP_TYP << 10)
#define ACPI_SLEEP_TYPE_S0 (0 << 10) // Working (normal running state)
#define ACPI_SLEEP_TYPE_S1 (1 << 10) // Light sleep (CPU stopped, RAM retained)
#define ACPI_SLEEP_TYPE_S2 (2 << 10) // Deeper sleep
#define ACPI_SLEEP_TYPE_S3 (3 << 10) // Suspend to RAM (Sleep)
#define ACPI_SLEEP_TYPE_S4 (4 << 10) // Suspend to Disk (Hibernate)
#define ACPI_SLEEP_TYPE_S5 (5 << 10) // Soft Off (Shutdown)

// These can be updated by your ACPI parser
extern uint16_t pm1a_cnt_port;
extern uint16_t pm1b_cnt_port;

// Power control interface
void power_shutdown(uint16_t slp_typ);
void power_restart(void);

#endif // POWER_H

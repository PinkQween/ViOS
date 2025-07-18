#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// PCI Configuration Address Register (0xCF8)
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// PCI Configuration Space Header
#define PCI_CONFIG_VENDOR_ID 0x00
#define PCI_CONFIG_DEVICE_ID 0x02
#define PCI_CONFIG_COMMAND 0x04
#define PCI_CONFIG_STATUS 0x06
#define PCI_CONFIG_REVISION 0x08
#define PCI_CONFIG_PROG_IF 0x09
#define PCI_CONFIG_SUBCLASS 0x0A
#define PCI_CONFIG_CLASS 0x0B
#define PCI_CONFIG_CACHE_LINE 0x0C
#define PCI_CONFIG_LATENCY 0x0D
#define PCI_CONFIG_HEADER_TYPE 0x0E
#define PCI_CONFIG_BIST 0x0F
#define PCI_CONFIG_BAR0 0x10
#define PCI_CONFIG_BAR1 0x14
#define PCI_CONFIG_BAR2 0x18
#define PCI_CONFIG_BAR3 0x1C
#define PCI_CONFIG_BAR4 0x20
#define PCI_CONFIG_BAR5 0x24
#define PCI_CONFIG_CARDBUS_CIS 0x28
#define PCI_CONFIG_SUBSYS_VENDOR 0x2C
#define PCI_CONFIG_SUBSYS_ID 0x2E
#define PCI_CONFIG_ROM_ADDRESS 0x30
#define PCI_CONFIG_CAPABILITIES 0x34
#define PCI_CONFIG_INTERRUPT_LINE 0x3C
#define PCI_CONFIG_INTERRUPT_PIN 0x3D
#define PCI_CONFIG_MIN_GRANT 0x3E
#define PCI_CONFIG_MAX_LATENCY 0x3F

// PCI Command Register Bits
#define PCI_COMMAND_IO_SPACE 0x01
#define PCI_COMMAND_MEMORY_SPACE 0x02
#define PCI_COMMAND_BUS_MASTER 0x04
#define PCI_COMMAND_SPECIAL_CYCLES 0x08
#define PCI_COMMAND_MWI_ENABLE 0x10
#define PCI_COMMAND_VGA_PALETTE_SNOOP 0x20
#define PCI_COMMAND_PARITY_ERROR 0x40
#define PCI_COMMAND_SERR_ENABLE 0x100
#define PCI_COMMAND_FAST_BACK 0x200
#define PCI_COMMAND_INTERRUPT_DISABLE 0x400

// PCI Status Register Bits
#define PCI_STATUS_INTERRUPT_STATUS 0x08
#define PCI_STATUS_CAPABILITIES_LIST 0x10
#define PCI_STATUS_66MHZ_CAPABLE 0x20
#define PCI_STATUS_FAST_BACK 0x80
#define PCI_STATUS_PARITY_ERROR 0x100
#define PCI_STATUS_DEVSEL_TIMING 0x600
#define PCI_STATUS_SIGNALED_TARGET_ABORT 0x800
#define PCI_STATUS_RECEIVED_TARGET_ABORT 0x1000
#define PCI_STATUS_RECEIVED_MASTER_ABORT 0x2000
#define PCI_STATUS_SIGNALED_SYSTEM_ERROR 0x4000
#define PCI_STATUS_DETECTED_PARITY_ERROR 0x8000

// PCI Header Types
#define PCI_HEADER_TYPE_DEVICE 0x00
#define PCI_HEADER_TYPE_BRIDGE 0x01
#define PCI_HEADER_TYPE_CARDBUS 0x02

// PCI Class Codes
#define PCI_CLASS_UNCLASSIFIED 0x00
#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_CLASS_NETWORK 0x02
#define PCI_CLASS_DISPLAY 0x03
#define PCI_CLASS_MULTIMEDIA 0x04
#define PCI_CLASS_MEMORY 0x05
#define PCI_CLASS_BRIDGE 0x06
#define PCI_CLASS_COMMUNICATION 0x07
#define PCI_CLASS_SYSTEM 0x08
#define PCI_CLASS_INPUT 0x09
#define PCI_CLASS_DOCKING 0x0A
#define PCI_CLASS_PROCESSOR 0x0B
#define PCI_CLASS_SERIAL_BUS 0x0C
#define PCI_CLASS_WIRELESS 0x0D
#define PCI_CLASS_INTELLIGENT_IO 0x0E
#define PCI_CLASS_SATELLITE 0x0F
#define PCI_CLASS_ENCRYPTION 0x10
#define PCI_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_PROCESSING_ACCELERATOR 0x12
#define PCI_CLASS_NON_ESSENTIAL 0x13
#define PCI_CLASS_COPROCESSOR 0x40
#define PCI_CLASS_UNASSIGNED 0xFF

// BAR Types
#define PCI_BAR_TYPE_MEMORY 0x00
#define PCI_BAR_TYPE_IO 0x01
#define PCI_BAR_MEMORY_32BIT 0x00
#define PCI_BAR_MEMORY_64BIT 0x04
#define PCI_BAR_MEMORY_PREFETCHABLE 0x08

// Maximum number of PCI devices to track
#define MAX_PCI_DEVICES 256

// PCI Device Structure
typedef struct pci_device
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
    uint8_t header_type;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint32_t bar[6];
    uint32_t bar_size[6];
    uint8_t bar_type[6];
    bool enabled;
    void *driver_data;
} pci_device_t;

// PCI Driver Structure
typedef struct pci_driver
{
    const char *name;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    int (*probe)(pci_device_t *device);
    int (*remove)(pci_device_t *device);
    int (*suspend)(pci_device_t *device);
    int (*resume)(pci_device_t *device);
    struct pci_driver *next;
} pci_driver_t;

// PCI Error Codes
typedef enum
{
    PCI_SUCCESS = 0,
    PCI_ERROR_INVALID_DEVICE = -1,
    PCI_ERROR_INVALID_FUNCTION = -2,
    PCI_ERROR_INVALID_REGISTER = -3,
    PCI_ERROR_DEVICE_NOT_FOUND = -4,
    PCI_ERROR_DRIVER_NOT_FOUND = -5,
    PCI_ERROR_ALREADY_REGISTERED = -6,
    PCI_ERROR_MEMORY_ALLOCATION = -7,
    PCI_ERROR_BAR_MAPPING = -8,
    PCI_ERROR_INTERRUPT_SETUP = -9
} pci_error_t;

// Function Declarations
int pci_init(void);
void pci_shutdown(void);

// Configuration Space Access
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

void pci_config_write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);
void pci_config_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

// Device Discovery and Management
int pci_scan_bus(void);
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t *pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
pci_device_t *pci_get_device(uint8_t bus, uint8_t device, uint8_t function);
int pci_get_device_count(void);
pci_device_t *pci_get_device_by_index(int index);

// Device Operations
int pci_enable_device(pci_device_t *device);
int pci_disable_device(pci_device_t *device);
int pci_enable_bus_master(pci_device_t *device);
int pci_disable_bus_master(pci_device_t *device);

// BAR Operations
int pci_read_bar(pci_device_t *device, int bar_index);
void *pci_map_bar(pci_device_t *device, int bar_index);
void pci_unmap_bar(pci_device_t *device, int bar_index, void *mapped_address);
uint32_t pci_get_bar_size(pci_device_t *device, int bar_index);
bool pci_bar_is_io(pci_device_t *device, int bar_index);
bool pci_bar_is_memory(pci_device_t *device, int bar_index);
bool pci_bar_is_prefetchable(pci_device_t *device, int bar_index);

// Interrupt Operations
int pci_get_interrupt_line(pci_device_t *device);
int pci_set_interrupt_line(pci_device_t *device, uint8_t irq);
int pci_enable_interrupts(pci_device_t *device);
int pci_disable_interrupts(pci_device_t *device);

// Driver Management
int pci_register_driver(pci_driver_t *driver);
int pci_unregister_driver(pci_driver_t *driver);
int pci_match_device(pci_device_t *device);

// Capability Support
uint8_t pci_find_capability(pci_device_t *device, uint8_t cap_id);
int pci_read_capability(pci_device_t *device, uint8_t cap_offset, uint8_t *buffer, size_t size);

// Power Management
int pci_set_power_state(pci_device_t *device, uint8_t state);
uint8_t pci_get_power_state(pci_device_t *device);

// Debug and Utility Functions
void pci_dump_device_info(pci_device_t *device);
void pci_dump_all_devices(void);
const char *pci_get_class_name(uint8_t class_code);
const char *pci_get_vendor_name(uint16_t vendor_id);
const char *pci_get_device_name(uint16_t vendor_id, uint16_t device_id);

#endif // PCI_H

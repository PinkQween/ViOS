#include "pci.h"
#include "io/io.h"
#include <stddef.h>
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "debug/simple_serial.h"

static pci_device_t pci_devices[MAX_PCI_DEVICES];
static int pci_device_count = 0;

uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t address = ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
                       ((uint32_t)function << 8) | (offset & 0xfc) |
                       ((uint32_t)0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t data = pci_config_read32(bus, device, function, offset & ~0x2);
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xffff);
}

uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t data = pci_config_read32(bus, device, function, offset & ~0x3);
    return (uint8_t)((data >> ((offset & 3) * 8)) & 0xff);
}

static void pci_check_device(uint8_t bus, uint8_t device)
{
    uint16_t vendor_id = pci_config_read16(bus, device, 0, PCI_CONFIG_VENDOR_ID);
    if (vendor_id == 0xFFFF)
        return; // Device doesn't exist

    for (uint8_t function = 0; function < 8; ++function)
    {
        uint16_t vendor_id = pci_config_read16(bus, device, function, PCI_CONFIG_VENDOR_ID);
        if (vendor_id == 0xFFFF || vendor_id == 0x0000)
            continue;
        
        // Additional validation - check device ID too
        uint16_t device_id = pci_config_read16(bus, device, function, PCI_CONFIG_DEVICE_ID);
        if (device_id == 0xFFFF || device_id == 0x0000)
            continue;

        if (pci_device_count >= MAX_PCI_DEVICES) {
            simple_serial_puts("DEBUG: PCI device array full, skipping remaining devices\n");
            return;
        }

        pci_device_t *dev = &pci_devices[pci_device_count++];
        dev->bus = bus;
        dev->device = device;
        dev->function = function;
        dev->vendor_id = vendor_id;
        dev->device_id = device_id;
        dev->class_code = pci_config_read8(bus, device, function, PCI_CONFIG_CLASS);
        dev->subclass = pci_config_read8(bus, device, function, PCI_CONFIG_SUBCLASS);
        dev->prog_if = pci_config_read8(bus, device, function, PCI_CONFIG_PROG_IF);
        dev->revision = pci_config_read8(bus, device, function, PCI_CONFIG_REVISION);
        dev->header_type = pci_config_read8(bus, device, function, PCI_CONFIG_HEADER_TYPE);
        dev->interrupt_line = pci_config_read8(bus, device, function, PCI_CONFIG_INTERRUPT_LINE);
        dev->interrupt_pin = pci_config_read8(bus, device, function, PCI_CONFIG_INTERRUPT_PIN);

        // Read Base Address Registers
        for (int bar = 0; bar < 6; ++bar)
        {
            uint32_t bar_value = pci_config_read32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4);
            dev->bar[bar] = bar_value;

            if (bar_value == 0) {
                // Empty BAR
                dev->bar_type[bar] = PCI_BAR_TYPE_MEMORY;
                dev->bar_size[bar] = 0;
            }
            else if (bar_value & 0x1)
            {
                dev->bar_type[bar] = PCI_BAR_TYPE_IO;
                // For I/O BARs, also calculate size
                pci_config_write32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4, 0xFFFFFFFF);
                uint32_t size_mask = pci_config_read32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4);
                pci_config_write32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4, bar_value);
                dev->bar_size[bar] = ~(size_mask & ~0x3) + 1;
            }
            else
            {
                dev->bar_type[bar] = PCI_BAR_TYPE_MEMORY;
                // To determine BAR size, write all 1s and read back
                pci_config_write32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4, 0xFFFFFFFF);
                uint32_t size_mask = pci_config_read32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4);
                // Restore original BAR value
                pci_config_write32(bus, device, function, PCI_CONFIG_BAR0 + bar * 4, bar_value);
                dev->bar_size[bar] = ~(size_mask & ~0xF) + 1;
            }
        }
        
        // If this is function 0 and not a multifunction device, don't check other functions
        if (function == 0 && !(dev->header_type & 0x80)) {
            break;
        }
    }
}

int pci_scan_bus(void)
{
    pci_device_count = 0;
    
    simple_serial_puts("DEBUG: Starting PCI bus scan\n");
    
    // Start with bus 0 and scan recursively based on bridges found
    for (uint8_t bus = 0; bus < 8; ++bus)  // Most systems have buses 0-7
    {
        bool bus_has_devices = false;
        for (uint8_t device = 0; device < 32; ++device)
        {
            // Quick check if device exists before full scan
            uint16_t vendor_id = pci_config_read16(bus, device, 0, PCI_CONFIG_VENDOR_ID);
            if (vendor_id != 0xFFFF && vendor_id != 0x0000) {
                bus_has_devices = true;
                pci_check_device(bus, device);
                
                // Safety check to prevent infinite loops
                if (pci_device_count >= MAX_PCI_DEVICES) {
                    simple_serial_puts("DEBUG: PCI device limit reached\n");
                    return pci_device_count;
                }
            }
        }
        
        // If no devices found on this bus, likely no more buses to scan
        if (!bus_has_devices && bus > 0) {
            break;
        }
    }
    
    simple_serial_puts("DEBUG: PCI scan complete. Found devices:\n");
    for (int i = 0; i < pci_device_count; ++i) {
        simple_serial_puts("  Device ");
        simple_serial_put_hex(i);
        simple_serial_puts(": Bus ");
        simple_serial_put_hex(pci_devices[i].bus);
        simple_serial_puts(", Device ");
        simple_serial_put_hex(pci_devices[i].device);
        simple_serial_puts(", Function ");
        simple_serial_put_hex(pci_devices[i].function);
        simple_serial_puts(", Vendor: 0x");
        simple_serial_put_hex(pci_devices[i].vendor_id);
        simple_serial_puts(", Device ID: 0x");
        simple_serial_put_hex(pci_devices[i].device_id);
        simple_serial_puts(", Class: 0x");
        simple_serial_put_hex(pci_devices[i].class_code);
        simple_serial_puts(", Subclass: 0x");
        simple_serial_put_hex(pci_devices[i].subclass);
        simple_serial_puts(", BAR0: 0x");
        simple_serial_put_hex(pci_devices[i].bar[0]);
        simple_serial_puts(", BAR0 type: ");
        if (pci_devices[i].bar_type[0] == PCI_BAR_TYPE_IO) {
            simple_serial_puts("I/O");
        } else {
            simple_serial_puts("Memory");
        }
        simple_serial_puts("\n");
    }
    
    return pci_device_count;
}

pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id)
{
    for (int i = 0; i < pci_device_count; ++i)
    {
        if (pci_devices[i].vendor_id == vendor_id && pci_devices[i].device_id == device_id)
        {
            return &pci_devices[i];
        }
    }
    return NULL;
}

void pci_config_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value)
{
    uint32_t address = ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
                       ((uint32_t)function << 8) | (offset & 0xfc) |
                       ((uint32_t)0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

void pci_config_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value)
{
    uint32_t address = ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
                       ((uint32_t)function << 8) | (offset & 0xfc) |
                       ((uint32_t)0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);

    uint32_t data = inl(PCI_CONFIG_DATA);
    uint32_t shift = (offset & 2) * 8;
    data = (data & ~(0xFFFF << shift)) | ((uint32_t)value << shift);
    outl(PCI_CONFIG_DATA, data);
}

void pci_config_write8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value)
{
    uint32_t address = ((uint32_t)bus << 16) | ((uint32_t)device << 11) |
                       ((uint32_t)function << 8) | (offset & 0xfc) |
                       ((uint32_t)0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);

    uint32_t data = inl(PCI_CONFIG_DATA);
    uint32_t shift = (offset & 3) * 8;
    data = (data & ~(0xFF << shift)) | ((uint32_t)value << shift);
    outl(PCI_CONFIG_DATA, data);
}

pci_device_t *pci_find_device_by_class(uint8_t class_code, uint8_t subclass)
{
    for (int i = 0; i < pci_device_count; ++i)
    {
        if (pci_devices[i].class_code == class_code && pci_devices[i].subclass == subclass)
        {
            return &pci_devices[i];
        }
    }
    return NULL;
}

pci_device_t *pci_get_device(uint8_t bus, uint8_t device, uint8_t function)
{
    for (int i = 0; i < pci_device_count; ++i)
    {
        if (pci_devices[i].bus == bus && pci_devices[i].device == device && pci_devices[i].function == function)
        {
            return &pci_devices[i];
        }
    }
    return NULL;
}

int pci_get_device_count(void)
{
    return pci_device_count;
}

pci_device_t *pci_get_device_by_index(int index)
{
    if (index < 0 || index >= pci_device_count)
    {
        return NULL;
    }
    return &pci_devices[index];
}

int pci_enable_device(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command |= PCI_COMMAND_IO_SPACE | PCI_COMMAND_MEMORY_SPACE;
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    device->enabled = true;
    return PCI_SUCCESS;
}

int pci_disable_device(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command &= ~(PCI_COMMAND_IO_SPACE | PCI_COMMAND_MEMORY_SPACE);
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    device->enabled = false;
    return PCI_SUCCESS;
}

int pci_enable_bus_master(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command |= PCI_COMMAND_BUS_MASTER;
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    return PCI_SUCCESS;
}

int pci_disable_bus_master(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command &= ~PCI_COMMAND_BUS_MASTER;
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    return PCI_SUCCESS;
}

int pci_read_bar(pci_device_t *device, int bar_index)
{
    if (!device || bar_index < 0 || bar_index >= 6)
        return PCI_ERROR_INVALID_DEVICE;

    uint32_t bar_value = pci_config_read32(device->bus, device->device, device->function, PCI_CONFIG_BAR0 + bar_index * 4);
    device->bar[bar_index] = bar_value;

    // Determine BAR size by writing all 1s and reading back
    pci_config_write32(device->bus, device->device, device->function, PCI_CONFIG_BAR0 + bar_index * 4, 0xFFFFFFFF);
    uint32_t size_mask = pci_config_read32(device->bus, device->device, device->function, PCI_CONFIG_BAR0 + bar_index * 4);
    pci_config_write32(device->bus, device->device, device->function, PCI_CONFIG_BAR0 + bar_index * 4, bar_value);

    if (bar_value & 0x1)
    {
        // I/O BAR
        device->bar_type[bar_index] = PCI_BAR_TYPE_IO;
        device->bar_size[bar_index] = ~(size_mask & 0xFFFFFFFC) + 1;
    }
    else
    {
        // Memory BAR
        device->bar_type[bar_index] = PCI_BAR_TYPE_MEMORY;
        device->bar_size[bar_index] = ~(size_mask & 0xFFFFFFF0) + 1;
    }

    return PCI_SUCCESS;
}

bool pci_bar_is_io(pci_device_t *device, int bar_index)
{
    if (!device || bar_index < 0 || bar_index >= 6)
        return false;
    return device->bar_type[bar_index] == PCI_BAR_TYPE_IO;
}

bool pci_bar_is_memory(pci_device_t *device, int bar_index)
{
    if (!device || bar_index < 0 || bar_index >= 6)
        return false;
    return device->bar_type[bar_index] == PCI_BAR_TYPE_MEMORY;
}

bool pci_bar_is_prefetchable(pci_device_t *device, int bar_index)
{
    if (!device || bar_index < 0 || bar_index >= 6)
        return false;
    return (device->bar[bar_index] & PCI_BAR_MEMORY_PREFETCHABLE) != 0;
}

uint32_t pci_get_bar_size(pci_device_t *device, int bar_index)
{
    if (!device || bar_index < 0 || bar_index >= 6)
        return 0;
    return device->bar_size[bar_index];
}

int pci_get_interrupt_line(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;
    return device->interrupt_line;
}

int pci_set_interrupt_line(pci_device_t *device, uint8_t irq)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    pci_config_write8(device->bus, device->device, device->function, PCI_CONFIG_INTERRUPT_LINE, irq);
    device->interrupt_line = irq;
    return PCI_SUCCESS;
}

int pci_enable_interrupts(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command &= ~PCI_COMMAND_INTERRUPT_DISABLE;
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    return PCI_SUCCESS;
}

int pci_disable_interrupts(pci_device_t *device)
{
    if (!device)
        return PCI_ERROR_INVALID_DEVICE;

    uint16_t command = pci_config_read16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND);
    command |= PCI_COMMAND_INTERRUPT_DISABLE;
    pci_config_write16(device->bus, device->device, device->function, PCI_CONFIG_COMMAND, command);
    return PCI_SUCCESS;
}

void pci_dump_device_info(pci_device_t *device)
{
    if (!device)
        return;

    simple_serial_puts("PCI Device: ");
    simple_serial_puts("Bus: ");
    // Add hex printing utilities here
    simple_serial_puts("\n");
}

void pci_dump_all_devices(void)
{
    simple_serial_puts("PCI Devices Found:\n");
    for (int i = 0; i < pci_device_count; ++i)
    {
        pci_dump_device_info(&pci_devices[i]);
    }
}

const char *pci_get_class_name(uint8_t class_code)
{
    switch (class_code)
    {
    case PCI_CLASS_UNCLASSIFIED:
        return "Unclassified";
    case PCI_CLASS_MASS_STORAGE:
        return "Mass Storage";
    case PCI_CLASS_NETWORK:
        return "Network";
    case PCI_CLASS_DISPLAY:
        return "Display";
    case PCI_CLASS_MULTIMEDIA:
        return "Multimedia";
    case PCI_CLASS_MEMORY:
        return "Memory";
    case PCI_CLASS_BRIDGE:
        return "Bridge";
    case PCI_CLASS_COMMUNICATION:
        return "Communication";
    case PCI_CLASS_SYSTEM:
        return "System";
    case PCI_CLASS_INPUT:
        return "Input";
    default:
        return "Unknown";
    }
}

int pci_init(void)
{
    simple_serial_puts("DEBUG: Initializing PCI subsystem...\n");
    int device_count = pci_scan_bus();
    simple_serial_puts("DEBUG: PCI scan complete, found ");
    // Add device count printing here
    simple_serial_puts(" devices\n");
    return device_count;
}

void pci_shutdown(void)
{
    simple_serial_puts("DEBUG: Shutting down PCI subsystem...\n");
}

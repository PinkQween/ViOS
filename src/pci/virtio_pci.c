#include "virtio_pci.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "debug/simple_serial.h"
#include "memory/memory.h"
#include "string/string.h"
#include <stddef.h>

static virtio_pci_device_t virtio_devices[16];
static int virtio_device_count = 0;

int virtio_pci_init(void) {
    simple_serial_puts("DEBUG: Initializing VirtIO PCI transport layer...\\n");
    
    // Find all VirtIO devices
    int device_count = pci_get_device_count();
    for (int i = 0; i < device_count; i++) {
        pci_device_t *pci_dev = pci_get_device_by_index(i);
        if (pci_dev && pci_dev->vendor_id == VIRTIO_PCI_VENDOR_ID) {
            if (virtio_device_count < 16) {
                virtio_pci_device_t *virtio_dev = &virtio_devices[virtio_device_count];
                if (virtio_pci_probe_device(pci_dev, virtio_dev) == 0) {
                    virtio_device_count++;
                    simple_serial_puts("DEBUG: Found VirtIO device\\n");
                }
            }
        }
    }
    
    simple_serial_puts("DEBUG: VirtIO PCI transport layer initialized\\n");
    return virtio_device_count;
}

void virtio_pci_shutdown(void) {
    for (int i = 0; i < virtio_device_count; i++) {
        virtio_pci_reset_device(&virtio_devices[i]);
    }
    virtio_device_count = 0;
}

virtio_pci_device_t *virtio_pci_find_device(uint16_t device_id) {
    for (int i = 0; i < virtio_device_count; i++) {
        if (virtio_devices[i].pci_dev->device_id == device_id) {
            return &virtio_devices[i];
        }
    }
    return NULL;
}

int virtio_pci_probe_device(pci_device_t *pci_dev, virtio_pci_device_t *virtio_dev) {
    if (!pci_dev || !virtio_dev) {
        return -1;
    }
    
    memset(virtio_dev, 0, sizeof(virtio_pci_device_t));
    virtio_dev->pci_dev = pci_dev;
    
    // Enable the device
    pci_enable_device(pci_dev);
    
    // Get base address from BAR0 (support both I/O and memory space)
    if (pci_bar_is_io(pci_dev, 0)) {
        virtio_dev->io_base = pci_dev->bar[0] & ~0x3;
        virtio_dev->use_mmio = false;
        simple_serial_puts("DEBUG: VirtIO device using I/O space\n");
    } else if (pci_bar_is_memory(pci_dev, 0)) {
        virtio_dev->io_base = pci_dev->bar[0] & ~0xF;
        virtio_dev->use_mmio = true;
        simple_serial_puts("DEBUG: VirtIO device using memory-mapped I/O\n");
    } else {
        simple_serial_puts("DEBUG: VirtIO device BAR0 is neither I/O nor memory\n");
        return -1;
    }
    
    // Reset the device
    virtio_pci_reset_device(virtio_dev);
    
    return 0;
}

int virtio_pci_init_device(virtio_pci_device_t *dev) {
    if (!dev) return -1;
    
    // Set ACKNOWLEDGE status
    virtio_pci_add_status(dev, VIRTIO_STATUS_ACKNOWLEDGE);
    
    // Set DRIVER status
    virtio_pci_add_status(dev, VIRTIO_STATUS_DRIVER);
    
    // Read features
    dev->features = virtio_pci_get_features(dev);
    
    dev->initialized = true;
    return 0;
}

void virtio_pci_reset_device(virtio_pci_device_t *dev) {
    if (!dev) return;
    
    // Reset device
    outb(dev->io_base + VIRTIO_PCI_STATUS, 0);
    dev->status = 0;
    dev->initialized = false;
}

uint32_t virtio_pci_get_features(virtio_pci_device_t *dev) {
    if (!dev) return 0;
    return inl(dev->io_base + VIRTIO_PCI_HOST_FEATURES);
}

int virtio_pci_set_features(virtio_pci_device_t *dev, uint32_t features) {
    if (!dev) return -1;
    
    dev->guest_features = features;
    outl(dev->io_base + VIRTIO_PCI_GUEST_FEATURES, features);
    return 0;
}

bool virtio_pci_has_feature(virtio_pci_device_t *dev, uint32_t feature) {
    if (!dev) return false;
    return (dev->features & (1 << feature)) != 0;
}

uint8_t virtio_pci_get_status(virtio_pci_device_t *dev) {
    if (!dev) return 0;
    return inb(dev->io_base + VIRTIO_PCI_STATUS);
}

int virtio_pci_set_status(virtio_pci_device_t *dev, uint8_t status) {
    if (!dev) return -1;
    
    outb(dev->io_base + VIRTIO_PCI_STATUS, status);
    dev->status = status;
    return 0;
}

int virtio_pci_add_status(virtio_pci_device_t *dev, uint8_t status) {
    if (!dev) return -1;
    
    uint8_t current_status = virtio_pci_get_status(dev);
    return virtio_pci_set_status(dev, current_status | status);
}

int virtio_pci_setup_queue(virtio_pci_device_t *dev, uint16_t queue_idx, uint16_t size) {
    if (!dev || queue_idx >= 16) return -1;
    
    // Select queue
    outw(dev->io_base + VIRTIO_PCI_QUEUE_SEL, queue_idx);
    
    // Check if queue exists
    uint16_t queue_size = inw(dev->io_base + VIRTIO_PCI_QUEUE_NUM);
    if (queue_size == 0) {
        return -1; // Queue doesn't exist
    }
    
    // Use the requested size or the maximum available
    if (size == 0 || size > queue_size) {
        size = queue_size;
    }
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    queue->queue_size = size;
    queue->queue_select = queue_idx;
    queue->last_avail_idx = 0;
    queue->last_used_idx = 0;
    
    // Allocate memory for queue structures
    size_t desc_size = sizeof(virtio_ring_desc_t) * size;
    size_t avail_size = sizeof(virtio_ring_avail_t) + sizeof(uint16_t) * size;
    size_t used_size = sizeof(virtio_ring_used_t) + sizeof(virtio_ring_used_elem_t) * size;
    
    queue->desc = (virtio_ring_desc_t *)kzalloc(desc_size);
    queue->avail = (virtio_ring_avail_t *)kzalloc(avail_size);
    queue->used = (virtio_ring_used_t *)kzalloc(used_size);
    
    if (!queue->desc || !queue->avail || !queue->used) {
        if (queue->desc) kfree(queue->desc);
        if (queue->avail) kfree(queue->avail);
        if (queue->used) kfree(queue->used);
        return -1;
    }
    
    // Set physical addresses (assuming identity mapping for now)
    queue->desc_phys = queue->desc;
    queue->avail_phys = queue->avail;
    queue->used_phys = queue->used;
    
    return 0;
}

int virtio_pci_activate_queue(virtio_pci_device_t *dev, uint16_t queue_idx) {
    if (!dev || queue_idx >= 16) return -1;
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    if (!queue->desc) return -1;
    
    // Select queue
    outw(dev->io_base + VIRTIO_PCI_QUEUE_SEL, queue_idx);
    
    // Set queue PFN (Page Frame Number)
    uint32_t pfn = (uint32_t)queue->desc_phys / 4096;
    outl(dev->io_base + VIRTIO_PCI_QUEUE_PFN, pfn);
    
    return 0;
}

void virtio_pci_notify_queue(virtio_pci_device_t *dev, uint16_t queue_idx) {
    if (!dev) return;
    outw(dev->io_base + VIRTIO_PCI_QUEUE_NOTIFY, queue_idx);
}

uint16_t virtio_pci_get_queue_size(virtio_pci_device_t *dev, uint16_t queue_idx) {
    if (!dev || queue_idx >= 16) return 0;
    
    outw(dev->io_base + VIRTIO_PCI_QUEUE_SEL, queue_idx);
    return inw(dev->io_base + VIRTIO_PCI_QUEUE_NUM);
}

uint8_t virtio_pci_config_read8(virtio_pci_device_t *dev, uint16_t offset) {
    if (!dev) return 0;
    return inb(dev->io_base + VIRTIO_PCI_CONFIG + offset);
}

uint16_t virtio_pci_config_read16(virtio_pci_device_t *dev, uint16_t offset) {
    if (!dev) return 0;
    return inw(dev->io_base + VIRTIO_PCI_CONFIG + offset);
}

uint32_t virtio_pci_config_read32(virtio_pci_device_t *dev, uint16_t offset) {
    if (!dev) return 0;
    return inl(dev->io_base + VIRTIO_PCI_CONFIG + offset);
}

void virtio_pci_config_write8(virtio_pci_device_t *dev, uint16_t offset, uint8_t value) {
    if (!dev) return;
    outb(dev->io_base + VIRTIO_PCI_CONFIG + offset, value);
}

void virtio_pci_config_write16(virtio_pci_device_t *dev, uint16_t offset, uint16_t value) {
    if (!dev) return;
    outw(dev->io_base + VIRTIO_PCI_CONFIG + offset, value);
}

void virtio_pci_config_write32(virtio_pci_device_t *dev, uint16_t offset, uint32_t value) {
    if (!dev) return;
    outl(dev->io_base + VIRTIO_PCI_CONFIG + offset, value);
}

uint8_t virtio_pci_get_isr_status(virtio_pci_device_t *dev) {
    if (!dev) return 0;
    return inb(dev->io_base + VIRTIO_PCI_ISR);
}

int virtio_pci_setup_interrupts(virtio_pci_device_t *dev) {
    if (!dev) return -1;
    
    // Enable interrupts on the PCI device
    pci_enable_interrupts(dev->pci_dev);
    
    // TODO: Set up interrupt handler
    
    return 0;
}

void virtio_pci_handle_interrupt(virtio_pci_device_t *dev) {
    if (!dev) return;
    
    uint8_t isr = virtio_pci_get_isr_status(dev);
    dev->isr_status = isr;
    
    if (isr & VIRTIO_ISR_QUEUE) {
        // Handle queue interrupt
        simple_serial_puts("DEBUG: VirtIO queue interrupt\\n");
    }
    
    if (isr & VIRTIO_ISR_CONFIG) {
        // Handle configuration change interrupt
        simple_serial_puts("DEBUG: VirtIO config change interrupt\\n");
    }
}

void virtio_pci_dump_device_info(virtio_pci_device_t *dev) {
    if (!dev) return;
    
    simple_serial_puts("VirtIO PCI Device:\\n");
    simple_serial_puts("  Device ID: ");
    // TODO: Add hex printing
    simple_serial_puts("\\n");
    simple_serial_puts("  I/O Base: ");
    // TODO: Add hex printing
    simple_serial_puts("\\n");
}

const char *virtio_pci_get_device_name(uint16_t device_id) {
    switch (device_id) {
        case VIRTIO_DEVICE_ID_NET: return "VirtIO Network";
        case VIRTIO_DEVICE_ID_BLOCK: return "VirtIO Block";
        case VIRTIO_DEVICE_ID_CONSOLE: return "VirtIO Console";
        case VIRTIO_DEVICE_ID_ENTROPY: return "VirtIO Entropy";
        case VIRTIO_DEVICE_ID_BALLOON: return "VirtIO Balloon";
        case VIRTIO_DEVICE_ID_SCSI: return "VirtIO SCSI";
        case VIRTIO_DEVICE_ID_9P: return "VirtIO 9P";
        case VIRTIO_DEVICE_ID_GPU: return "VirtIO GPU";
        default: return "Unknown VirtIO Device";
    }
}

// Buffer management functions
int virtio_pci_add_buffer(virtio_pci_device_t *dev, uint16_t queue_idx, void *buffer, uint32_t len, bool write) {
    if (!dev || queue_idx >= 16 || !buffer || len == 0) return -1;
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    if (!queue->desc) return -1;
    
    // Find a free descriptor
    uint16_t desc_idx = queue->last_avail_idx % queue->queue_size;
    virtio_ring_desc_t *desc = &queue->desc[desc_idx];
    
    // Set up descriptor
    desc->addr = (uint64_t)(uintptr_t)buffer;
    desc->len = len;
    desc->flags = write ? VIRTIO_DESC_F_WRITE : 0;
    desc->next = 0;
    
    // Add to available ring
    queue->avail->ring[queue->last_avail_idx % queue->queue_size] = desc_idx;
    queue->last_avail_idx++;
    
    // Update available index
    queue->avail->idx = queue->last_avail_idx;
    
    return 0;
}

int virtio_pci_add_buffers(virtio_pci_device_t *dev, uint16_t queue_idx, void **buffers, uint32_t *lens, bool *writes, uint16_t count) {
    if (!dev || queue_idx >= 16 || !buffers || !lens || count == 0) return -1;
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    if (!queue->desc) return -1;
    
    uint16_t first_desc_idx = queue->last_avail_idx % queue->queue_size;
    
    // Set up descriptors
    for (uint16_t i = 0; i < count; i++) {
        uint16_t desc_idx = (queue->last_avail_idx + i) % queue->queue_size;
        virtio_ring_desc_t *desc = &queue->desc[desc_idx];
        
        desc->addr = (uint64_t)(uintptr_t)buffers[i];
        desc->len = lens[i];
        desc->flags = (writes && writes[i]) ? VIRTIO_DESC_F_WRITE : 0;
        
        if (i < count - 1) {
            desc->flags |= VIRTIO_DESC_F_NEXT;
            desc->next = (desc_idx + 1) % queue->queue_size;
        } else {
            desc->next = 0;
        }
    }
    
    // Add to available ring
    queue->avail->ring[queue->last_avail_idx % queue->queue_size] = first_desc_idx;
    queue->last_avail_idx++;
    
    // Update available index
    queue->avail->idx = queue->last_avail_idx;
    
    return 0;
}

void *virtio_pci_get_buffer(virtio_pci_device_t *dev, uint16_t queue_idx, uint32_t *len) {
    if (!dev || queue_idx >= 16) return NULL;
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    if (!queue->used) return NULL;
    
    if (queue->last_used_idx == queue->used->idx) {
        return NULL; // No used buffers
    }
    
    virtio_ring_used_elem_t *used_elem = &queue->used->ring[queue->last_used_idx % queue->queue_size];
    if (len) *len = used_elem->len;
    
    // Get the buffer from the descriptor
    virtio_ring_desc_t *desc = &queue->desc[used_elem->id];
    void *buffer = (void *)(uintptr_t)desc->addr;
    
    queue->last_used_idx++;
    
    return buffer;
}

bool virtio_pci_has_used_buffer(virtio_pci_device_t *dev, uint16_t queue_idx) {
    if (!dev || queue_idx >= 16) return false;
    
    virtio_queue_t *queue = &dev->queues[queue_idx];
    if (!queue->used) return false;
    
    return queue->last_used_idx != queue->used->idx;
}

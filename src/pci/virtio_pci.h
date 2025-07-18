#ifndef VIRTIO_PCI_H
#define VIRTIO_PCI_H

#include <stdint.h>
#include <stdbool.h>
#include "pci.h"

// VirtIO PCI Vendor ID
#define VIRTIO_PCI_VENDOR_ID 0x1AF4

// VirtIO Device IDs
#define VIRTIO_DEVICE_ID_NET       0x1000
#define VIRTIO_DEVICE_ID_BLOCK     0x1001
#define VIRTIO_DEVICE_ID_CONSOLE   0x1003
#define VIRTIO_DEVICE_ID_ENTROPY   0x1005
#define VIRTIO_DEVICE_ID_BALLOON   0x1002
#define VIRTIO_DEVICE_ID_SCSI      0x1004
#define VIRTIO_DEVICE_ID_9P        0x1009
#define VIRTIO_DEVICE_ID_GPU       0x1050

// VirtIO PCI Configuration Registers (Legacy)
#define VIRTIO_PCI_HOST_FEATURES    0x00
#define VIRTIO_PCI_GUEST_FEATURES   0x04
#define VIRTIO_PCI_QUEUE_PFN        0x08
#define VIRTIO_PCI_QUEUE_NUM        0x0C
#define VIRTIO_PCI_QUEUE_SEL        0x0E
#define VIRTIO_PCI_QUEUE_NOTIFY     0x10
#define VIRTIO_PCI_STATUS           0x12
#define VIRTIO_PCI_ISR              0x13
#define VIRTIO_PCI_CONFIG           0x14

// VirtIO Status Register Values
#define VIRTIO_STATUS_ACKNOWLEDGE    0x01
#define VIRTIO_STATUS_DRIVER         0x02
#define VIRTIO_STATUS_DRIVER_OK      0x04
#define VIRTIO_STATUS_FEATURES_OK    0x08
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 0x40
#define VIRTIO_STATUS_FAILED         0x80

// VirtIO ISR Status Values
#define VIRTIO_ISR_QUEUE             0x01
#define VIRTIO_ISR_CONFIG            0x02

// VirtIO Feature Bits (Common)
#define VIRTIO_F_NOTIFY_ON_EMPTY     24
#define VIRTIO_F_ANY_LAYOUT          27
#define VIRTIO_F_RING_INDIRECT_DESC  28
#define VIRTIO_F_RING_EVENT_IDX      29
#define VIRTIO_F_VERSION_1           32

// VirtIO Queue Maximum Size
#define VIRTIO_QUEUE_MAX_SIZE        1024

// VirtIO Descriptor Flags
#define VIRTIO_DESC_F_NEXT           1
#define VIRTIO_DESC_F_WRITE          2
#define VIRTIO_DESC_F_INDIRECT       4

// VirtIO Ring Descriptor
typedef struct virtio_ring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} virtio_ring_desc_t;

// VirtIO Ring Available
typedef struct virtio_ring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} virtio_ring_avail_t;

// VirtIO Ring Used Element
typedef struct virtio_ring_used_elem {
    uint32_t id;
    uint32_t len;
} virtio_ring_used_elem_t;

// VirtIO Ring Used
typedef struct virtio_ring_used {
    uint16_t flags;
    uint16_t idx;
    virtio_ring_used_elem_t ring[];
} virtio_ring_used_t;

// VirtIO Queue Structure
typedef struct virtio_queue {
    uint16_t queue_size;
    uint16_t queue_select;
    uint32_t queue_pfn;
    uint16_t last_avail_idx;
    uint16_t last_used_idx;
    
    virtio_ring_desc_t *desc;
    virtio_ring_avail_t *avail;
    virtio_ring_used_t *used;
    
    void *desc_phys;
    void *avail_phys;
    void *used_phys;
} virtio_queue_t;

// VirtIO PCI Device Structure
typedef struct virtio_pci_device {
    pci_device_t *pci_dev;
    uint32_t io_base;
    bool use_mmio;          // True if device uses memory-mapped I/O, false for port I/O
    uint32_t features;
    uint32_t guest_features;
    uint8_t status;
    uint8_t isr_status;
    uint16_t config_offset;
    
    virtio_queue_t queues[16];
    uint16_t num_queues;
    
    void *device_config;
    size_t config_size;
    
    bool initialized;
} virtio_pci_device_t;

// VirtIO PCI Transport Functions
int virtio_pci_init(void);
void virtio_pci_shutdown(void);

// Device Discovery and Management
virtio_pci_device_t *virtio_pci_find_device(uint16_t device_id);
int virtio_pci_probe_device(pci_device_t *pci_dev, virtio_pci_device_t *virtio_dev);
int virtio_pci_init_device(virtio_pci_device_t *dev);
void virtio_pci_reset_device(virtio_pci_device_t *dev);

// Feature Negotiation
uint32_t virtio_pci_get_features(virtio_pci_device_t *dev);
int virtio_pci_set_features(virtio_pci_device_t *dev, uint32_t features);
bool virtio_pci_has_feature(virtio_pci_device_t *dev, uint32_t feature);

// Status Management
uint8_t virtio_pci_get_status(virtio_pci_device_t *dev);
int virtio_pci_set_status(virtio_pci_device_t *dev, uint8_t status);
int virtio_pci_add_status(virtio_pci_device_t *dev, uint8_t status);

// Queue Management
int virtio_pci_setup_queue(virtio_pci_device_t *dev, uint16_t queue_idx, uint16_t size);
int virtio_pci_activate_queue(virtio_pci_device_t *dev, uint16_t queue_idx);
void virtio_pci_notify_queue(virtio_pci_device_t *dev, uint16_t queue_idx);
uint16_t virtio_pci_get_queue_size(virtio_pci_device_t *dev, uint16_t queue_idx);

// Queue Operations
int virtio_pci_add_buffer(virtio_pci_device_t *dev, uint16_t queue_idx, 
                         void *buffer, uint32_t len, bool write);
int virtio_pci_add_buffers(virtio_pci_device_t *dev, uint16_t queue_idx,
                          void **buffers, uint32_t *lens, bool *writes, uint16_t count);
void *virtio_pci_get_buffer(virtio_pci_device_t *dev, uint16_t queue_idx, uint32_t *len);
bool virtio_pci_has_used_buffer(virtio_pci_device_t *dev, uint16_t queue_idx);

// Configuration Space Access
uint8_t virtio_pci_config_read8(virtio_pci_device_t *dev, uint16_t offset);
uint16_t virtio_pci_config_read16(virtio_pci_device_t *dev, uint16_t offset);
uint32_t virtio_pci_config_read32(virtio_pci_device_t *dev, uint16_t offset);
void virtio_pci_config_write8(virtio_pci_device_t *dev, uint16_t offset, uint8_t value);
void virtio_pci_config_write16(virtio_pci_device_t *dev, uint16_t offset, uint16_t value);
void virtio_pci_config_write32(virtio_pci_device_t *dev, uint16_t offset, uint32_t value);

// Interrupt Handling
uint8_t virtio_pci_get_isr_status(virtio_pci_device_t *dev);
int virtio_pci_setup_interrupts(virtio_pci_device_t *dev);
void virtio_pci_handle_interrupt(virtio_pci_device_t *dev);

// Utility Functions
void virtio_pci_dump_device_info(virtio_pci_device_t *dev);
const char *virtio_pci_get_device_name(uint16_t device_id);

#endif // VIRTIO_PCI_H

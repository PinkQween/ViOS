#include "virtio_pci.h"
#include "vigfx/virtio_gpu.h"
#include "debug/simple_serial.h"

// VirtIO GPU PCI Integration Example
// This shows how to use the PCI transport layer with VirtIO GPU

static virtio_pci_device_t *virtio_gpu_pci_device = NULL;

// Initialize VirtIO GPU using PCI transport
int virtio_gpu_pci_init(void) {
    simple_serial_puts("DEBUG: Initializing VirtIO GPU PCI transport...\\n");
    
    // Find VirtIO GPU device
    virtio_gpu_pci_device = virtio_pci_find_device(VIRTIO_DEVICE_ID_GPU);
    if (!virtio_gpu_pci_device) {
        simple_serial_puts("DEBUG: No VirtIO GPU device found\\n");
        return -1;
    }
    
    // Initialize the device
    if (virtio_pci_init_device(virtio_gpu_pci_device) != 0) {
        simple_serial_puts("DEBUG: Failed to initialize VirtIO GPU device\\n");
        return -1;
    }
    
    // Get device features
    uint32_t features = virtio_pci_get_features(virtio_gpu_pci_device);
    simple_serial_puts("DEBUG: VirtIO GPU features: ");
    simple_serial_put_hex(features);
    simple_serial_puts("\n");
    
    // Setup basic features (example)
    uint32_t guest_features = 0;
    // Add desired features here
    virtio_pci_set_features(virtio_gpu_pci_device, guest_features);
    
    // Set FEATURES_OK status
    virtio_pci_add_status(virtio_gpu_pci_device, VIRTIO_STATUS_FEATURES_OK);
    
    // Check if features are accepted
    if (!(virtio_pci_get_status(virtio_gpu_pci_device) & VIRTIO_STATUS_FEATURES_OK)) {
        simple_serial_puts("DEBUG: VirtIO GPU rejected features\\n");
        return -1;
    }
    
    // Setup queues (VirtIO GPU typically has 2 queues: control and cursor)
    if (virtio_pci_setup_queue(virtio_gpu_pci_device, 0, 64) != 0) {
        simple_serial_puts("DEBUG: Failed to setup control queue\\n");
        return -1;
    }
    
    if (virtio_pci_setup_queue(virtio_gpu_pci_device, 1, 16) != 0) {
        simple_serial_puts("DEBUG: Failed to setup cursor queue\\n");
        return -1;
    }
    
    // Activate queues
    virtio_pci_activate_queue(virtio_gpu_pci_device, 0);
    virtio_pci_activate_queue(virtio_gpu_pci_device, 1);
    
    // Set DRIVER_OK status
    virtio_pci_add_status(virtio_gpu_pci_device, VIRTIO_STATUS_DRIVER_OK);
    
    simple_serial_puts("DEBUG: VirtIO GPU PCI transport initialized successfully\\n");
    return 0;
}

// Send command to VirtIO GPU
int virtio_gpu_pci_send_command(const void *cmd, size_t cmd_size, void *resp, size_t resp_size) {
    if (!virtio_gpu_pci_device) {
        simple_serial_puts("DEBUG: No VirtIO GPU device available\n");
        return -1;
    }
    
    if (!cmd || cmd_size == 0) {
        simple_serial_puts("DEBUG: Invalid command parameters\n");
        return -1;
    }
    
    simple_serial_puts("DEBUG: Sending VirtIO GPU command, size: ");
    simple_serial_put_hex(cmd_size);
    simple_serial_puts("\n");
    
    // Add command buffer to queue
    int result = virtio_pci_add_buffer(virtio_gpu_pci_device, 0, (void *)cmd, cmd_size, false);
    if (result != 0) {
        simple_serial_puts("DEBUG: Failed to add command buffer to VirtIO GPU\n");
        return -1;
    }
    
    // Add response buffer if expected
    if (resp && resp_size > 0) {
        result = virtio_pci_add_buffer(virtio_gpu_pci_device, 0, resp, resp_size, true);
        if (result != 0) {
            simple_serial_puts("DEBUG: Failed to add response buffer to VirtIO GPU\n");
            return -1;
        }
    }
    
    // Notify the device
    virtio_pci_notify_queue(virtio_gpu_pci_device, 0);
    
    simple_serial_puts("DEBUG: VirtIO GPU command sent successfully\n");
    return 0;
}

// Get VirtIO GPU configuration
int virtio_gpu_pci_get_config(void *config, size_t config_size) {
    if (!virtio_gpu_pci_device || !config) {
        return -1;
    }
    
    // Read configuration space
    for (size_t i = 0; i < config_size; i++) {
        ((uint8_t*)config)[i] = virtio_pci_config_read8(virtio_gpu_pci_device, i);
    }
    
    return 0;
}

// Update the existing virtio_gpu.c stub functions to use PCI transport
int virtio_gpu_pci_init_stub(void) {
    return virtio_gpu_pci_init();
}

int virtio_gpu_pci_send_command_stub(const void *cmd, size_t cmd_size, void *resp, size_t resp_size) {
    return virtio_gpu_pci_send_command(cmd, cmd_size, resp, resp_size);
}

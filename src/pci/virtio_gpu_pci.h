#ifndef VIRTIO_GPU_PCI_H
#define VIRTIO_GPU_PCI_H

#include <stdint.h>
#include <stddef.h>
#include "virtio_pci.h"

// VirtIO GPU PCI Integration Functions
int virtio_gpu_pci_init(void);
int virtio_gpu_pci_send_command(const void *cmd, size_t cmd_size, void *resp, size_t resp_size);
int virtio_gpu_pci_get_config(void *config, size_t config_size);

// Stub functions for existing API compatibility
int virtio_gpu_pci_init_stub(void);
int virtio_gpu_pci_send_command_stub(const void *cmd, size_t cmd_size, void *resp, size_t resp_size);

#endif // VIRTIO_GPU_PCI_H

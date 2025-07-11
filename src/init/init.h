#ifndef INIT_H
#define INIT_H

#include <stdint.h>
#include <stdbool.h>
#include "task/process.h"

// Service states
#define SERVICE_STATE_STOPPED    0
#define SERVICE_STATE_STARTING   1
#define SERVICE_STATE_RUNNING    2
#define SERVICE_STATE_STOPPING   3
#define SERVICE_STATE_FAILED     4

// Service types
#define SERVICE_TYPE_ESSENTIAL   0  // Must be running for system to function
#define SERVICE_TYPE_OPTIONAL    1  // Can fail without system failure
#define SERVICE_TYPE_USER        2  // User-space services

// Maximum number of services
#define MAX_SERVICES 32

struct service {
    char name[64];
    char executable_path[256];
    uint8_t service_type;
    uint8_t state;
    uint16_t restart_count;
    uint32_t startup_time;
    struct process *process;
    bool auto_restart;
    bool enabled;
    
    // Dependencies
    char dependencies[8][64];  // Max 8 dependencies per service
    int dependency_count;
    
    // Service-specific data
    void *service_data;
};

// Core system services
extern struct service core_services[];

// Init system functions
int init_system_initialize(void);
int init_start_service(const char *service_name);
int init_stop_service(const char *service_name);
int init_restart_service(const char *service_name);
struct service *init_get_service(const char *service_name);
int init_register_service(struct service *service);
void init_monitor_services(void);

// Service management
int service_create(const char *name, const char *executable_path, uint8_t type);
int service_add_dependency(const char *service_name, const char *dependency);
bool service_dependencies_met(struct service *service);

// Core service definitions
int graphics_server_init(void);
int terminal_server_init(void);
int window_manager_init(void);

#endif

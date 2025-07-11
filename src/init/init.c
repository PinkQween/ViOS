#include "init.h"
#include "common/string.h"
#include "common/debug.h"
#include "task/scheduler.h"
#include "memory/memory.h"
#include "fs/vfs.h"

// Global service registry
static struct service services[MAX_SERVICES];
static int service_count = 0;

// Core services configuration
struct service core_services[] = {
    {
        .name = "graphics_server",
        .executable_path = "/system/bin/graphics_server",
        .service_type = SERVICE_TYPE_ESSENTIAL,
        .state = SERVICE_STATE_STOPPED,
        .auto_restart = true,
        .enabled = true,
        .dependency_count = 0
    },
    {
        .name = "terminal_server", 
        .executable_path = "/system/bin/terminal_server",
        .service_type = SERVICE_TYPE_ESSENTIAL,
        .state = SERVICE_STATE_STOPPED,
        .auto_restart = true,
        .enabled = true,
        .dependency_count = 1
    },
    {
        .name = "window_manager",
        .executable_path = "/system/bin/window_manager",
        .service_type = SERVICE_TYPE_OPTIONAL,
        .state = SERVICE_STATE_STOPPED,
        .auto_restart = true,
        .enabled = false,  // Started on demand
        .dependency_count = 1
    }
};

int init_system_initialize(void) {
    debug_print("Initializing init system...\n");
    
    // Initialize service registry
    for (int i = 0; i < MAX_SERVICES; i++) {
        memset(&services[i], 0, sizeof(struct service));
    }
    service_count = 0;
    
    // Register core services
    strcpy(core_services[1].dependencies[0], "graphics_server");  // terminal depends on graphics
    strcpy(core_services[2].dependencies[0], "graphics_server");  // window manager depends on graphics
    
    for (int i = 0; i < 3; i++) {
        if (init_register_service(&core_services[i]) < 0) {
            debug_print("Failed to register core service: %s\n", core_services[i].name);
            return -1;
        }
    }
    
    // Start essential services
    debug_print("Starting essential services...\n");
    if (init_start_service("graphics_server") < 0) {
        debug_print("Failed to start graphics server\n");
        return -1;
    }
    
    if (init_start_service("terminal_server") < 0) {
        debug_print("Failed to start terminal server\n");
        return -1;
    }
    
    debug_print("Init system initialized successfully\n");
    return 0;
}

int init_register_service(struct service *service) {
    if (service_count >= MAX_SERVICES) {
        debug_print("Service registry full\n");
        return -1;
    }
    
    // Check for duplicate names
    for (int i = 0; i < service_count; i++) {
        if (strcmp(services[i].name, service->name) == 0) {
            debug_print("Service %s already registered\n", service->name);
            return -1;
        }
    }
    
    memcpy(&services[service_count], service, sizeof(struct service));
    service_count++;
    
    debug_print("Registered service: %s\n", service->name);
    return 0;
}

struct service *init_get_service(const char *service_name) {
    for (int i = 0; i < service_count; i++) {
        if (strcmp(services[i].name, service_name) == 0) {
            return &services[i];
        }
    }
    return NULL;
}

bool service_dependencies_met(struct service *service) {
    for (int i = 0; i < service->dependency_count; i++) {
        struct service *dep = init_get_service(service->dependencies[i]);
        if (!dep || dep->state != SERVICE_STATE_RUNNING) {
            debug_print("Service %s dependency %s not met\n", 
                       service->name, service->dependencies[i]);
            return false;
        }
    }
    return true;
}

int init_start_service(const char *service_name) {
    struct service *service = init_get_service(service_name);
    if (!service) {
        debug_print("Service %s not found\n", service_name);
        return -1;
    }
    
    if (service->state == SERVICE_STATE_RUNNING) {
        debug_print("Service %s already running\n", service_name);
        return 0;
    }
    
    if (!service->enabled) {
        debug_print("Service %s is disabled\n", service_name);
        return -1;
    }
    
    // Check dependencies
    if (!service_dependencies_met(service)) {
        debug_print("Dependencies not met for service %s\n", service_name);
        return -1;
    }
    
    service->state = SERVICE_STATE_STARTING;
    debug_print("Starting service: %s\n", service_name);
    
    // For now, call the init functions directly
    // In a full implementation, this would spawn processes
    if (strcmp(service_name, "graphics_server") == 0) {
        if (graphics_server_init() < 0) {
            service->state = SERVICE_STATE_FAILED;
            return -1;
        }
    } else if (strcmp(service_name, "terminal_server") == 0) {
        if (terminal_server_init() < 0) {
            service->state = SERVICE_STATE_FAILED;
            return -1;
        }
    } else if (strcmp(service_name, "window_manager") == 0) {
        if (window_manager_init() < 0) {
            service->state = SERVICE_STATE_FAILED;
            return -1;
        }
    }
    
    service->state = SERVICE_STATE_RUNNING;
    service->startup_time = 0; // TODO: implement timing
    
    debug_print("Service %s started successfully\n", service_name);
    return 0;
}

int init_stop_service(const char *service_name) {
    struct service *service = init_get_service(service_name);
    if (!service) {
        debug_print("Service %s not found\n", service_name);
        return -1;
    }
    
    if (service->state != SERVICE_STATE_RUNNING) {
        debug_print("Service %s is not running\n", service_name);
        return 0;
    }
    
    service->state = SERVICE_STATE_STOPPING;
    debug_print("Stopping service: %s\n", service_name);
    
    // TODO: Implement proper process termination
    if (service->process) {
        // process_terminate(service->process);
    }
    
    service->state = SERVICE_STATE_STOPPED;
    service->process = NULL;
    
    debug_print("Service %s stopped\n", service_name);
    return 0;
}

int init_restart_service(const char *service_name) {
    if (init_stop_service(service_name) < 0) {
        return -1;
    }
    
    return init_start_service(service_name);
}

void init_monitor_services(void) {
    for (int i = 0; i < service_count; i++) {
        struct service *service = &services[i];
        
        // Check if service process has crashed
        if (service->state == SERVICE_STATE_RUNNING && service->process) {
            // TODO: Check if process is still alive
            // if (process_is_dead(service->process)) {
            //     service->state = SERVICE_STATE_FAILED;
            //     if (service->auto_restart) {
            //         init_restart_service(service->name);
            //     }
            // }
        }
    }
}

int service_create(const char *name, const char *executable_path, uint8_t type) {
    if (service_count >= MAX_SERVICES) {
        return -1;
    }
    
    struct service *service = &services[service_count];
    strcpy(service->name, name);
    strcpy(service->executable_path, executable_path);
    service->service_type = type;
    service->state = SERVICE_STATE_STOPPED;
    service->auto_restart = true;
    service->enabled = true;
    service->dependency_count = 0;
    
    service_count++;
    return 0;
}

int service_add_dependency(const char *service_name, const char *dependency) {
    struct service *service = init_get_service(service_name);
    if (!service) {
        return -1;
    }
    
    if (service->dependency_count >= 8) {
        return -1;
    }
    
    strcpy(service->dependencies[service->dependency_count], dependency);
    service->dependency_count++;
    
    return 0;
}

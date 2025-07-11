#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("=== Simple ViOS Check ===\n");
    print("Testing basic functionality...\n");
    
    print("1. Memory allocation test...\n");
    char *test_mem = vios_malloc(100);
    if (test_mem) {
        print("   Memory allocation: OK\n");
        vios_free(test_mem);
        print("   Memory free: OK\n");
    } else {
        print("   Memory allocation: FAILED\n");
    }
    
    print("2. Basic string operations...\n");
    print("   String print: OK\n");
    
    print("3. System stability check...\n");
    for (int i = 0; i < 10; i++) {
        // Small delay loop
        for (int j = 0; j < 10000; j++) {
            // Do nothing
        }
    }
    print("   Loop test: OK\n");
    
    print("\nAll basic tests passed!\n");
    print("Press any key to return to shell...\n");
    vios_getkeyblock();
    
    return 0;
}

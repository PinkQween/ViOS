#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("=== Malicious Test Program ===\n");
    print("Testing security measures...\n\n");

    // Test 1: Use-after-free attack (should be detected)
    print("Test 1: Attempting use-after-free attack...\n");
    char *ptr = malloc(20);
    if (ptr)
    {
        strcpy(ptr, "attack string");
        print("Memory allocated and filled\n");

        free(ptr);
        print("Memory freed\n");

        // This should trigger security violation
        print("Attempting to access freed memory...\n");
        ptr[0] = 'X'; // This should be detected!
        print("WARNING: Use-after-free was not detected!\n");
    }

    // Test 2: Double free attack (should be detected)
    print("\nTest 2: Attempting double-free attack...\n");
    char *ptr2 = malloc(30);
    if (ptr2)
    {
        strcpy(ptr2, "double free test");
        print("Memory allocated\n");

        free(ptr2);
        print("First free completed\n");

        // This should trigger security violation
        print("Attempting second free...\n");
        free(ptr2); // This should be detected!
        print("WARNING: Double-free was not detected!\n");
    }

    // Test 3: Invalid pointer free (should be detected)
    print("\nTest 3: Attempting to free invalid pointer...\n");
    char *invalid_ptr = (char *)0x12345678; // Invalid address
    print("Attempting to free invalid pointer...\n");
    free(invalid_ptr); // This should be detected!
    print("WARNING: Invalid free was not detected!\n");

    // Test 4: Large allocation (should be blocked)
    print("\nTest 4: Attempting suspiciously large allocation...\n");
    char *large_ptr = malloc(2000000000); // 2GB - should be blocked
    if (large_ptr)
    {
        print("WARNING: Large allocation was not blocked!\n");
        free(large_ptr);
    }
    else
    {
        print("SUCCESS: Large allocation correctly blocked\n");
    }

    print("\n=== Malicious tests completed ===\n");
    print("If you see this message, some security measures may have failed.\n");

    while (1)
    {
    }
    return 0;
}
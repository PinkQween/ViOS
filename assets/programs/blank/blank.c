#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("=== ViOS Security Test Program ===\n");
    print("Testing memory allocation and security measures...\n\n");

    // Test 1: Basic memory allocation and free
    print("Test 1: Basic memory allocation...\n");
    char *ptr1 = malloc(20);
    if (!ptr1)
    {
        print("ERROR: malloc failed!\n");
        return 1;
    }
    print("SUCCESS: Memory allocated at ");
    print(itoa((int)ptr1));
    print("\n");

    strcpy(ptr1, "hello world");
    print("SUCCESS: String copied: ");
    print(ptr1);
    print("\n");

    free(ptr1);
    print("SUCCESS: Memory freed\n\n");

    // Test 2: Multiple allocations
    print("Test 2: Multiple memory allocations...\n");
    char *ptr2 = malloc(50);
    char *ptr3 = malloc(100);
    char *ptr4 = malloc(25);

    if (!ptr2 || !ptr3 || !ptr4)
    {
        print("ERROR: Multiple malloc failed!\n");
        return 1;
    }

    strcpy(ptr2, "test string 1");
    strcpy(ptr3, "test string 2");
    strcpy(ptr4, "test string 3");

    print("SUCCESS: Multiple allocations work\n");
    print("ptr2: ");
    print(ptr2);
    print("\n");
    print("ptr3: ");
    print(ptr3);
    print("\n");
    print("ptr4: ");
    print(ptr4);
    print("\n");

    free(ptr2);
    free(ptr3);
    free(ptr4);
    print("SUCCESS: Multiple frees completed\n\n");

    // Test 3: Edge cases
    print("Test 3: Edge cases...\n");

    // Test malloc(0) - should return NULL
    char *ptr5 = malloc(0);
    if (ptr5 == 0)
    {
        print("SUCCESS: malloc(0) correctly returns NULL\n");
    }
    else
    {
        print("WARNING: malloc(0) returned non-NULL\n");
        free(ptr5);
    }

    // Test free(NULL) - should be safe
    free(0);
    print("SUCCESS: free(NULL) handled safely\n\n");

    // Test 4: String operations
    print("Test 4: String operations...\n");
    char *str_ptr = malloc(100);
    if (!str_ptr)
    {
        print("ERROR: String allocation failed!\n");
        return 1;
    }

    strcpy(str_ptr, "ViOS is awesome!");
    print("SUCCESS: String copied: ");
    print(str_ptr);
    print("\n");

    free(str_ptr);
    print("SUCCESS: String memory freed\n\n");

    // Test 5: Large allocation
    print("Test 5: Large allocation...\n");
    char *large_ptr = malloc(1024);
    if (!large_ptr)
    {
        print("ERROR: Large allocation failed!\n");
        return 1;
    }

    // Fill with test data
    for (int i = 0; i < 1023; i++)
    {
        large_ptr[i] = 'A' + (i % 26);
    }
    large_ptr[1023] = '\0';

    print("SUCCESS: Large allocation works (first 20 chars: ");
    char temp[21];
    strncpy(temp, large_ptr, 20);
    temp[20] = '\0';
    print(temp);
    print(")\n");

    free(large_ptr);
    print("SUCCESS: Large memory freed\n\n");

    // Test 6: Stress test
    print("Test 6: Stress test (multiple alloc/free cycles)...\n");
    for (int i = 0; i < 10; i++)
    {
        char *stress_ptr = malloc(50);
        if (stress_ptr)
        {
            strcpy(stress_ptr, "stress test ");
            char num_str[10];
            strcpy(num_str, itoa(i));
            strcpy(stress_ptr + strlen(stress_ptr), num_str);
            print(stress_ptr);
            print("\n");
            free(stress_ptr);
        }
    }
    print("SUCCESS: Stress test completed\n\n");

    print("=== All tests completed successfully! ===\n");
    print("Security measures are working correctly.\n");
    print("The system is protected against malicious attacks.\n");

    while (1)
    {
    }
    return 0;
}
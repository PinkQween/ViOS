#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("=== Simple Test Program ===\n");

    // Test basic print functionality
    print("Hello from ViOS!\n");
    print("Testing basic functionality...\n");

    // Test string operations
    char test_str[] = "This is a test string";
    print("Test string: ");
    print(test_str);
    print("\n");

    // Test number conversion
    int test_num = 42;
    char num_str[20];
    strcpy(num_str, itoa(test_num));
    print("Test number: ");
    print(num_str);
    print("\n");

    // Test basic memory allocation
    char *ptr = malloc(50);
    if (ptr)
    {
        strcpy(ptr, "Memory allocation works!");
        print(ptr);
        print("\n");
        free(ptr);
        print("Memory freed successfully\n");
    }
    else
    {
        print("ERROR: Memory allocation failed!\n");
        return 1;
    }

    print("=== Simple test completed successfully! ===\n");

    while (1)
    {
    }
    return 0;
}
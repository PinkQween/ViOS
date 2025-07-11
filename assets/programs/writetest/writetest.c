#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char **argv)
{
    print("=== ViOS Write Test Program ===\n");
    print("Testing file write functionality...\n\n");

    // Test writing to an existing file (let's write to an existing file first)
    print("Test 1: Reading existing file to verify filesystem works\n");
    
    const char *read_filename = "0:/shell.elf";
    char *existing_data = vios_read(read_filename);
    if (existing_data) {
        print("SUCCESS: Can read existing files\n");
        vios_free(existing_data);
    } else {
        print("WARNING: Cannot read existing files - filesystem may not be mounted\n");
    }
    
    print("\nTest 2: Attempting to write test data\n");
    const char *test_data = "Hello, this is a test file created by ViOS!\n";
    const char *filename = "0:/testfile.txt";
    
    print("Writing test data to file...\n");
    int result = vios_write(filename, test_data, strlen(test_data));
    
    if (result == 0)
    {
        print("SUCCESS: File written successfully!\n");
        
        print("\nTest 3: Reading back the file content\n");
        char *read_data = vios_read(filename);
        if (read_data)
        {
            print("File content: ");
            print(read_data);
            vios_free(read_data);
        }
        else
        {
            print("ERROR: Could not read back the file\n");
        }
    }
    else
    {
        print("ERROR: Failed to write file\n");
    }
    
    print("\nWrite functionality test complete!\n");
    print("Press any key to return to shell...\n");
    vios_getkeyblock();
    
    return 0;
}

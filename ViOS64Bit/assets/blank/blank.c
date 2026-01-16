#include "vios.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "file.h"

int main(int argc, char** argv)
{
    // Write a test file
    printf("Creating test file: @:/testfile.txt\n");
    
    int fd = fopen("@:/testfile.txt", "w");
    if (fd <= 0) {
        printf("Error: Could not open file for writing!\n");
        return -1;
    }
    
    const char* message = "Hello from ViOS!\nThis is a test of the FAT16 write system.\nFile I/O is working!\n";
    int len = strlen(message);
    
    int written = fwrite(message, 1, len, fd);
    printf("Wrote %i bytes to file\n", written);
    
    fclose(fd);
    vios_print("File created successfully!\n");
    
    // Now read it back and print it
    printf("Reading file back: @:/testfile.txt\n");
    printf("========================================\n");
    
    fd = fopen("@:/testfile.txt", "r");
    if (fd <= 0) {
        printf("Error: Could not open file for reading!\n");
        return -1;
    }
    
    char buffer[512];
    int read_bytes = fread(buffer, 1, sizeof(buffer) - 1, fd);
    buffer[read_bytes] = '\0';  // Null terminate
    
    printf(buffer);
    printf("========================================\n");
    printf("Read %i bytes from file\n", read_bytes);
    
    fclose(fd);
    printf("Test complete!\n");
    
    return 0;
}

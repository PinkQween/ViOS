#include "vios.h"

extern "C" int main(int argc, char **argv);

extern "C" void cpp_start()
{
    struct process_arguments arguments;
    vios_process_get_arguments(&arguments);

    int res = main(arguments.argc, arguments.argv);

    if (res == 0)
    {
        // Handle successful termination
    }
}
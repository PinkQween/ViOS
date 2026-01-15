cmake_minimum_required(VERSION 3.16)
project(ViOS LANGUAGES C ASM_NASM)

# Detect platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_MACOS TRUE)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}. Only Linux and macOS are supported.")
endif()

# Options
option(BUILD_TESTS "Build test suite" OFF)
set(EDK2_ROOT "${CMAKE_SOURCE_DIR}/../../.." CACHE PATH "EDK2 root directory")
set(CROSS_COMPILE "x86_64-elf-" CACHE STRING "Cross compiler prefix")

# Find required tools
find_program(NASM nasm REQUIRED)
find_program(PYTHON3 python3 REQUIRED)
find_program(DD dd REQUIRED)

# Verify cross-compiler
find_program(CROSS_GCC ${CROSS_COMPILE}gcc)
if(NOT CROSS_GCC)
    message(WARNING "Cross compiler '${CROSS_COMPILE}gcc' not found. Build may fail.")
endif()

# Output directories
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Build EDK2 UEFI bootloader
add_custom_command(
    OUTPUT ${OUTPUT_DIR}/ViOS.efi
    COMMAND ${CMAKE_COMMAND} -E echo "Building UEFI bootloader..."
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/build_edk2.sh ${EDK2_ROOT} ${CROSS_COMPILE}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${EDK2_ROOT}/Build/MdeModule/DEBUG_GCC5/X64/ViOS.efi
        ${OUTPUT_DIR}/ViOS.efi
    WORKING_DIRECTORY ${EDK2_ROOT}
    COMMENT "Building UEFI bootloader"
)

# Build kernel
add_custom_command(
    OUTPUT ${OUTPUT_DIR}/kernel.bin
    COMMAND ${CMAKE_COMMAND} -E echo "Building kernel..."
    COMMAND ${CMAKE_COMMAND} -E env "CROSS_COMPILE=${CROSS_COMPILE}" 
        $(MAKE) -C ${CMAKE_SOURCE_DIR}/ViOS64Bit build
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ViOS64Bit/bin/kernel.bin
        ${OUTPUT_DIR}/kernel.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ViOS64Bit
    COMMENT "Building kernel"
)

# Create bootable disk image
add_custom_command(
    OUTPUT ${OUTPUT_DIR}/os.img
    COMMAND ${CMAKE_COMMAND} -E echo "Creating disk image..."
    COMMAND ${DD} if=/dev/zero of=${OUTPUT_DIR}/os.img bs=1M count=700 status=none
    COMMAND ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/create_gpt.py ${OUTPUT_DIR}/os.img
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/populate_disk_${CMAKE_SYSTEM_NAME}.sh 
        ${OUTPUT_DIR}/os.img ${OUTPUT_DIR} ${CMAKE_SOURCE_DIR}/ViOS64Bit
    DEPENDS ${OUTPUT_DIR}/ViOS.efi ${OUTPUT_DIR}/kernel.bin
    COMMENT "Creating bootable disk image"
)

# Default target
add_custom_target(vios ALL DEPENDS ${OUTPUT_DIR}/os.img)

# Clean target  
add_custom_target(clean-all
    COMMAND $(MAKE) -C ${CMAKE_SOURCE_DIR}/ViOS64Bit clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OUTPUT_DIR}
    COMMENT "Cleaning all build artifacts"
)

# Install
install(FILES ${OUTPUT_DIR}/os.img
    DESTINATION bin
    COMPONENT runtime
)

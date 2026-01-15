#!/bin/bash
set -e

EDK2_ROOT="$1"
CROSS_COMPILE="${2:-x86_64-elf-}"

cd "$EDK2_ROOT"

export EDK_TOOLS_PATH="${EDK2_ROOT}/BaseTools"
export GCC5_X64_PREFIX="${CROSS_COMPILE}"

source edksetup.sh BaseTools

build -a X64 -t GCC5 -p MdeModulePkg/MdeModulePkg.dsc \
      -m MdeModulePkg/Application/ViOS64BitDev/ViOS.inf

#!/bin/bash

#/bin/bash
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
if [ "${1,,}" == "img" ]; then
    make img
else
    make all
fi

#!/bin/bash

PROJECT_NAME=$(basename $(pwd))
echo "Running GDB ${PROJECT_NAME}"

arm-none-eabi-gdb -tui -ex "target extended-remote localhost:4242" build/${PROJECT_NAME}.elf

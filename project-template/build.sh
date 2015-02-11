#!/bin/bash -e

PROJECT_NAME=$(basename $(dirname $(pwd)))

mkdir -p build
cd build
rm ${PROJECT_NAME}.elf || echo "Cannot remove. ${PROJECT_NAME}.elf not build?"
make ${PROJECT_NAME}.bin && \
make ${PROJECT_NAME}.list


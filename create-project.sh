#!/bin/bash

function die {
  echo >&2 "$@"
  exit 1
}

[ "$#" -eq 2 ] || die "usage: $(basename $0) <project-name> <chip>"

DIR=$(cd $(dirname "$0") && pwd)
PROJECT_NAME=$1
CHIP=$2
PROJECT_TEMPLATE_DIR=${DIR}/project-template
STM32LIB_DIR=${DIR}/lib

case ${CHIP} in
STM32F072xB|STM32F072RBT6)
  STM32_CHIP_DEF=STM32F072
  STM32_FLASH_SIZE=$(echo "128 * 1024" | bc)
  STM32_RAM_SIZE=$(echo "16 * 1024" | bc)
  SYSTEM_FILE_NAME=system_stm32f0xx.c
  STARTUP_FILE_NAME=startup_stm32f072xb.s
  LINKER_FILE_NAME=stm32f072rb_flash.ld
  ;;
STM32F103RBT6)
  STM32_CHIP_DEF=STM32F10X_MD
  STM32_FLASH_SIZE=$(echo "128 * 1024" | bc)
  STM32_RAM_SIZE=$(echo "20 * 1024" | bc)
  SYSTEM_FILE_NAME=system_stm32f10x.c
  STARTUP_FILE_NAME=startup_stm32f10x_md.s
  LINKER_FILE_NAME=stm32f10x-128k_flash-20k_ram.ld
  ;;
STM32F103RET6)
  STM32_CHIP_DEF=STM32F10X_CL
  STM32_FLASH_SIZE=$(echo "512 * 1024" | bc)
  STM32_RAM_SIZE=$(echo "64 * 1024" | bc)
  SYSTEM_FILE_NAME=system_stm32f10x.c
  STARTUP_FILE_NAME=startup_stm32f10x_md.s
  LINKER_FILE_NAME=stm32f10x-512k_flash-64k_ram.ld
  ;;
*)
  die "invalid chip"
  ;;
esac

echo "Creating project ${PROJECT_NAME}"
echo "  STM32_CHIP_DEF   = ${STM32_CHIP_DEF}"
echo "  STM32_FLASH_SIZE = ${STM32_FLASH_SIZE}"
echo "  STM32_RAM_SIZE   = ${STM32_RAM_SIZE}"
echo "  STM32LIB_DIR     = ${STM32LIB_DIR}"

if [ ! -f build.sh ]; then
  echo "Creating build.sh"
  cp ${PROJECT_TEMPLATE_DIR}/build.sh build.sh
fi

if [ ! -f run-cmake.sh ]; then
  echo "Creating run-cmake.sh"
  cp ${PROJECT_TEMPLATE_DIR}/run-cmake.sh run-cmake.sh
fi

if [ ! -f run-gdb.sh ]; then
  echo "Creating run-gdb.sh"
  cp ${PROJECT_TEMPLATE_DIR}/run-gdb.sh run-gdb.sh
fi

if [ ! -f run-st-util-server.sh ]; then
  echo "Creating run-st-util-server.sh"
  cp ${PROJECT_TEMPLATE_DIR}/run-st-util-server.sh run-st-util-server.sh
fi

if [ ! -f ${STARTUP_FILE_NAME} ]; then
  echo "Creating ${STARTUP_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/startup/${STARTUP_FILE_NAME} ${STARTUP_FILE_NAME}
fi

if [ ! -f ${LINKER_FILE_NAME} ]; then
  echo "Creating ${LINKER_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/linker/${LINKER_FILE_NAME} ${LINKER_FILE_NAME}
fi

if [ ! -f gcc_stm32.cmake ]; then
  echo "Creating gcc_stm32.cmake"
  cp ${PROJECT_TEMPLATE_DIR}/gcc_stm32.cmake gcc_stm32.cmake
fi

if [ ! -f main.c ]; then
  echo "Creating main.c"
  cp ${PROJECT_TEMPLATE_DIR}/main.c main.c
fi

if [ ! -f platform_config.h ]; then
  echo "Creating platform_config.h"
  cp ${PROJECT_TEMPLATE_DIR}/platform_config/platform_config.h platform_config.h
fi

if [ ! -f interrupts.c ]; then
  echo "Creating interrupts.c"
  cp ${PROJECT_TEMPLATE_DIR}/interrupts.c interrupts.c
fi

if [ ! -f ${SYSTEM_FILE_NAME} ]; then
  echo "Creating ${SYSTEM_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/system/${SYSTEM_FILE_NAME} ${SYSTEM_FILE_NAME}
fi

if [ ! -f CMakeLists.txt ]; then
  echo "Creating CMakeLists.txt"
  cp ${PROJECT_TEMPLATE_DIR}/CMakeLists.txt CMakeLists.txt
  sed -i -- "s|%PROJECT_NAME%|${PROJECT_NAME}|g" CMakeLists.txt
  sed -i -- "s|%STM32LIB_DIR%|${STM32LIB_DIR}|g" CMakeLists.txt
  sed -i -- "s|%CHIP%|${CHIP}|g" CMakeLists.txt
  sed -i -- "s|%LINKER_FILE_NAME%|${LINKER_FILE_NAME}|g" CMakeLists.txt
  sed -i -- "s|%STARTUP_FILE_NAME%|${STARTUP_FILE_NAME}|g" CMakeLists.txt
  sed -i -- "s|%SYSTEM_FILE_NAME%|${SYSTEM_FILE_NAME}|g" CMakeLists.txt
  sed -i -- "s|%STM32_CHIP_DEF%|${STM32_CHIP_DEF}|g" CMakeLists.txt
  sed -i -- "s|%STM32_FLASH_SIZE%|${STM32_FLASH_SIZE}|g" CMakeLists.txt
  sed -i -- "s|%STM32_RAM_SIZE%|${STM32_RAM_SIZE}|g" CMakeLists.txt
fi

#!/bin/bash

DIR=$(cd $(dirname "$0") && pwd)
PROJECT_NAME=$1
CHIP=$2
PROJECT_TEMPLATE_DIR=${DIR}/project-template
STM32LIB_DIR=${DIR}/lib

echo "Creating project ${PROJECT_NAME}"
echo "  CHIP=${CHIP}"
echo "  STM32LIB_DIR=${STM32LIB_DIR}"

STM32_CHIP_DEF=STM32F072xB
STM32_FLASH_SIZE=$(echo "128 * 1024" | bc)
STM32_RAM_SIZE=$(echo "16 * 1024" | bc)

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

STARTUP_FILE_NAME=startup_stm32f072xb.s
if [ ! -f ${STARTUP_FILE_NAME} ]; then
  echo "Creating ${STARTUP_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/startup/${STARTUP_FILE_NAME} ${STARTUP_FILE_NAME}
fi

LINKER_FILE_NAME=stm32f072rb_flash.ld
if [ ! -f ${LINKER_FILE_NAME} ]; then
  echo "Creating ${LINKER_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/${LINKER_FILE_NAME} ${LINKER_FILE_NAME}
fi

if [ ! -f gcc_stm32.cmake ]; then
  echo "Creating gcc_stm32.cmake"
  cp ${PROJECT_TEMPLATE_DIR}/gcc_stm32.cmake gcc_stm32.cmake
fi

SYSTEM_FILE_NAME=system_stm32f0xx.c
if [ ! -f ${SYSTEM_FILE_NAME} ]; then
  echo "Creating ${SYSTEM_FILE_NAME}"
  cp ${PROJECT_TEMPLATE_DIR}/${SYSTEM_FILE_NAME} ${SYSTEM_FILE_NAME}
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

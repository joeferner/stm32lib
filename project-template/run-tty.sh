#!/bin/bash

TTY_DEV_PATH=/dev/serial/by-path/

function getTTYInfo() {
  path=${TTY_DEV_PATH}/$1
  ttyPath=$(/sbin/udevadm info -q path -n ${path})
  properties="$(/sbin/udevadm info --query=property -p ${ttyPath})"
  devName=$(echo "${properties}" | awk -F= '$1=="DEVNAME" {print $2}')
  idSerial=$(echo "${properties}" | awk -F= '$1=="ID_SERIAL" {print $2}')
  idModel=$(echo "${properties}" | awk -F= '$1=="ID_MODEL_FROM_DATABASE" {print $2}')
}


baud=$(cat platform_config.h | grep ' DEBUG_BAUD ' | awk '{print $3}')
ttys=($(ls -1 ${TTY_DEV_PATH}))

if [ ${#ttys[@]} -eq 1 ]; then
  tty=${ttys[0]}
else
  for (( i=0; i<${#ttys[@]}; i++ )); do
    f=${ttys[i]}
    getTTYInfo $f
    echo "$i: $devName \"$idSerial\" \"$idModel\""
  done
  echo -n "Choose the tty: "
  read choice
  tty=${ttys[$choice]}
fi

getTTYInfo ${tty}

echo "Connecting to $devName \"$idSerial\" \"$idModel\" at baud $baud"
minicom -D $devName -b $baud

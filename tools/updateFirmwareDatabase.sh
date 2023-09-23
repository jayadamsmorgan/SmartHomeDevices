#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DEVICES_DIR=$SCRIPT_DIR/../Devices
for folder in $DEVICES_DIR/*; do
   if [ -d "$folder" ]; then
      folder_name=$(basename "${folder}")
      echo 'Updating '$folder_name
      pio run -d $folder -s
      firmware_folder=$SCRIPT_DIR/../../HomeServer/tools/firmware/$folder_name
      if [ ! -d "$firmware_folder" ]; then
        mkdir -p "$firmware_folder"
      fi
      cp $folder/.pio/build/esp32/firmware.bin $firmware_folder/latest.bin
   fi
done

echo "Firmware Database Updated"

#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pio run -d $SCRIPT_DIR/../Devices/HomeBasicDevice -s
pio run -d $SCRIPT_DIR/../Devices/HomeLightDevice -s

cp $SCRIPT_DIR/../Devices/HomeBasicDevice/.pio/build/esp32-s3-devkitc-1/firmware.bin $SCRIPT_DIR/../../HomeServer/tools/firmware/BasicDevice/latest.bin
cp $SCRIPT_DIR/../Devices/HomeLightDevice/.pio/build/esp32-s3-devkitc-1/firmware.bin $SCRIPT_DIR/../../HomeServer/tools/firmware/LightDevice/latest.bin

echo "Firmware Database Updated"

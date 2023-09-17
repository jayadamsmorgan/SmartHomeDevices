#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pio run -d $SCRIPT_DIR/../Devices/HomeBasicDevice/ -t compiledb
pio run -d $SCRIPT_DIR/../Devices/HomeLightDevice/ -t compiledb

cp $SCRIPT_DIR/../Devices/HomeBasicDevice/compile_commands.json $SCRIPT_DIR/../HomeDeviceLib/compile_commands.json

echo "Success"

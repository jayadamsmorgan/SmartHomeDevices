#!/bin/sh

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pio run -d $SCRIPT_DIR/../Devices/Home$1 -s

python3 $SCRIPT_DIR/espota.py -i 192.168.1.$2 -a admin -f Devices/Home$1/.pio/build/esp32-s3-devkitc-1/firmware.bin -r -t 3


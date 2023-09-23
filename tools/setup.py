#!/usr/bin/env python3

import os
import inspect
import pyinputplus as pyip
import subprocess


device_types = []
script_directory = os.path.dirname(os.path.abspath(
  inspect.getfile(inspect.currentframe())))
for name in os.listdir(script_directory + "/../Devices/"):
    if name.startswith("Home"):
        device_types.append(name)


print("Select the type of Device you want to use:")
target_type = pyip.inputMenu(device_types, numbered = True)

boards_result = subprocess.run("pio boards esp32", shell=True, capture_output=True, text=True)
if boards_result.returncode != 0:
    subprocess.run("sh " + script_directory + "install_dependencies.sh", shell=True, capture_output=True, text=True)
    boards_result = subprocess.run("pio boards esp32", shell=True, capture_output=True, text=True)

boards = []
board_lines = boards_result.stdout.splitlines()
for board_line in board_lines:
    boards.append(board_line)

del boards[:5]

print("Select your ESP32 board from the list:")
target_board = pyip.inputMenu(boards, numbered = True)

file_path = script_directory + "/../Devices/" + target_type + "/platformio.ini"
file = open(file_path, "w+")
lines = [
    "[env:esp32]\n",
    "platform = espressif32\n",
    "framework = arduino\n",
    "extra_scripts = pre:../../tools/pre_build_script.py\n",
    "build_flags = -I lib -I include\n",
    "lib_deps = HomeDeiceLib=symlink://../../HomeDeviceLib\n",
    "board = " + target_board.split(" ")[0]
]
file.writelines(lines)
file.close()

result = subprocess.run("pio run -d " + script_directory + "/../Devices/" + target_type, shell=True, capture_output=False, text=True)
print("Done")

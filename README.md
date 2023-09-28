# SmartHomeDevices

SmartHomeDevices is a project implementing ESP32 smart devices for [JavaHomeServer](https://github.com/jayadamsmorgan/JavaHomeServer) using Arduino framework.

## Installation

### PlatformIO Core CLI

1. Install [PlatformIO Core CLI](https://docs.platformio.org/en/stable/core/index.html)
2. Choose the Smart Device you want, you can see list of devices and their custom features down below. Choose the one that fits you the most.
3. Open the chosen '.cpp' file with your favorite editor.
4. Modify predefined configuration values, such as `SSID` and `PASS` for your home network, GPIO pins, etc. You can also modify OTA settings, delays and other predefined values if you need to.
5. Open the platformio.ini file in the selected smart device folder and change the board ID to the one you are using.
   - You can use `pio device list` to check the list of the connected devices
   - Check the full list of boards with `pio boards`
6. Open terminal and navigate to the folder with chosen smart device.
7. Connect your ESP32 board and run the command:

   ```zsh
   pio run -t upload
   ```

## OTA updating

Once you flash ESP32 with this software you will be able to reflash it via Wi-Fi:

    python3 tools/espota.py -i esp-ip-or-hostname.local -a your_fancy_password -f path/to/your/firmware.bin -r

Or if you have custom port:

    python3 tools/espota.py -i esp-ip-or-hostname.local -p port -a your_fancy_password -f path/to/your/firmware.bin -r

## List of available devices

- [HomeBasicDevice](Devices/HomeBasicDevice)
- [HomeLightDevice](Devices/HomeLightDevice)
- [HomeRGBLightDevice](Devices/HomeRGBLightDevice)

## TODO

- Add more devices with different features.
- Script to automate installations.
- Add installation instructions for Platformio IDE.
- Support more platforms besides ESP32
- Documentation

# SmartHomeDevices

SmartHomeDevices is a project implementing ESP32 smart devices for [JavaHomeServer](https://github.com/jayadamsmorgan/JavaHomeServer) using Arduino framework.

## List of implemented devices

- [HomeBasicDevice](Devices/HomeBasicDevice)
- [HomeLightDevice](Devices/HomeLightDevice)
- [HomeRGBLightDevice](Devices/HomeRGBLightDevice)

## Installation

### PlatformIO Core CLI

1. Clone repository and `cd` into it

   ```zsh
       git clone https://github.com/jayadamsmorgan/SmartHomeDevices.git
       cd SmartHomeDevices
   ```

2. Install dependencies using `install_dependencies.sh`:

   ```zsh
       sh tools/install_dependencies.sh
   ```

   This script installs [Homebrew Package Manager](https://brew.sh) and uses it to install other dependencies.
   You can also install dependencies on your own, here is a list of them:

   - [PlatformIO Core CLI](https://docs.platformio.org/en/stable/core/installation/index.html)
   - `python3`
   - `pyyaml`
   - `pyinputplus`

3. Choose the [Device](Devices) you want to implement, and edit it's `device_settings.yaml` configuration file. The `Required` fields is mostly just GPIO and Wi-Fi configuration, while `Optional` values are for advanced users for more precise tuning and extra options.

4. Connect your ESP32 and run `setup.py` script:

   ```zsh
       python3 tools/setup.py
   ```

5. At the prompts select the desired Device implementation you chose at step 3 and the type of your ESP32 board. You can use numbers to select. The script will compile the firmware and flash your ESP32.

6. Voilà!

Note: You can also do it without the script by just specifying your board in `platformio.ini` and running `pio run -t upload`. Adding the setup script felt more convinient.

## OTA updating

Once you flash ESP32 with this software you will be able to reflash it via Wi-Fi. [JavaHomeServer](https://github.com/jayadamsmorgan/JavaHomeServer) uses the default values to update Devices, but if you want to update Devices over-the-air manually you can do so using `espota.py`:

    python3 tools/espota.py -i esp-ip-or-hostname -p your_ota_port -a your_fancy_ota_password -f path/to/your/firmware.bin -r

## Contributing

Feel free to contribute any piece of code.

## TODO

- Add more devices with different features.
- Support more platforms besides ESP32
- Documentation

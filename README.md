# SmartHomeDevices
SmartHomeDevices is a project implementing ESP32 smart devices for [JavaHomeServer](https://github.com/jayadamsmorgan/JavaHomeServer) using Arduino framework.



## Installation

### Arduino IDE
1. Choose the Smart Device you need, you can see their own custom features down below. Choose the one that fits you the most.
2. Open the chosen '.cpp' file in Arduino IDE. If you have trouble opening it, change the file extension to '.ino'.
3. Choose the ESP32 board you have.
4. Modify predefined configuration values, such as SSID and PASS for your home network, GPIO pins, etc. You can also modify OTA settings, delays and other predefined values if you need to.
5. Upload sketch to your ESP32.

### PlatformIO Core CLI
1. Install [PlatformIO Core CLI](https://docs.platformio.org/en/stable/core/index.html)
2. Choose the Smart Device you want, you can see list of devices and their custom features down below. Choose the one that fits you the most.
3. Open the chosen '.cpp' file with your favorite editor.
4. Modify predefined configuration values, such as ```SSID``` and ```PASS``` for your home network, GPIO pins, etc. You can also modify OTA settings, delays and other predefined values if you need to.
5. Open the platformio.ini file in the selected smart device folder and change the board ID to the one you are using.
   * You can use ``` pio device list ``` to check the list of the connected devices
   * Check the full list of boards with ``` pio boards ```
7. Open terminal and navigate to the folder with chosen smart device.
8. Connect your ESP32 board and run the command:

    ```zsh
    pio run -t upload
    ```

## OTA updating
Once you flash ESP32 with this software you will be able to reflash it via Wi-Fi. Refer to [PlatofrmIO ESP32 BasicOTA](https://github.com/JakubAndrysek/BasicOTA-ESP32-library/blob/master/README.md). 

## List of available devices
* [HomeBasicDevice](Devices/HomeBasicDevice)
* [HomeLightDevice](Devices/HomeLightDevice)


## TODO
* Add more devices with different features.
* Script to automate installations.
* Add installation instructions for Platformio IDE.
* Script for OTA updating without Arduino IDE.
* Support more platforms besides ESP32
* Documentation


 

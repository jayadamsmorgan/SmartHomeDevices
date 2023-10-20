#include "HomeDevice.hpp"

#define Device HomeDevice.json["device"]

// GPIO pins setup
void gpio_setup() {
  pinMode(INPUT_GPIO_PIN, INPUT_MODE);
}

void setup() {

  gpio_setup();
  
  #ifdef DEBUG
  HomeDevice.debug = true;
  #endif // DEBUG
  HomeDevice
    .serial_init()
    .eeprom_init()
    .wifi_init(STR(WIFI_SSID), STR(WIFI_PASS))
    .udp_init(UDP_PORT);
    #ifdef ENABLE_OTA
    HomeDevice.ota_init(STR(OTA_PASSWORD), OTA_PORT);
    #endif // ENABLE_OTA
  
  if (HomeDevice.eepromEmpty) {
    Device["analogRead"] = 0;
  }

}

unsigned long previous_action_time = 0;

void loop() {

  while (!HomeDevice.isConnected) { }

  if (HomeDevice.isUpdating) {
    return;
  }
  // Main logic
  if (millis() - previous_action_time >= ACTION_DELAY) {
    HomeDevice.send_current_state_to_server();
    previous_action_time = millis();
  }
  
  Device["analogRead"] = analogRead(INPUT_GPIO_PIN);

  #ifdef ENABLE_OTA
  HomeDevice.ota_handle();
  #endif // ENABLE_OTA
}


#include "HomeDevice.hpp"

// GPIO pins setup
void gpio_setup() {
  pinMode(OUTPUT_GPIO_PIN, OUTPUT);
}

void setup() {

  gpio_setup();

  //HomeDevice.debug = true;

  HomeDevice
    .serial_init()
    .eeprom_init()
    .wifi_init(STR(WIFI_SSID), STR(WIFI_PASS))
    .udp_init(UDP_PORT)
    .ota_init(STR(OTA_PASSWORD), OTA_PORT);

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
  
  digitalWrite(OUTPUT_GPIO_PIN, HomeDevice.isOn ? HIGH : LOW);

  HomeDevice.ota_handle();
}


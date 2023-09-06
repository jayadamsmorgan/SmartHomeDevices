#include "HomeDevice.hpp"

// Change values for your WiFi
#define SSID "*********"
#define PASS "*********"

#define OUTPUT_GPIO_PIN 13

#define UDP_PORT 5051

#define ACTION_DELAY 500

// You can change OTA variables here
#define OTA_PORT 3232
#define OTA_PASSWORD "admin"

// GPIO pins setup
void gpio_setup() {
  pinMode(OUTPUT_GPIO_PIN, OUTPUT);
}

void setup() {

  gpio_setup();

  //HomeDevice.debug = true;

  HomeDevice.serial_init();
  
  HomeDevice.eeprom_init();
  
  HomeDevice.wifi_init(SSID, PASS);

  HomeDevice.udp_init(UDP_PORT);

  HomeDevice.load_previous_state();

  HomeDevice.ota_init(OTA_PASSWORD, OTA_PORT);

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
  
  digitalWrite(OUTPUT_GPIO_PIN, HomeDevice.state.isOn ? HIGH : LOW);

  HomeDevice.ota_handle();

  String red = HomeDevice.get_data_variable("red");
}


#include "HomeDevice.hpp"

void gpio_setup() {
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_PIN, PWM_CHANNEL);
}

void setup() {

  gpio_setup();

  //HomeDevice.debug = true;
  HomeDevice
    .serial_init()
    .eeprom_init()
    .wifi_init(STR(WIFI_SSID), STR(WIFI_PASS))
    .udp_init(UDP_PORT)
    .ota_init(STR(OTA_PASSWORD), OTA_PORT)
    .on_turn_on([]{
      unsigned long previous_action_time;
      uint8_t brightness = HomeDevice.json["brightness"];
      for (int i = 0; i <= brightness; i++) {
        if (millis() - previous_action_time >= SOFT_TURN_DURATION / brightness) {
          ledcWrite(PWM_CHANNEL, i);
          previous_action_time = millis();
        }
      }
    })
    .on_turn_off([]{
      unsigned long previous_action_time;
      uint8_t brightness = HomeDevice.json["brightness"];
      for (int i = brightness; i >= 0; i--) {
        if (millis() - previous_action_time >= SOFT_TURN_DURATION / brightness) {
          ledcWrite(PWM_CHANNEL, i);
          previous_action_time = millis();
        }
      }
    });

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
  
  uint8_t brightness = HomeDevice.json["brightness"];

  ledcWrite(PWM_CHANNEL, HomeDevice.isOn ? brightness : 0);

  ArduinoOTA.handle();
}


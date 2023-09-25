#include "HomeDevice.hpp"

uint8_t brightness;

unsigned long previous_action_time = 0;

bool turning_on = false;
bool turning_off = false;

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
      turning_on = true;
    })
    .on_turn_off([]{
      turning_off = false;
    });

}

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
  
  brightness = HomeDevice.json["brightness"];

  if (turning_on) {
    brightness = 0;
    uint8_t targetBrightness = HomeDevice.json["brightness"];
    while (brightness < targetBrightness) {
      brightness++;
      ledcWrite(PWM_CHANNEL, brightness);
      if (targetBrightness != 0) {
        delay(SOFT_TURN_DURATION / targetBrightness);
      }
    }
    turning_on = false;
  }
  if (turning_off) {
    uint8_t previousBrightness = brightness;
    while (brightness != 0) {
      brightness--;
      ledcWrite(PWM_CHANNEL, brightness);
      if (previousBrightness != 0) {
        delay(SOFT_TURN_DURATION / previousBrightness);
      }
    }
    turning_off = false;
  }

  ledcWrite(PWM_CHANNEL, HomeDevice.isOn ? brightness : 0);

  ArduinoOTA.handle();
}


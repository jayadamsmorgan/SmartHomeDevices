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
  
  #if (DEBUG)
  HomeDevice.debug = true;
  #endif
  HomeDevice
    .serial_init()
    .eeprom_init()
    .wifi_init(STR(WIFI_SSID), STR(WIFI_PASS))
    #if (SOFT_SWITCH)
    .on_turn_on([]{
      turning_on = true;
    })
    .on_turn_off([]{
      turning_off = false;
    })
    #endif
    .udp_init(UDP_PORT)
    .ota_init(STR(OTA_PASSWORD), OTA_PORT);

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
  
  #if SOFT_SWITCH
  uint8_t newBrightness = HomeDevice.json["brightness"];

  if (abs(newBrightness - brightness) >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
    if (newBrightness > brightness) {
      while (brightness != newBrightness) {
        brightness++;
        ledcWrite(PWM_CHANNEL, brightness);
        delay(SOFT_TURN_DURATION / 100);
      }
    } else {
      while (brightness != newBrightness) {
        brightness--;
        ledcWrite(PWM_CHANNEL, brightness);
        delay(SOFT_TURN_DURATION / 100);
      }
    }
  } else {
    brightness = newBrightness;
  }

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
  #else
  brightness = HomeDevice.json["brightness"];
  #endif // SOFT_SWITCH

  ledcWrite(PWM_CHANNEL, HomeDevice.isOn ? brightness : 0);

  ArduinoOTA.handle();
}


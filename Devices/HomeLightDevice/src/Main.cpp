#include "HomeDevice.hpp"

#define Device HomeDevice.json["device"]

uint8_t brightness;

unsigned long previous_action_time = 0;

#ifdef SOFT_SWITCH
bool turning_on = false;
bool turning_off = false;
#endif // SOFT_SWITCH

void gpio_setup() {
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_PIN, PWM_CHANNEL);
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
    #if (SOFT_SWITCH)
    .on_turn_on([]{
      turning_on = true;
    })
    .on_turn_off([]{
      turning_off = false;
    })
    #endif // SOFT_SWITCH
    .udp_init(UDP_PORT);
    #ifdef ENABLE_OTA
    HomeDevice.ota_init(STR(OTA_PASSWORD), OTA_PORT);
    #endif // ENABLE_OTA

  if (HomeDevice.eepromEmpty) {
    Device["brightness"] = 100;
  }

  brightness = Device["brightness"];
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
  
  #ifdef SOFT_SWITCH
  if (HomeDevice.isOn) {
    ledcWrite(PWM_CHANNEL, brightness);
  }
  #endif // SOFT_SWITCH

  if (!HomeDevice.newDataArrived) {
    return;
  }
  HomeDevice.newDataArrived = false;


  #ifdef SOFT_SWITCH
  uint8_t target_brightness = HomeDevice.json["device"]["brightness"];
  if (turning_on) {
    if (target_brightness >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
      while (brightness != target_brightness) {
        brightness++;
        ledcWrite(PWM_CHANNEL, brightness);
        if (target_brightness != 0) {
          delay(SOFT_TURN_DURATION / target_brightness);
        } else {
          delay(SOFT_TURN_DURATION / 255);
        }
      }
    } else {
      brightness = target_brightness;
    }
    turning_on = false;
  } else if (turning_off) {
    if (brightness >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
      while (brightness != 0) {
        brightness--;
        ledcWrite(PWM_CHANNEL, brightness);
        if (target_brightness != 0) {
          delay(SOFT_TURN_DURATION / target_brightness);
        } else {
          delay(SOFT_TURN_DURATION / 255);
        }
      }
    } else {
      brightness = 0;
    }
    turning_off = false;
  } else if (HomeDevice.isOn){
    uint8_t change = abs(brightness - target_brightness);
    if (change >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
      while (brightness != target_brightness) {
        if (brightness > target_brightness) {
          brightness--;
        } else {
          brightness++;
        }
        ledcWrite(PWM_CHANNEL, brightness);
        delay(SOFT_TURN_DURATION / change);
      }
    } else {
      brightness = target_brightness;
    }
  }

  #else // SOFT_SWITCH
  brightness = HomeDevice.json["brightness"];
  if (HomeDevice.isOn) {
    ledcWrite(PWM_CHANNEL, brightness);
  } else {
    ledcWrite(PWM_CHANNEL, 0);
  }
  #endif // SOFT_SWITCH

  #ifdef ENABLE_OTA
  ArduinoOTA.handle();
  #endif // ENABLE_OTA
}


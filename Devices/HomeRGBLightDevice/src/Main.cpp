#include "HomeDevice.hpp"

#ifdef USE_ADAFRUIT_NEOPIXEL
#include "Adafruit_NeoPixel.h"
#ifndef ADAFRUIT_LED_AMOUNT
#define ADAFRUIT_LED_AMOUNT 1
#endif // ADAFRUIT_LED_AMOUNT
Adafruit_NeoPixel rgb(ADAFRUIT_LED_AMOUNT, OUTPUT_ADAFRUIT_GPIO_PIN, ADA_RGB_GRB + ADA_RGB_FREQ);
#endif // USE_ADAFRUIT_NEOPIXEL

#define Device HomeDevice.json["device"]

uint8_t brightness;

unsigned long previous_action_time = 0;
RGBColor color;

#ifdef SOFT_SWITCH
bool turning_on = false;
bool turning_off = false;
#endif // SOFT_SWITCH

void gpio_setup() {
  #ifdef USE_ADAFRUIT_NEOPIXEL
  rgb.begin();
  #else // USE_ADAFRUIT_NEOPIXEL
  pinMode(OUTPUT_GPIO_RGB_LED_RED, OUTPUT);
  pinMode(OUTPUT_GPIO_RGB_LED_GREEN, OUTPUT);
  pinMode(OUTPUT_GPIO_RGB_LED_BLUE, OUTPUT);
  ledcSetup(PWM_CHANNEL_RED, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_RED, PWM_CHANNEL_RED);
  ledcSetup(PWM_CHANNEL_GREEN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_GREEN, PWM_CHANNEL_GREEN);
  ledcSetup(PWM_CHANNEL_BLUE, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_BLUE, PWM_CHANNEL_BLUE);
  #endif // USE_ADAFRUIT_NEOPIXEL
}

void rgb_control(uint8_t red, uint8_t green, uint8_t blue) {
  #ifdef USE_ADAFRUIT_NEOPIXEL
  rgb.setBrightness((brightness / 100.0) * 255.0);
  rgb.setPixelColor(0, rgb.Color(red, green, blue));
  rgb.show();
  #else // USE_ADAFRUIT_NEOPIXEL
  ledcWrite(PWM_CHANNEL_RED, red * brightness / 100.0);
  ledcWrite(PWM_CHANNEL_GREEN, green * brightness / 100.0);
  ledcWrite(PWM_CHANNEL_BLUE, blue * brightness / 100.0);
  #endif // USE_ADAFRUIT_NEOPIXEL
}

void rgb_control() {
  rgb_control(color.red, color.green, color.blue);
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
    .udp_init(UDP_PORT)
    #ifdef SOFT_SWITCH
    .on_turn_off([](){
      turning_off = true;
    })
    .on_turn_on([](){
      turning_on = true;
    });
    #endif // SOFT_SWITCH
    #ifdef ENABLE_OTA
    HomeDevice.ota_init(STR(OTA_PASSWORD), OTA_PORT);
    #endif // ENABLE_OTA

  if (HomeDevice.eepromEmpty) {
    Device["brightness"] = 100;
    Device["red"] = 100;
    Device["green"] = 0;
    Device["blue"] = 255;
  }

  brightness = Device["brightness"];
  color.red = Device["red"];
  color.green = Device["green"];
  color.blue = Device["blue"];
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
    rgb_control();
  }
  #endif // SOFT_SWITCH

  if (!HomeDevice.newDataArrived) {
    return;
  }
  HomeDevice.newDataArrived = false;


  #ifdef SOFT_SWITCH
  uint8_t target_brightness = Device["brightness"];
  if (turning_on) {
    if (target_brightness >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
      while (brightness != target_brightness) {
        brightness++;
        rgb_control();
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
        rgb_control();
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
        rgb_control();
        delay(SOFT_TURN_DURATION / change);
      }
    } else {
      brightness = target_brightness;
    }
  }

  float red = Device["red"];
  float green = Device["green"];
  float blue = Device["blue"];

  if (HomeDevice.isOn) {
    float redChange = red - color.red;
    float greenChange = green - color.green;
    float blueChange = blue - color.blue;
  if (std::abs(redChange) >= COLOR_SOFT_CHANGE_THRESHOLD ||
        std::abs(greenChange) >= COLOR_SOFT_CHANGE_THRESHOLD ||
        std::abs(blueChange) >= COLOR_SOFT_CHANGE_THRESHOLD) {
      float maxChange = std::max({std::abs(redChange), std::abs(greenChange), std::abs(blueChange)});
      float redTemp = color.red;
      float greenTemp = color.green;
      float blueTemp = color.blue;
      for (uint8_t progress = 1; progress < maxChange; ++progress) {
        redTemp += (float) (redChange / maxChange);
        greenTemp += (float) (greenChange / maxChange);
        blueTemp += (float) (blueChange / maxChange);
        rgb_control((uint8_t) redTemp, (uint8_t) greenTemp, (uint8_t) blueTemp);
        delay((int) (SOFT_TURN_DURATION / maxChange));
      }
    }
  }
  color.red = (uint8_t) red;
  color.green = (uint8_t) green;
  color.blue = (uint8_t) blue;

  #else // SOFT_SWITCH
  brightness = Device["brightness"];
  color.red = Device["red"];
  color.green = Device["green"];
  color.blue = Device["blue"];
  if (HomeDevice.isOn) {
    rgb_control();
  } else {
    brightness = 0;
    rgb_control();
  }
  #endif // SOFT_SWITCH

  #ifdef ENABLE_OTA
  ArduinoOTA.handle();
  #endif // ENABLE_OTA
}


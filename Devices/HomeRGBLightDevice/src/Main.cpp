#include "HomeDevice.hpp"

#if (USE_ADAFRUIT_NEOPIXEL)
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel rgb(1, ADAFRUIT_OUTPUT_GPIO_PIN, ADA_RGB_GRB + ADA_RGB_FREQ);
#endif

uint8_t brightness;

unsigned long previous_action_time = 0;
RGBColor color;

#if (SOFT_SWITCH)
bool turning_on = false;
bool turning_off = false;
#endif

void gpio_setup() {
  #if (USE_ADAFRUIT_NEOPIXEL)
  rgb.begin();
  #else
  pinMode(OUTPUT_GPIO_RGB_LED_RED, OUTPUT);
  pinMode(OUTPUT_GPIO_RGB_LED_GREEN, OUTPUT);
  pinMode(OUTPUT_GPIO_RGB_LED_BLUE, OUTPUT);
  ledcSetup(PWM_CHANNEL_RED, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_RED, PWM_CHANNEL_RED);
  ledcSetup(PWM_CHANNEL_GREEN, PWM_FREQ, PWM_CHANNEL_GREEN);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_GREEN, PWM_CHANNEL_GREEN);
  ledcSetup(PWM_CHANNEL_BLUE, PWM_FREQ, PWM_CHANNEL_BLUE);
  ledcAttachPin(OUTPUT_GPIO_RGB_LED_BLUE, PWM_CHANNEL_BLUE);
  #endif
}

void rgb_control() {
  #if (USE_ADAFRUIT_NEOPIXEL)
  rgb.setBrightness((brightness / 100.0) * 255.0);
  rgb.setPixelColor(0, rgb.Color(color.red, color.green, color.blue));
  rgb.show();
  #else
  ledcWrite(PWM_CHANNEL_RED, color.red * brightness / 100.0);
  ledcWrite(PWM_CHANNEL_GREEN, color.green * brightness / 100.0);
  ledcWrite(PWM_CHANNEL_BLUE, color.blue * brightness / 100.0);
  #endif
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
    .udp_init(UDP_PORT)
    #if (SOFT_SWITCH)
    .on_turn_off([](){
      turning_off = true;
    })
    .on_turn_on([](){
      turning_on = true;
    })
    #endif // SOFT_SWITCH
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
  uint8_t newBrightness = HomeDevice.json["brightness"];
  uint8_t red = HomeDevice.json["red"];
  uint8_t green = HomeDevice.json["green"];
  uint8_t blue = HomeDevice.json["blue"];
  
  #if SOFT_SWITCH
  if (abs(newBrightness - brightness) >= BRIGHTNESS_SOFT_CHANGE_THRESHOLD) {
    if (newBrightness > brightness) {
      while (brightness != newBrightness) {
        brightness++;
        rgb_control();
        delay(SOFT_TURN_DURATION / 100);
      }
    } else {
      while (brightness != newBrightness) {
        brightness--;
        rgb_control();
        delay(SOFT_TURN_DURATION / 100);
      }
    }
  } else {
    brightness = newBrightness;
  }

  if (abs(red - color.red) >= COLOR_SOFT_CHANGE_THRESHOLD ||
      abs(green - color.green) >= COLOR_SOFT_CHANGE_THRESHOLD ||
      abs(blue - color.blue) >= COLOR_SOFT_CHANGE_THRESHOLD) {
    int8_t redChange = red - color.red;
    int8_t greenChange = green - color.green;
    int8_t blueChange = blue - color.blue;
    int8_t maxChange = max(abs(redChange), abs(greenChange), abs(blueChange));
    float redStep = (float) redChange / maxChange;
    float greenStep = (float) greenChange / maxChange;
    float blueStep = (float) blueChange / maxChange;
    while (red != color.red && green != color.green && blue != color.blue) {
      if (red != color.red) {
        red += redStep;
      }
      if (green != color.green) {
        green += greenStep;
      }
      if (blue != color.blue) {
        blue += blueStep;
      }
      rgb_control();
      if (maxChange != 0) {
        delay(SOFT_TURN_DURATION / maxChange);
      }
    }
  } else {
    color.red = red;
    color.green = green;
    color.blue = blue;
  }

  if (turning_on) {
    brightness = 0;
    uint8_t targetBrightness = HomeDevice.json["brightness"];
    while (brightness < targetBrightness) {
      brightness++;
      rgb_control();
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
      rgb_control();
      if (previousBrightness != 0) {
        delay(SOFT_TURN_DURATION / previousBrightness);
      }
    }
    turning_off = false;
  }
  #endif // SOFT_SWITCH

  if (HomeDevice.isOn) {
    rgb_control();
  } else {
    brightness = 0;
    rgb_control();
  }

  ArduinoOTA.handle();
}


#include "HomeDevice.hpp"

#if (USE_ADAFRUIT_NEOPIXEL)
#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel rgb(1, OUTPUT_GPIO_PIN, ADA_RGB_GRB + ADA_RGB_FREQ);
#endif

uint8_t brightness;

unsigned long previous_action_time = 0;
RGBColor color;

bool turning_on = false;
bool turning_off = false;

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

  //HomeDevice.debug = true;
  HomeDevice
    .serial_init()
    .eeprom_init()
    .wifi_init(STR(WIFI_SSID), STR(WIFI_PASS))
    .udp_init(UDP_PORT)
    .on_turn_off([](){
      turning_off = true;
    })
    .on_turn_on([](){
      turning_on = true;
    })
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
  brightness = HomeDevice.json["brightness"];
  color.red = HomeDevice.json["red"];
  color.green = HomeDevice.json["green"];
  color.blue = HomeDevice.json["blue"];
  
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

  if (HomeDevice.isOn) {
    rgb_control();
  } else {
    brightness = 0;
    rgb_control();
  }

  ArduinoOTA.handle();
}


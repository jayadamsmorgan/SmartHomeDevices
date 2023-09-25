#ifndef __HomeDevice_lib
#define __HomeDevice_lib

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "sys/_stdint.h"
#include <functional>

#define ST(x) #x
#define STR(x) ST(x)

#define OTA_PORT_DEFAULT 3232
#define SERIAL_BAUD_DEFAULT 115200

struct RGBColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct RGBWColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};

class HomeDeviceClass
{
  public:

    bool debug = false;
    
    StaticJsonDocument<1024> json; 

    bool isOn;
    int id;

    bool isUpdating;
    bool isConnected;

    HomeDeviceClass();
    ~HomeDeviceClass();

    HomeDeviceClass& on_turn_on(std::function<void()> fn); // Perform function on turning on
    HomeDeviceClass& on_turn_off(std::function<void()> fn); // Perform function on turning off

    HomeDeviceClass& eeprom_init();
   
    HomeDeviceClass& serial_init(int baud = SERIAL_BAUD_DEFAULT);
    void log(String msg);

    HomeDeviceClass& udp_init(int udp_port);

    void parse_udp_packet(AsyncUDPPacket packet);
    void send_current_state_to_server();

    HomeDeviceClass& wifi_init(const char* ssid, const char* pass);
    HomeDeviceClass& ota_init(const char* password = "", int port = OTA_PORT_DEFAULT);
    void ota_handle();
  private:
    const char* ssid;
    const char* pass;

    std::function<void()> on_turn_on_function;
    std::function<void()> on_turn_off_function;

    void write_to_eeprom(String jsonString);

    AsyncUDP udp;

    void wifi_event_handler(WiFiEvent_t event);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HOMEDEVICE_LIB)
extern HomeDeviceClass HomeDevice;
#endif

#endif


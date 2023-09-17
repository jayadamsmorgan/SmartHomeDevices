#ifndef __HomeDevice_lib
#define __HomeDevice_lib

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

#define OTA_PORT_DEFAULT 3232
#define SERIAL_BAUD_DEFAULT 115200

#define CUSTOM_PROP_ARR_SIZE 10

class HomeDeviceClass
{
  public:

    bool debug = false;
    
    StaticJsonDocument<1024> json; 

    bool isOn;
    int id;

    bool isUpdating;
    bool isConnected;
    bool isTurningOn;
    bool isTurningOff;

    HomeDeviceClass();
    ~HomeDeviceClass();

    void eeprom_init();
   
    void serial_init(int baud = SERIAL_BAUD_DEFAULT);
    void log(String msg);

    void udp_init(int udp_port);

    void parse_udp_packet(AsyncUDPPacket packet);
    void send_current_state_to_server();

    void wifi_init(const char* ssid, const char* pass);
    void ota_init(const char* password = "", int port = OTA_PORT_DEFAULT);
    void ota_handle();
  private:
    const char* custom_properties[CUSTOM_PROP_ARR_SIZE][2];
    const char* ssid;
    const char* pass;

    void write_to_eeprom(String jsonString);

    AsyncUDP udp;

    void wifi_event_handler(WiFiEvent_t event);
    const char* deviceType;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HOMEDEVICE_LIB)
extern HomeDeviceClass HomeDevice;
#endif

#endif


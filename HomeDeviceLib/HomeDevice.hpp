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

class HomeDeviceClass
{
  public:
    struct State {
      int id;
      bool isOn;
      String data;
    };

    bool debug = false;
    
    StaticJsonDocument<1024> json; 

    State state;

    bool isUpdating;
    bool isConnected;

    HomeDeviceClass();
    ~HomeDeviceClass();
   
    void serial_init(int baud = SERIAL_BAUD_DEFAULT);
    void log(String msg);

    void eeprom_init();

    void udp_init(int udp_port);

    void load_previous_state();
    void parse_udp_packet(AsyncUDPPacket packet);
    void send_current_state_to_server();

    String get_data_variable(String var);

    void wifi_init(String ssid, String pass);
    void ota_init(String password = "", int port = OTA_PORT_DEFAULT);
    void ota_handle();
  private:
    String ssid;
    String pass;

    AsyncUDP udp;
    EEPROMClass eepromClass;

    void update_EEPROM_variables(int id, bool on, String data);
    void wifi_event_handler(WiFiEvent_t event);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HOMEDEVICE_LIB)
extern HomeDeviceClass HomeDevice;
#endif

#endif


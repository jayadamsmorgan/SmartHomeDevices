#include "HomeDevice.hpp"

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "EEPROM.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "ArduinoJson.h"

//#define DEBUG_SERIAL_INIT_DELAY 5000

#define INCOMING_WELCOME_BYTE 0x51
#define OUTCOMING_WELCOME_BYTE 0x50

HomeDeviceClass::HomeDeviceClass() {
  isUpdating = false;
  isConnected = false;
  newDataArrived = false;
}

HomeDeviceClass::~HomeDeviceClass() {

}

//  Serial initialization for debugging purposes
HomeDeviceClass& HomeDeviceClass::serial_init(int baud) {
  #ifdef DEBUG_SERIAL_INIT_DELAY
  vTaskDelay(DEBUG_SERIAL_INIT_DELAY / portTICK_PERIOD_MS);
  #endif
  Serial.begin(115200);
  while (!Serial) { }
  log("Serial initialized.");
  return HomeDevice;
}

void HomeDeviceClass::log(String msg) {
  if (debug) {
    Serial.println(msg);
  }
}

HomeDeviceClass& HomeDeviceClass::eeprom_init() {
  if(!EEPROM.begin(1024)) {
    log("Cannot initialize EEPROM... Restarting...");
    ESP.restart();
  }
  String jsonString = EEPROM.readString(0);
  log(jsonString);
  DeserializationError error = deserializeJson(json, jsonString);
  if (error) {
    log("No previous state was found. Using default state.");
    json["deviceType"] = STR(DEVICE_TYPE);
    json["device"]["id"] = "NO_ID";
    json["device"]["on"] = true;
    json["device"]["name"] = "NO_NAME";
    json["device"]["location"] = "NO_LOCATION";
    json["device"]["data"] = "";
    return HomeDevice;
  }
  isOn = json["device"]["on"];
  return HomeDevice;
}

void HomeDeviceClass::write_to_eeprom(String jsonString) {
  EEPROM.writeString(0, jsonString);
  EEPROM.commit();
}

void HomeDeviceClass::wifi_event_handler(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      log("WiFi connected. IP address: "
          + WiFi.localIP().toString());
      isConnected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      log("WiFi lost connection.");
      isConnected = false;
      wifi_init(ssid, pass);
      break;
    default: break;
  }
}

HomeDeviceClass& HomeDeviceClass::wifi_init(const char* ssid, const char* pass) {
  log("Connecting to WiFi...");
  HomeDeviceClass::ssid = ssid;
  HomeDeviceClass::pass = pass;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    wifi_event_handler(event);
  });
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  #ifdef WIFI_DISABLE_SLEEP
  WiFi.setSleep(false);
  #else
  WiFi.setSleep(true);
  #endif
  return HomeDevice;
}

// WiFi UDP initialization
HomeDeviceClass& HomeDeviceClass::udp_init(int udp_port) {
  if(udp.listen(udp_port)) {
    udp.onPacket([this](AsyncUDPPacket packet) {
      parse_udp_packet(packet);
    });
  }
  log("UDP Client initialized");
  return HomeDevice;
}

// Send state variables back to server
void HomeDeviceClass::send_current_state_to_server() {
  char welcome_char = OUTCOMING_WELCOME_BYTE;
  String dataStr;
  serializeJson(json, dataStr);
  dataStr = String(welcome_char) + dataStr;
  udp.broadcast(dataStr.c_str());
}

HomeDeviceClass& HomeDeviceClass::on_turn_on(std::function<void()> fn) {
  on_turn_on_function = fn;
  return HomeDevice;
}

HomeDeviceClass& HomeDeviceClass::on_turn_off(std::function<void()> fn) {
  on_turn_off_function = fn;
  return HomeDevice;
}

// Parse incoming UDP packet
void HomeDeviceClass::parse_udp_packet(AsyncUDPPacket packet) {
  String packet_string = packet.readString();
  const char *payload_data = packet_string.c_str();
  if (payload_data[0] != INCOMING_WELCOME_BYTE) {
    log("Skipping incoming packet: wrong WELCOME_BYTE.");
    return;
  }
  packet_string = packet_string.substring(1);

  DeserializationError error = deserializeJson(json, packet_string);

  // Test if parsing succeeds.
  if (error) {
    log("Deserialization failed.");
    return;
  }
  bool on = json["device"]["on"];

  if (on != isOn) {
    if (on) {
      if (on_turn_on_function) {
        on_turn_on_function();
      }
    } else {
      if (on_turn_off_function) {
        on_turn_off_function();
      }
    }
    isOn = on;
  } 

  write_to_eeprom(packet_string);

  log("Successfully parsed UDP Packet from " +
             packet.remoteIP().toString());

  send_current_state_to_server();
  newDataArrived = true;
}

HomeDeviceClass& HomeDeviceClass::ota_init(const char* password, int port) {
  if (strcmp(password, "") == 0) {
    ArduinoOTA.setPassword(password);
  }
  if (port != 0) {
    ArduinoOTA.setPort(port);
  }
  ArduinoOTA
    .onStart([this]() {
      isUpdating = true;
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      log("Start updating " + type);
    })
    .onEnd([this]() {
      log("OTA Update Success. Rebooting..."); 
    })
    .onProgress([this](unsigned int progress, unsigned int total) {
      String percentage = String(progress / (total / 100));
      log("Progress: " + percentage + "%");
    })
    .onError([this](ota_error_t error) {
      isUpdating = false;
      if (error == OTA_AUTH_ERROR) log("OTA Update Error: Auth Failed.");
      else if (error == OTA_BEGIN_ERROR) log("OTA Update Error: Begin Failed.");
      else if (error == OTA_CONNECT_ERROR) log("OTA Update Error: Connect Failed.");
      else if (error == OTA_RECEIVE_ERROR) log("OTA Update Error: Receive Failed.");
      else if (error == OTA_END_ERROR) log("OTA Update Error: End Failed.");
    });
  ArduinoOTA.begin();
  log("OTA Update Service started at port " + String(port)
      + " with hostname " + ArduinoOTA.getHostname() + ".");
  return HomeDevice;
}

void HomeDeviceClass::ota_handle() {
  ArduinoOTA.handle();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HOMEDEVICE_LIB)
HomeDeviceClass HomeDevice;
#endif


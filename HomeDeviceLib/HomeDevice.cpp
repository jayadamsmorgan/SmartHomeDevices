#include "HomeDevice.hpp"

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "ArduinoJson.h"

//#define DEBUG_SERIAL_INIT_DELAY 5000

#define INCOMING_WELCOME_BYTE 0x51
#define OUTCOMING_WELCOME_BYTE 0x50

HomeDeviceClass::HomeDeviceClass() {
  #if !defined(DEVICE_TYPE)
    #error DEVICE_TYPE was not declared. Use '-DDEVICE_TYPE="\"Your_Device_Type\""'
  #else
    deviceType = DEVICE_TYPE;
  #endif // DEVICE_TYPE
  isUpdating = false;
  isConnected = false;
  isTurningOn = false;
  isTurningOff = false;
}

HomeDeviceClass::~HomeDeviceClass() {

}

//  Serial initialization for debugging purposes
void HomeDeviceClass::serial_init(int baud) {
  #ifdef DEBUG_SERIAL_INIT_DELAY
  vTaskDelay(DEBUG_SERIAL_INIT_DELAY / portTICK_PERIOD_MS);
  #endif
  Serial.begin(115200);
  while (!Serial) { }
  log("Serial initialized.");
}

void HomeDeviceClass::log(String msg) {
  if (debug) {
    Serial.println(msg);
  }
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

void HomeDeviceClass::wifi_init(const char* ssid, const char* pass) {
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
}

// WiFi UDP initialization
void HomeDeviceClass::udp_init(int udp_port) {
  if(udp.listen(udp_port)) {
    udp.onPacket([this](AsyncUDPPacket packet) {
      parse_udp_packet(packet);
    });
  }
  log("UDP Client initialized");
}

// Send state variables back to server
void HomeDeviceClass::send_current_state_to_server() {
  char welcome_char = OUTCOMING_WELCOME_BYTE;
  String dataStr;
  serializeJson(json, dataStr);
  dataStr = String(welcome_char) + DEVICE_TYPE + ";" + dataStr;
  udp.broadcast(dataStr.c_str());
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
  id = json["id"];
  bool on = json["on"];

  if (on != isOn) {
    if (on) {
      isTurningOn = true;
      isTurningOff = false;
    } else {
      isTurningOff = true;
      isTurningOn = false;
    }
    isOn = on;
  } 

  log("Successfully parsed UDP Packet from " +
             packet.remoteIP().toString());

}

void HomeDeviceClass::ota_init(const char* password, int port) {
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
}

void HomeDeviceClass::ota_handle() {
  ArduinoOTA.handle();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HOMEDEVICE_LIB)
HomeDeviceClass HomeDevice;
#endif


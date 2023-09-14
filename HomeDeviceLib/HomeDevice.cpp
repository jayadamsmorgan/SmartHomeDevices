#include "HomeDevice.hpp"

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "HardwareSerial.h"
#include "WiFi.h"
#include "AsyncUDP.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

//#define DEBUG_SERIAL_INIT_DELAY 5000

#define INCOMING_WELCOME_BYTE 0x51
#define OUTCOMING_WELCOME_BYTE 0x50

HomeDeviceClass::HomeDeviceClass() {
  isUpdating = false;
  isConnected = false;
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

void HomeDeviceClass::eeprom_init() {
  eepromClass = EEPROMClass("eeprom0");
  log("Initializing EEPROM...");
  if(!eepromClass.begin(512)) {
    log("Failed to initialize EEPROM. Restarting...");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP.restart();
  }
  log("EEPROM successfully initialized.");
}

void HomeDeviceClass::wifi_event_handler(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      log("WiFi connected. IP address: " + WiFi.localIP().toString());
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

void HomeDeviceClass::wifi_init(String ssid, String pass) {
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
  String state_string = "";
  state_string = state_string + welcome_char + "BasicDevice;{";
  state_string = state_string + "\"id\":" + state.id;
  state_string = state_string + ",\"on\":" + state.isOn;
  state_string = state_string + ",\"data\":\"";
  state_string = state_string + "\"}";
  udp.broadcast(state_string.c_str());
}


// Load previously saved Device State
void HomeDeviceClass::load_previous_state() {
  eepromClass.get(0, state);
}


String HomeDeviceClass::get_data_variable(String var) {
  int startIndex = state.data.indexOf(";" + var + "=");
  if (startIndex == -1) {
    log("Cannot parse data: '" + var + "': no '" + var + "' param.");
    return "";
  }
  startIndex = startIndex + 2 + var.length();
  int valueEndIndex = state.data.indexOf(";", startIndex);
  if (valueEndIndex == -1) {
    log("Cannot parse data: '" + var + "': Packet syntax is wrong.");
    return "";
  }
  String stringValue = state.data.substring(startIndex, valueEndIndex);
  if (stringValue.equals("")) {
    log("Cannot parse data: invalid '" + var + "' value.");
  }
  return stringValue;
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
  int id = json["id"];
  bool isOn = json["on"];
  String data = json["data"].as<String>();

  log("Successfully parsed UDP Packet from " + packet.remoteIP().toString() + ".");

  update_EEPROM_variables(id, isOn, data);
}

// Write new values to eeprom
void HomeDeviceClass::update_EEPROM_variables(int id, bool on, String data) {
  bool valuesChanged = false;
  if (id != state.id) {
    state.id = id;
    valuesChanged = true;
  }
  if (on != state.isOn) {
    state.isOn = on;
    valuesChanged = true;
  }
  if (!data.equals(state.data)) {
    state.data = data;
    valuesChanged = true;
  }

  if (valuesChanged) { 
    eepromClass.put(0, state);
  }
}


void HomeDeviceClass::ota_init(String password, int port) {
  if (!password.equals("")) {
    ArduinoOTA.setPassword(password.c_str());
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
      String logString = "Progress: " + percentage + "%";
      log(logString);
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


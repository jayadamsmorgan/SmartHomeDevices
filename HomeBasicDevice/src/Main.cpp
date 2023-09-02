#include "USBCDC.h"
#include "WiFi.h"
#include "WiFiType.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "freertos/portmacro.h"
#include "EEPROM.h"
#include "AsyncUDP.h"
#include "ArduinoOTA.h"
#include "ESPmDNS.h"

// Uncomment DEBUG to have debug output to Serial
// #define DEBUG

// Change values for your WiFi
#define SSID "*********"
#define PASS "*********"

#define OUTPUT_GPIO_PIN 13

#define UDP_PORT 5051

#define OUTCOMING_WELCOME_BYTE 0x51
#define INCOMING_WELCOME_BYTE 0x52

#define ACTION_DELAY 500

#define FLASH_STORAGE_NAME "eeprom0"

// You can change OTA variables here
#define OTA_HOSTNAME "BasicDevice"
#define OTA_PORT 3232
#define OTA_PASSWORD "admin"

EEPROMClass eepromClass(FLASH_STORAGE_NAME);

AsyncUDP udp;

bool connected = false;

bool isUpdating = false;

struct State {
  int id;
  bool isOn;
};

State state;

// GPIO pins setup
void gpio_setup() {
  pinMode(OUTPUT_GPIO_PIN, OUTPUT);
}

// Logging messages to Serial for debugging purposes
void log(String msg) {
  #ifdef DEBUG
  Serial.println(msg);
  #endif // DEBUG
}

//  Serial initialization for debugging purposes
void serial_init() {
  #ifdef DEBUG
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  Serial.begin(115200);
  while (!Serial) { }
  log("Serial initialized.");
  #endif // DEBUG
}

// WiFi event handler
void wifi_event_handler(WiFiEvent_t event){
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          log("WiFi connected. IP address: " + WiFi.localIP().toString());
          connected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          log("WiFi lost connection.");
          connected = false;
          break;
      default: break;
    }
}

// WiFi initialization
void wifi_init() {
  log("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.onEvent(wifi_event_handler);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// EEPROM initialization
void eeprom_init() {
  log("Initializing EEPROM...");
  if(!eepromClass.begin(0x500)) {
    log("Failed to initialize EEPROM. Restarting...");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP.restart();
  }
  log("EEPROM successfully initialized.");
}

// Write new values to eeprom
void update_EEPROM_variables(int id, bool on) {
  if (id != state.id && on != state.isOn) {
    state.id = id;
    state.isOn = on;
    eepromClass.put(0, state);
    return;
  }
  if (id != state.id) {
    state.id = id;
    eepromClass.put(0, state);
    return;
  }
  if (on != state.isOn) {
    state.isOn = on;
    eepromClass.put(0, state);
    return;
  }
}

// Incoming packet payload string parsing
String parse_string_from_payload(String packet_string, String value) {
  int startIndex = packet_string.indexOf(";" + value + "=");
  if (startIndex == -1) {
    log("Cannot parse UDP Packet: '" + value + "': no '" + value + "' param.");
    return "";
  }
  startIndex = startIndex + 2 + value.length();
  int valueEndIndex = packet_string.indexOf(";", startIndex);
  if (valueEndIndex == -1) {
    log("Cannot parse UDP Packet: '" + value + "': Packet syntax is wrong.");
    return "";
  }
  String stringValue = packet_string.substring(startIndex, valueEndIndex);
  if (stringValue.equals("")) {
    log("Cannot parse UDP Packet: invalid '" + value + "' value.");
  }
  return stringValue;
}

// Parse incoming UDP packet
void parse_udp_packet(AsyncUDPPacket packet) {
  String packet_string = packet.readString();
  const char *payload_data = packet_string.c_str();
  if (payload_data[0] != INCOMING_WELCOME_BYTE) {
    log("Skipping incoming packet: wrong WELCOME_BYTE.");
    return;
  }
  String idString = parse_string_from_payload(packet_string, "id");
  if (idString.equals("")) {
    return;
  }
  int id = idString.toInt();
  if (id == 0) {
    log("Cannot parse UDP Packet: 'id' is not an Integer value.");
    return;
  }
  String onString = parse_string_from_payload(packet_string, "on");
  if (onString.equals("")) {
    return;
  }
  if (!onString.equalsIgnoreCase("true") && !onString.equalsIgnoreCase("false")) {
    log("Cannot parse UDP Packet: 'on' is not a Boolean value.");
    return;
  }
  bool on = onString.equalsIgnoreCase("true");
  log("Successfully parsed UDP Packet from " + packet.remoteIP().toString() + ".");
  update_EEPROM_variables(id, on);
}

// WiFi UDP initialization
void udp_init() {
  if(udp.listen(UDP_PORT)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      parse_udp_packet(packet);
    });
  }
  log("UDP Client initialized");
}

// Load previously saved Device State
void load_previous_state() {
  eepromClass.get(0, state);
} 

// Send state variables back to server
void send_current_state_to_server() {
  char welcome_char = OUTCOMING_WELCOME_BYTE;
  String state_string = "";
  state_string = state_string + welcome_char + "BasicDevice";
  state_string = state_string + ";id=" + state.id;
  state_string = state_string + ";on=" + state.isOn;
  state_string = state_string + ";ipAddress=" + WiFi.localIP().toString();
  udp.broadcast(state_string.c_str());
}

// Initialize Over-The-Air update
void ota_update_init() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA
    .onStart([]() {
      isUpdating = true;
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      log("Start updating " + type);
    })
    .onEnd([]() {
      log("OTA Update Success.");
      isUpdating = false;
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      String percentage = String(progress / (total / 100));
      String logString = "Progress: " + percentage + "%";
      log(logString);
    })
    .onError([](ota_error_t error) {
      isUpdating = false;
      if (error == OTA_AUTH_ERROR) log("OTA Update Error: Auth Failed.");
      else if (error == OTA_BEGIN_ERROR) log("OTA Update Error: Begin Failed.");
      else if (error == OTA_CONNECT_ERROR) log("OTA Update Error: Connect Failed.");
      else if (error == OTA_RECEIVE_ERROR) log("OTA Update Error: Receive Failed.");
      else if (error == OTA_END_ERROR) log("OTA Update Error: End Failed.");
    });
  ArduinoOTA.begin();
  log("OTA Update Service started at port " + String(OTA_PORT)
      + " with hostname " + String(OTA_HOSTNAME) + ".");
}

void setup() {

  gpio_setup();

  serial_init();
  
  eeprom_init();
  
  wifi_init();

  udp_init();

  load_previous_state();

  ota_update_init();

}

unsigned long previous_action_time = 0;

void loop() {

  while (!connected) {
    // Try to reconnect after disconnect
    log("WiFi disconnected. Trying to reconnect...");
    if (millis() - previous_action_time >= ACTION_DELAY) {
      wifi_init();
      previous_action_time = millis();
    }
  }
  if (isUpdating) {
    return;
  }
  // Main logic
  if (millis() - previous_action_time >= ACTION_DELAY) {
    send_current_state_to_server();
    previous_action_time = millis();
  }
  
  digitalWrite(OUTPUT_GPIO_PIN, state.isOn ? HIGH : LOW);

  ArduinoOTA.handle();
}


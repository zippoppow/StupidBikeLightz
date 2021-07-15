#include <Arduino.h>
#include <SPIFFS.h>
#include <esp_now.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>


// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

#define LED_PIN   26
#define BTN_PIN   22
#define HTTP_PORT 8008


// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

// Button debouncing
const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

// WiFi credentials

const char *WIFI_SSID = "WoodGaryBrain2.4";
const char *WIFI_PASS = "R0d30Rodeo1";

//const char *WIFI_SSID = "Tranzmishunz";
//const char *WIFI_PASS = "M00nb4lLZ";

//add correct address
uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


typedef struct test_struct {
  int x;
  int y;
} test_struct;

test_struct test;


// ----------------------------------------------------------------------------
// Definition of the LED component
// ----------------------------------------------------------------------------

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    // methods
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};


// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------

Led onboard_led = {LED_BUILTIN, false};
Led led = {LED_PIN, false};
//Button button = {BTN_PIN, HIGH, 0, 0};

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");


// ----------------------------------------------------------------------------
// SPIFFS initialization
// ----------------------------------------------------------------------------
//The SPI Flash File System or SPIFFS is a light-weight file system for microcontrollers with an SPI flash chip. A flash file system is designed for storing files on flash memory–based storage devices optimized for the particular system.
//SPI stands for Serial Peripheral Interface. Flash memory is a type of non-volatile storage that is electrically eraseable and rewriteable. Flash memory is a kind of non-volatile memory much used for storing programs for simple microprocessors.
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Cannot mount SPIFFS volume...");
    while (1) {
        onboard_led.on = millis() % 200 < 50;
        onboard_led.update();
    }
  }

    //  (SPIFFS.begin(true));
}


// ----------------------------------------------------------------------------
// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

String processor(const String &var) {
    return String(var == "STATE" && led.on ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

void initWebServer() {
    server.on("/", onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
}



// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void notifyClients(String theStatus) {
    ws.textAll(theStatus);
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {

        data[len] = 0;
      //  lastactiveAnim = activeAnim;

        //handle for multiple buttons here
        if (strcmp((char *)data, "off") == 0)
        {
            led.on = false;

            //send message via EspNow to other controller


          //  activeAnim = 0;
            notifyClients("off");
        }

        if (strcmp((char *)data, "on") == 0)
        {
          //activate this controller's led: on
          led.on = true;

          //activate this controller's animation
          //  activeAnim = 1;

          //send message via EspNow to other controller

          //send message back to front-end
            notifyClients("on");
        }
    }
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{

    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.println();
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.println();
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}



// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

JSONVar board;

AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  board["id"] = incomingReadings.id;

  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);

  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
}


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}







void setup(){
   pinMode(onboard_led.pin, OUTPUT);
    pinMode(led.pin, OUTPUT);
 //   pinMode(button.pin, INPUT);

    Serial.begin(115200);
    led.on = true;
    led.update();

    Serial.println("macAddress");
    Serial.println(WiFi.macAddress());

    initSPIFFS();
    Serial.println("SPIFFS");
    initWiFi();
    Serial.println("WiFi");
    initWebSocket();
    Serial.println("WebSockets");
    initWebServer();
    Serial.println("WebServer");

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
   
    // register peer
    esp_now_peer_info_t peerInfo;
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    // register first peer  
    memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
}

void loop() {
  // put your main code here, to run repeatedly:
}
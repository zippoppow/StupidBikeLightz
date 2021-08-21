#include <string>
#include<unistd.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

#include <Arduino.h>

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <esp_now.h>
#include <WiFi.h>


// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------
#define BTN_PIN 15

//macros for SoftwareSerial connection
#define Rx (16)
#define Tx (17)

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

// Button debouncing
const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

SoftwareSerial swSer1;
bool NoData;
char inByte;

// WiFi credentials

const char *WIFI_SSID = "WoodGaryBrain2.4";
const char *WIFI_PASS = "R0d30Rodeo1";

// const char *WIFI_SSID = "Tranzmishunz";
// const char *WIFI_PASS = "M00nb4lLZ";

const int PushButton = 15;

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0x7C, 0x9E, 0xBD, 0x06, 0x66, 0xEC};
// uint8_t broadcastAddress2[] = {0x10, 0x52, 0x1C, 0x5E, 0x4E, 0x58};
//uint8_t broadcastAddresses[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct test_struct {
  int x;
} test_struct;

test_struct test;

// ----------------------------------------------------------------------------
// Definition of the Button component
// ----------------------------------------------------------------------------

struct Button {
    //state variables
    uint8_t  pin;
    bool     lastReading;
    uint32_t lastDebounceTime;
    uint16_t state;

    // methods determining the logical state of the button
    bool pressed()                { return state == 1; }
    bool released()               { return state == 0xffff; }
    bool held(uint16_t count = 0) { return state > 1 + count && state < 0xffff; }

    // method for reading the physical state of the button
    void read() {
        // reads the voltage on the pin connected to the button
        bool reading = digitalRead(pin);

        // if the logic level has changed since the last reading,
        // we reset the timer which counts down the necessary time
        // beyond which we can consider that the bouncing effect
        // has passed.
        if (reading != lastReading) {
            lastDebounceTime = millis();
        }

        // from the moment we're out of the bouncing phase
        // the actual status of the button can be determined
        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            // don't forget that the read pin is pulled-up
            bool pressed = reading == LOW;
            if (pressed) {
                     if (state  < 0xfffe) state++;
                else if (state == 0xfffe) state = 2;
            } else if (state) {
                state = state == 0xffff ? 0 : 0xffff;
            }
        }

        // finally, each new reading is saved
        lastReading = reading;
    }
};

Button button = {BTN_PIN, HIGH, 0, 0};


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
 
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // // This statement will declare pin 15 as digital input 
  // pinMode(button.pin, INPUT);
 
  WiFi.mode(WIFI_STA);
 
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
  // // register second peer  
  // memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  // if (esp_now_add_peer(&peerInfo) != ESP_OK){
  //   Serial.println("Failed to add peer");
  //   return;
  // }
  // /// register third peer
  // memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  // if (esp_now_add_peer(&peerInfo) != ESP_OK){
  //   Serial.println("Failed to add peer");
  //   return;
  // }

  // //SoftwareSerial 
  //   swSer1.begin(115200, SWSERIAL_8N1, Tx, Rx, false, 256);
  //   while (!Serial) {
  //     ; // wait for serial port to connect. Needed for Leonardo only
  //   }

  //HardwareSerial
    Serial2.begin(19200, SERIAL_8N1, 16, 17);

}
 
void loop() {


  // // digitalRead function stores the Push button state 
  // // in variable push_button_state
  // int Push_button_state = digitalRead(PushButton);
  // // if condition checks if push button is pressed
  // // if pressed LED will turn on otherwise remain off 
  // if ( Push_button_state == HIGH )
  // { 
  //   //digitalWrite(LEDPIN, HIGH); 
  //   Serial.println("Push Button On");
  //   Serial.println("call espnow client");

  // }
  // else 
  // {
  //   //digitalWrite(LEDPIN, LOW); 
  //   Serial.println("Push Button Off");
  // }


  // swSer1.listen();
  // //Serial.println("Data from port one:");
  // // while there is data coming in, read it
  // // and send to the hardware serial port:
  // while (swSer1.available() > 0) {
  //   inByte = swSer1.read();
  //   Serial.write(inByte);
  //   NoData = false;

  //   test.x = 2;
  //   esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
      
  //   if (result == ESP_OK) {
  //       Serial.println("Sent with success TTTTTTTTTTT");
  //   }else {
  //       Serial.println("Error sending the data");
  //   }
  // }

  // if(!NoData){
  //   if(inByte == 0){
  //     Serial.println("NoData");
  //     NoData = true;
  //   }
  // }

  while (Serial2.available() > 0) {
    inByte = Serial2.read();
    Serial.write(inByte);
    NoData = false;

    test.x = 2;
    esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
      
    if (result == ESP_OK) {
        Serial.println("Sent with success TTTTTTTTTTT");
    }else {
        Serial.println("Error sending the data");
    }
  }

  if(!NoData){
    if(inByte == 0){
      Serial.println("NoData");
      NoData = true;
    }
  }


  
  delay(5000);

  
}
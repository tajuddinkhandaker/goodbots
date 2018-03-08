/*
 *  This sketch sends data via HTTP GET requests to "portfolio.asdtechltd.com" service.
 *  @reference:
 *  http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/nodemcu-io-basics-pwm/
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

const char* ssid     = "Despicable Me";
const char* password = "iron#Tank?1097";

const char* host = "portfolio.asdtechltd.com";
const uint16_t port = 80;

// (D4) => 2, (D2) => 0
int relayInputs[] = { 2, 0 };
int lightsStates[] = { 1, 0 };// 2 lights: first one ON, second one OFF

// JSON input string.
//
// It's better to use a char[] as shown here.
// If you use a const char* or a String, ArduinoJson will
// have to make a copy of the input in the JsonBuffer.
// @param payload
int updateStates(const String& payload)
{
  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  // const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 20);

  // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.parseObject(payload);

  lightsStates[0] = root["lights"][0]; // 1
  lightsStates[1] = root["lights"][1]; // 0  

  // Test if parsing succeeds.
  if (!root.success())
  {
    USE_SERIAL.println("parseObject() failed");
    return 0;
  }
  return 1;
}

void setup() {
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  
  while (!Serial)
  {
    USE_SERIAL.println("Waiting for serial ... ");
    delay(10);
    continue;
  }
  delay(10);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--)
  {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }

  // We start by connecting to a WiFi network

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.print("Connecting to ");
  USE_SERIAL.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);
  
  // put your setup code here, to run once:
  pinMode(relayInputs[0], OUTPUT); // initialize pin as OUTPUT
  pinMode(relayInputs[1], OUTPUT); // initialize pin as OUTPUT

  changeStates();
}

void changeStates()
{
    if (lightsStates[0] == 1)
    {
        digitalWrite(relayInputs[0], LOW); // turn relay off
    }
    else
    {
        digitalWrite(relayInputs[0], HIGH); // turn relay on
    }
    delay(1000);
    
    if (lightsStates[1] == 1)
    {
        digitalWrite(relayInputs[1], LOW); // turn relay off
    }
    else
    {
        digitalWrite(relayInputs[1], HIGH); // turn relay on
    }
    delay(1000);
}

void loop() {
  // wait for WiFi connection
  if((WiFiMulti.run() == WL_CONNECTED)) {

      HTTPClient http;

      USE_SERIAL.print("[HTTP] begin...\n");
      // configure traged server and url
      //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
      http.begin(host, port, "/api/v1/iot/test"); //HTTP

      USE_SERIAL.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if(httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

          // file found at server
          if(httpCode == HTTP_CODE_OK) {
              String payload = http.getString();
              if (updateStates(payload) == 1)
              {
                  USE_SERIAL.println("Payload parsed ... [OK]");
                  changeStates();
              }
              else
              {
                  USE_SERIAL.println("Payload parsed ... [FAILED]");
                  USE_SERIAL.println(payload);
              }
          }
      }
      else
      {
          USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
      
      delay(3000);
  }
  else
  {
      delay(10000);
  }
}

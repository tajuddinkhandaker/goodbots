#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//for LED status
#include <Ticker.h>

#include "ComponentManager.h"
#include "HttpConnectionManager.h"

Ticker ticker;
// (D4) => 2, (D2) => 0
int relayInputs[] = { 2, 0 };
int lightsStates[] = { 1, 0 };// 2 lights: first one ON, second one OFF

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void switchStates()
{

  digitalWrite(relayInputs[0], lightsStates[0] == 1 ? LOW : HIGH); // turn relay on/ off
  Serial.printf("Light 1 %s\n", lightsStates[0] == 1 ?  "ON" : "OFF");

  digitalWrite(relayInputs[1], lightsStates[1] == 1 ? LOW : HIGH); // turn relay on/ off
  Serial.printf("Light 2 %s\n", lightsStates[1] == 1 ?  "ON" : "OFF");
  delay(1000);
}

bool isLogResponse(JsonObject& root)
{
  return root["data"].success() && root["data"]["log"].success();
}

// JSON input string.
//
// It's better to use a char[] as shown here.
// If you use a const char* or a String, ArduinoJson will
// have to make a copy of the input in the JsonBuffer.
// @param payload
bool updateStates(const String& payload)
{
  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  // const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(2) + 20);

  // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.parseObject(payload);

  // Test if parsing succeeds.
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }
  bool updated = root["updated"];
  if (updated)
  {
    lightsStates[0] = root["lights"][0]; // 1
    lightsStates[1] = root["lights"][1]; // 0
  }
  return updated;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  while (!Serial)
  {
    Serial.println("Waiting for serial ... ");
    delay(10);
    continue;
  }
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...[OK]");
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);

  // put your setup code here, to run once:
  pinMode(relayInputs[0], OUTPUT); // initialize pin as OUTPUT
  pinMode(relayInputs[1], OUTPUT); // initialize pin as OUTPUT
}

void loop() {
  // put your main code here, to run repeatedly:
  HTTPClient http;
  HttpConnectionManager conn(http);
  if (conn.authenticate())
  {
    Serial.println("Ready to send/ fetch automation data.");
    String response;
    String uri = "/api/v1/clients/components/states/botclients/"; //"/api/v1/clients/components/states/botclients/"
    uri += client_id;
    if (conn.get(uri.c_str(), "", response))
    {
      //Serial.println(response);
    
      if (updateStates(response))
      {
        Serial.println("Component updated... [OK]");
        switchStates();
      }
    }
  }

  http.end();

  delay(10000); //was: 3000
}

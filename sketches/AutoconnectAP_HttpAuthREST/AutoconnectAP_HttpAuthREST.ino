
#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <ESP8266HTTPClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//for LED status
#include <Ticker.h>

#define DEBUG true

Ticker ticker;
// (D4) => 2, (D2) => 0
int relayInputs[] = { 2, 0 };
int lightsStates[] = { 1, 0 };// 2 lights: first one ON, second one OFF

// server config
const char* host = "goodbots.asdtechltd.com";
const uint16_t port = 80;

// SSL Setup
// http://askubuntu.com/questions/156620/how-to-verify-the-ssl-fingerprint-by-command-line-wget-curl/
// echo | openssl s_client -connect www.googleapis.com:443 | openssl x509 -fingerprint -noout

const char* fingerprint = "D9 E4 0E 51 98 C6 11 B5 5C E4 DC C5 AF D3 96 42 37 B3 27 4A";
// D9:E4:0E:51:98:C6:11:B5:5C:E4:DC:C5:AF:D3:96:42:37:B3:27:4A

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
    Serial.println("parseObject() failed");
    return false;
  }
  return true;
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

void handleHttpResponse(HTTPClient& http, int httpCode)
{
  switch (httpCode)
  {
    case HTTP_CODE_OK:
      {
        // file found at server
        String payload = http.getString();
        if (updateStates(payload))
        {
          Serial.println("Payload parsed ... [OK]");
          switchStates();
        }
        else
        {
          Serial.println("Payload parsed ... [FAILED]");
#ifdef DEBUG
          Serial.println(payload);
#endif
        }
        break;
      }
    case HTTP_CODE_FOUND:
      {
        String payload = http.getString();
        Serial.printf("[HTTP] GET... Found with redirection url [%s]\n", payload.c_str());

        delay(5000);
        if (client(http, "/login", false, "POST", "") != httpCode)
        {
          handleHttpResponse(http, client(http, "/login", true, "POST", ""));
        }
        break;
      }
    //case HTTPC_ERROR_CONNECTION_REFUSED:
    case HTTP_CODE_UNAUTHORIZED:
      {
        Serial.println("[HTTP] GET... Unauthorized");
        String auth_uri = "/oauth/authorize?";
        auth_uri += "&client_id=1";
        auth_uri += "&redirect_uri=urn:ietf:wg:oauth:2.0:oob";
        auth_uri += "&response_type=code";
        auth_uri += "&scope=";

        delay(5000);
        if (client(http, auth_uri, false, "GET", "") == httpCode)
        {
          handleHttpResponse(http, client(http, auth_uri, true, "GET", ""));
        }
        break;
      }
    default:
      break;
  }
}

int client(HTTPClient& http, const String& uri, bool https, const char* type, const String& payload)
{

  Serial.print("[HTTP] begin...\n");
  // configure target server and url
  // http.begin(host, port, uri, fingerprint); //HTTPS
  // http.begin(host, port, uri); //HTTP
  String url = https ? "https://" : "http://";
  url += host;
  url += uri;
  https ? http.begin(url, fingerprint) : http.begin(url);
  http.setAuthorization("firewings1097@gmail.com", "123456");

  Serial.printf("[HTTP] %s...%s\n", type, payload.length() == 0 ? "[No Payload]" : "[With Payload]" );
  // start connection and send HTTP header
  int httpCode = payload.length() == 0 ? http.sendRequest(type) : http.sendRequest(type, payload);;

  if (httpCode < 0)
  {
    Serial.printf("[HTTP] %s... [%d], error: %s\nURI=[%s]\n", type, httpCode, HTTPClient::errorToString(httpCode).c_str(), url.c_str());
  }
  else
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] %s... code: %d\nURI=[%s]\n", type, httpCode, url.c_str());
  }
  return httpCode;
}

void loop() {
  // put your main code here, to run repeatedly:
  HTTPClient http;

  // try to a host
  // curl -u firewings1097@gmail.com:123456 -G http://goodbots.asdtechltd.com:80/api/v1/iot/test
  handleHttpResponse(http, client(http, "/api/v1/iot/test", false, "GET", ""));

  http.end();

  delay(5000); //was: 3000
}

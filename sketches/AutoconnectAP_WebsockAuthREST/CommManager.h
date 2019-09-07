
/**************************************************************
   CommManager is a library for the electrical automation component
   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/

#ifndef CommManager_h
#define CommManager_h

#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//for LED status
#include <Ticker.h>

#include "config.h"

class CommManager
{
public:
  CommManager() = default;
  bool connect(bool resetSettings)
  {
    //set led pin as output
    pinMode(BUILTIN_LED, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    m_ticker.attach(0.6, m_tickCallback);
  
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    
    //reset settings - for testing
    if (resetSettings)
    {
      wifiManager.resetSettings();
    }
  
    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(m_configCallback);
  
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
    
    m_ticker.detach();
    
    //keep LED on
    digitalWrite(BUILTIN_LED, LOW);
    return true;
  }

private:
  static void tick()
  {
    //toggle state
    int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
    digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
  }
  
  //gets called when WiFiManager enters configuration mode
  static void configModeCallback (WiFiManager* myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
    m_ticker.attach(0.2, m_tickCallback);
  }

private:
  static Ticker m_ticker;
  static constexpr Ticker::callback_t m_tickCallback = &tick;
  static constexpr void (*m_configCallback) (WiFiManager*) = &configModeCallback;
};

#endif

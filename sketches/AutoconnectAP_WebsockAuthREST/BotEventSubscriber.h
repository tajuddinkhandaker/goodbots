
/**************************************************************
   BotEventSubscriber is a library for the electrical automation component
   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/

#ifndef BotEventSubscriber_h
#define BotEventSubscriber_h

#include <ESP8266WiFi.h>

#include <WebSocketsClient.h>

#include <Hash.h>

#include "config.h"

class BotEventSubscriber
{
public:
  BotEventSubscriber() = default;

  void subscribe()
  {
    m_webSocket.beginSSL(HOST, SOCKET_LISTEN_PORT); //192.168.0.123
    m_webSocket.onEvent(std::bind(&BotEventSubscriber::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void listen()
  {
    m_webSocket.loop();
  }
  
private:
  void webSocketEvent(WStype_t type, uint8_t* payload, size_t length)
  {
      switch(type) {
          case WStype_DISCONNECTED:
              Serial.printf("[WSc] Disconnected!\n");
              break;
          case WStype_CONNECTED:
              {
                  Serial.printf("[WSc] Connected to url: %s\n",  payload);
          
                  // send message to server when Connected
                  m_webSocket.sendTXT("Connected");
              }
              break;
          case WStype_TEXT:
              Serial.printf("[WSc] get text: %s\n", payload);
  
              // send message to server
              // m_webSocket.sendTXT("message here");
              break;
          case WStype_BIN:
              Serial.printf("[WSc] get binary length: %u\n", length);
              hexdump(payload, length);
  
              // send data to server
              // m_webSocket.sendBIN(payload, length);
              break;
      }
  
  }

private:
  WebSocketsClient m_webSocket;
};

#endif

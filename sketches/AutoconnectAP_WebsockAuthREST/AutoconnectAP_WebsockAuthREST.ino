/*
 * AutoconnectAP_WebsocketAuthREST.ino
 *
 *  Created on: 1.04.2018
 *
 *  note SSL is only possible with the ESP8266
 *
 */

#include "CommManager.h"
#include "BotEventSubscriber.h"

BotEventSubscriber g_eventSubscriber;

void startSerialComm()
{  
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
}

void setup() {
  
  startSerialComm();
  CommManager commManager;
  if (commManager.connect(false))
  {
    g_eventSubscriber.subscribe();
  }
}

void loop() {
    g_eventSubscriber.listen();
}

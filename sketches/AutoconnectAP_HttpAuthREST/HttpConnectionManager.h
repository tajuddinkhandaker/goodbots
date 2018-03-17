/**************************************************************
   HTTP connection and authentication manager

   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/
#ifndef HTTPCONNECTIONMANAGER_H
#define HTTPCONNECTIONMANAGER_H

#include <ArduinoJson.h>
#include "config.h"
#include "common.h"

String accessToken;

class HttpConnectionManager
{
  public:
    HttpConnectionManager(HTTPClient& http)
      : m_http(http)
    {}
    void clearToken()
    {
      accessToken.remove(0, accessToken.length());
    }   
    
    bool authenticate()
    {
        if (hasAccessToken())
          return true;
        const char* keys[] = { "grant_type", "client_id", "client_secret", "scope" };
        const char* values[] = { "client_credentials", client_id, client_secret, "*" };
        String payload;
        toJson(keys, values, 4, payload);
        int httpCode = client(m_http, "/oauth/token", false, "POST", payload);
        String response;
        handleHttpResponse(httpCode, response);
        return httpCode == HTTP_CODE_OK && parseToken(response);
    }

    void handleHttpResponse(int httpCode, String& payload)
    {
      switch (httpCode)
      {
        case HTTP_CODE_OK:
          {
            // file found at server
            payload = m_http.getString();
    #ifdef DEBUG
            Serial.println(payload);
    #endif
            break;
          }
        default:
         {
            int newHttpCode = RLog(m_http, httpCode, "... [FAILED]");
            if (newHttpCode != httpCode)
            {
              handleHttpResponse(newHttpCode, payload);
            }
            break;
         }
      }
    }
    
   private:
    bool hasAccessToken() { return accessToken.length() > 0; }
    bool parseToken(const String& response)
    {
        DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(3) + 1250);
        JsonObject& root = jsonBuffer.parseObject(response);
        if (!root.success())
        {
          Serial.println("access token parse failed");
          return false;
        }
        //const char* token_type = root["token_type"]; // "Bearer"
        //long expires_in = root["expires_in"]; // 31536000
        
        const char* token = root["access_token"];
        accessToken = token;
        return hasAccessToken();
    }
    
   private:
    HTTPClient& m_http;
};

#endif

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
uint16_t timeoutSec = 10;
 
class HttpConnectionManager
{
  public:
    HttpConnectionManager(HTTPClient& http)
      : m_http(http)
    {} 
    
    bool authenticate()
    {
        if (hasAccessToken())
          return true;
        const char* keys[] = { "grant_type", "client_id", "client_secret", "scope" };
        const char* values[] = { "client_credentials", client_id, client_secret, "check-components-status write-error-logs" };
        String payload;
        toJson(keys, values, 4, payload);
        int httpCode = client(m_http, "/oauth/token", false, "POST", "", payload);
        String response;
        handleHttpResponse(httpCode, response);
        //Serial.println(payload);
        return httpCode == HTTP_CODE_OK && parseToken(response);
    }

    bool get(const char* route, const String& payload, String& response)
    {
        int httpCode = client(m_http, route, false, "GET", accessToken, payload);
        handleHttpResponse(httpCode, response);
        if (httpCode == HTTPC_ERROR_READ_TIMEOUT)
        {
          m_http.setTimeout(timeoutSec++ * 1000);
          Serial.printf("Timeout fixed at: %d seconds\n", timeoutSec);
        }
        return httpCode == HTTP_CODE_OK;
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
            if (httpCode >= HTTP_CODE_BAD_REQUEST && httpCode <= HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE)
            {
              clearToken();
            }
            else if (httpCode > 0)
            {
              RLog(m_http, httpCode, "... [FAILED]");
            }
            break;
         }
      }
    }
    
   private:
    void clearToken()
    {
      accessToken.remove(0, accessToken.length());
    }  
    bool hasAccessToken() { return accessToken.length() > 0; }
    static const char* toBearerToken(const char* token)
    {
        //Serial.println(token);        
        char bearerToken[strlen(token) + 8];
        strcpy(bearerToken, "Bearer ");
        strcat(bearerToken, token);
        return bearerToken;
    }
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
        accessToken = toBearerToken(token);
        return hasAccessToken();
    }
    
   private:
    HTTPClient& m_http;
    int m_timeoutSec = 10;
};

#endif

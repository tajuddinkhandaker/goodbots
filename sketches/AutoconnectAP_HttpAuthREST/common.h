
/**************************************************************
   Common globals header

   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/
#ifndef COMMON_H
#define COMMON_H

int client(HTTPClient& http, const String& uri, bool https, const char* type, const String& bearerToken, const String& payload)
{
  // configure target server and url
  // http.begin(host, port, uri, fingerprint); //HTTPS
  // http.begin(host, port, uri); //HTTP
  String url = https ? "https://" : "http://";
  url += host;
  url += uri;
  https ? http.begin(url, fingerprint) : http.begin(url);
  //http.setAuthorization(username, password);
  Serial.print("[HTTP] begin...\n");

  http.addHeader("Accept", "application/json");
  if ( payload.length() > 0  && !http.hasHeader("Content-Type"))
  {
    http.addHeader("Content-Type", "application/json; charset=utf-8");
  }
  if (bearerToken.length() > 0 && !http.hasHeader("Authorization"))
  {
    http.addHeader("Authorization", bearerToken);
  }

  // start connection and send HTTP header
  int httpCode = payload.length() == 0 ? http.sendRequest(type) : http.sendRequest(type, payload);

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

void toJson(const char* keys[], const char* values[], size_t count, String& payload)
{
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(count));

  JsonObject& root = jsonBuffer.createObject();
  for (size_t index = 0; index < count; ++index)
  {
    root[keys[index]] = values[index];
  }
  root.printTo(payload);
} 

int RLog(HTTPClient& http, int httpCode, const char* msg)
{
    char code[4];
    snprintf(code, sizeof(code), "%d", httpCode);        
    const char* keys[] = { "client_id", "http_code", "message" };
    const char* values[] = { client_id, code, msg };
    String payload;
    toJson(keys, values, 3, payload);
    return client(http, "/api/v1/clients/log", false, "POST", "", payload);
}

#endif

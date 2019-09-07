
/**************************************************************
   Configuration header

   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

//#define DEBUG
#define MAX_COMPONENT_COUNT 2 // channel count of interfaced relay
#define MAX_PROPERTY_COUNT 1  // ON/ OFF state

// server config
#define HOST "goodbots.asdtechltd.com"
#define PORT 80
#define SOCKET_LISTEN_PORT 81

// SSL Setup
// http://askubuntu.com/questions/156620/how-to-verify-the-ssl-fingerprint-by-command-line-wget-curl/
// echo | openssl s_client -connect www.googleapis.com:443 | openssl x509 -fingerprint -noout

// D9:E4:0E:51:98:C6:11:B5:5C:E4:DC:C5:AF:D3:96:42:37:B3:27:4A
#define FINGERPRINT "D9 E4 0E 51 98 C6 11 B5 5C E4 DC C5 AF D3 96 42 37 B3 27 4A"

#define USERNAME "firewings1097@gmail.com"
#define PASSWORD "123456"

// try to a host
// curl -u firewings1097@gmail.com:123456 -G http://goodbots.asdtechltd.com:80/api/v1/iot/test
// curl -H "Content-Type: application/json" -X POST -d '{"client_id": 1,"client_secret":"HGCGdwIXuc25YfNttnpY4CMIs8zCwiK4DnarIlPX", "grant_type": "client_credentials"}' http://goodbots.asdtechltd.com:80/oauth/token
#define CLIENT_ID "1"
#define CLIENT_SECRET "HGCGdwIXuc25YfNttnpY4CMIs8zCwiK4DnarIlPX"

#endif

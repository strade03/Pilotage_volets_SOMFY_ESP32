#ifndef WOL_H_
#define WOL_H_

#include "Arduino.h"
#include <WiFiUdp.h>

#define IP_LGTH 16

void cleanMac(char* str);
// Fonctions génériques adresse mac sous la forme xx:xx:xx:xx:xx:xx ip 192.168.0.1
void sendWOL(char* mac,char* serveur_ip);


#endif //WOL_H_

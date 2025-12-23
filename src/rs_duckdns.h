#ifndef RS_DUCKDNS_H_
#define RS_DUCKDNS_H_

#include "Arduino.h"

#define DUCKDNS_DOMAIN_LENGTH 64
#define DUCKDNS_TOKEN_LENGTH 40

// Met à jour l'IP sur DuckDNS
bool update_duckdns();

// Vérifie si DuckDNS est configuré
bool is_duckdns_configured();

#endif /* RS_DUCKDNS_H_ */
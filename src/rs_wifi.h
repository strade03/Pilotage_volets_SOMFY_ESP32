#ifndef RS_WIFI_H_
#define RS_WIFI_H_

#include "Arduino.h"

#define MAX_FAILING_WIFI 10

String get_liste_reseau(void);
void scanner_reseau(void);
bool connect_to_wifi();
void start_softap(void);
void handle_dns_requests(void);
void check_wifi(void);
bool check_internet(void);

#endif /* RS_WIFI_H_ */

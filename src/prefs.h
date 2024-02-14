#ifndef PREFS_H_
#define PREFS_H_

#include "Arduino.h"

#include <Preferences.h>

#define KEY_LENGTH 10
#define IP_LENGTH 16
#define NTP_SERVER_LENGTH 20
#define ACCESSPOINT_LENGTH 28
#define PASSWORD_LENGTH 32

// Fonctions génériques
void prefs_set_str(const char* folder,const char* name_key,String key);
void prefs_get_str(const char* folder,const char* name_key,char * key, const int length,const char* defaultValue);
void prefs_set_bool(const char* folder,const char* name_key,boolean key);
boolean prefs_get_bool(const char* folder,const char* name_key,boolean defaultValue);
void prefs_set_long(const char* folder,const char* name_key,long key);
long prefs_get_long(const char* folder,const char* name_key,long defaultValue);






uint16_t prefs_get_prgmcount();
void prefs_set_prgmcount(int prgmcount);
void prefs_loadprgms(void);
void prefs_storeprgms(void);

#endif //PREFS_H_

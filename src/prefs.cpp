#include "prefs.h"
#include "prgm.h"
#include "misc.h"

Preferences preferences;

char buffer[PRGM_COUNT * sizeof(prgm_t)];
prgm_t *prgms = (prgm_t *) buffer;


void prefs_set(const char* folder,const char* name_key,String key) {
  preferences.begin(folder,false);
  preferences.putString(name_key, key);
  preferences.end();
}

void prefs_get(const char* folder,const char* name_key,char * key,const int length,const char* defaultValue) {
  preferences.begin(folder,false);
  if (preferences.isKey(name_key)) {
    preferences.getString(name_key).toCharArray(key, length);
  } else {
    // La clé n'existe pas, utilisez la valeur par défaut
    strncpy(key, defaultValue, length);
  }
  preferences.end();
}

void prefs_set_bool(const char* folder,const char* name_key,boolean key) {
  preferences.begin(folder,false);
  preferences.putBool(name_key, key);
  preferences.end();
}

boolean prefs_get_bool(const char* folder,const char* name_key,boolean defaultValue) {
  boolean result = defaultValue;
  preferences.begin(folder,false);
  if (preferences.isKey(name_key)) 
    result=preferences.getBool(name_key);
  preferences.end();
  return result; 
}

void prefs_set_long(const char* folder,const char* name_key,long key) {
  preferences.begin(folder,false);
  preferences.putLong(name_key, key);
  preferences.end();
}

long prefs_get_long(const char* folder,const char* name_key,long defaultValue) {
  long result = defaultValue;
  preferences.begin(folder,false);
  if (preferences.isKey(name_key)) 
    result=preferences.getLong(name_key);
  preferences.end();
  return result; 
}



void prefs_set_key(String key) {
  
  write_output_ln("PREFS - Storing key");
  preferences.begin("application",false);
  preferences.putString("key", key);
  preferences.end();
  
}


void prefs_get_key(char * key) {
  
  // Serial.print("PREFS - Reading key : ");
  preferences.begin("application",false);
  preferences.getString("key").toCharArray(key, KEY_LENGTH);
  // Serial.println(key);
  preferences.end();
    
}

void prefs_set_token(String token) {
  
  write_output_ln("PREFS - Storing token");
  preferences.begin("token",false);
  preferences.putString("token", token);
  preferences.end();
  
}


void prefs_get_token(char * token) {
  
  // Serial.print("PREFS - Reading token : ");
  preferences.begin("token",false);
  preferences.getString("token").toCharArray(token, 16);
  // Serial.println(token);
  preferences.end();
    
}

void prefs_set_ntp_server(String ntp_server) {
  
  write_output_ln("PREFS - Storing ntp server url");
  preferences.begin("ntp",false);
  preferences.putString("ntp_server", ntp_server);
  preferences.end();
  
}


void prefs_get_ntp_server(char * ntp_server) {
  
  // Serial.print("PREFS - Reading ntp server url : ");
  preferences.begin("ntp",false);
  preferences.getString("ntp_server").toCharArray(ntp_server, NTP_SERVER_LENGTH);
  if(ntp_server == NULL || strlen(ntp_server) == 0 || ntp_server[0] == '\0'){
    String("pool.ntp.org").toCharArray(ntp_server, NTP_SERVER_LENGTH);
  }
  // Serial.println(ntp_server);
  preferences.end();
  
}

void prefs_set_accesspoint(String accesspoint) {
  
  write_output_ln("PREFS - Storing accesspoint");
  preferences.begin("wifi",false);
  preferences.putString("accesspoint", accesspoint);
  preferences.end();
  
}


void prefs_get_accesspoint(char * accesspoint) {
  
  // Serial.print("PREFS - Reading accesspoint : ");
  preferences.begin("wifi",false);
  preferences.getString("accesspoint").toCharArray(accesspoint, ACCESSPOINT_LENGTH);
  // Serial.println(accesspoint);
  preferences.end();
  // Serial.print("PREFS - Reading accesspoint : ");
  
}


void prefs_set_password(String password) {
  
  write_output_ln("PREFS - Storing password");
  preferences.begin("wifi",false);
  preferences.putString("password", password);
  preferences.end();
  
}


void prefs_get_password(char * password) {
  
  // Serial.print("PREFS - Reading password : ");
  preferences.begin("wifi",false);
  preferences.getString("password").toCharArray(password, PASSWORD_LENGTH);
  // Serial.println(password);
  preferences.end();
  
}


uint16_t prefs_get_prgmcount() {

  uint16_t value = 0;
  
  // Serial.print("PREFS - Reading prgmcount : ");
  preferences.begin("prgms",false);
  value = preferences.getUShort("prgmcount");
  // Serial.println(value);
  preferences.end();

  return value;
  
}


void prefs_set_prgmcount(int prgmcount) {
  
  write_output_ln("PREFS - Storing prgmcount");
  preferences.begin("prgms",false);
  preferences.putUShort("prgmcount", prgmcount);
  preferences.end();
  
}


void prefs_loadprgms() {
  
  write_output_ln("PREFS - loadprgms");
  preferences.begin("prgms",false);

  size_t prgms_length = preferences.getBytesLength("prgms");

  // Serial.print("PREFS - loadprgms - Size in memory :");
  // Serial.println(prgms_length);

  if (prgms_length != PRGM_COUNT * sizeof(prgm_t)) { // simple check that data fits
    write_output_ln("PRGM - loadprgms - Memory invalid or empty, cleaning");
    preferences.putBytes("prgms", prgms, PRGM_COUNT * sizeof(prgm_t));
  }
  
  preferences.getBytes("prgms", buffer, prgms_length);
  write_output_ln("PREFS - loadprgms - Data loaded");

  preferences.end();
}


void prefs_storeprgms() {
  
  write_output_ln("PREFS - storeprgms");
  preferences.begin("prgms",false);
  preferences.putBytes("prgms", prgms, PRGM_COUNT * sizeof(prgm_t));
  preferences.end();
}

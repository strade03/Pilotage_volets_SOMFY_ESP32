#include "prefs.h"
#include "prgm.h"
#include "misc.h"

Preferences preferences;

char buffer[PRGM_COUNT * sizeof(prgm_t)];
prgm_t *prgms = (prgm_t *) buffer;


void prefs_set_str(const char* folder,const char* name_key,String key) {
  preferences.begin(folder,false);
  preferences.putString(name_key, key);
  preferences.end();
}

void prefs_get_str(const char* folder,const char* name_key,char * key,const int length,const char* defaultValue) {
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

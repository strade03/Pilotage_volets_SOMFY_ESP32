#include "WiFi.h"
#include "time.h"
#include "sys/time.h"
#include "prefs.h"
#include "misc.h"

#include <WiFiClientSecure.h> 
#include <HTTPClient.h>  

// Pour calculer le décalage horaire et l'heure d'été/hiver. // TODO voir comment améliorer 
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

char ntpServer[NTP_SERVER_LENGTH];
bool time_set = false;

String sunrise = "";
String sunset = "";
String timezone = "";
String  temperature = "";
String pressure = "";
String humidity = "";
String visibility = "";
String  win_speed = "";
String win_deg = "";
String icon = "";
uint8_t hour_maj=0; // TODO A SUPP
uint8_t minute_maj=0;
extern bool time_set;

bool inittime(void) {
  prefs_get_ntp_server(ntpServer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    // write_output_ln("Failed to obtain time in inittime");
    time_set=false;
    return false;
  }
  time_set=true;
  return true;
}


bool gettime(uint8_t * hours, uint8_t * minutes) {

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    // Serial.println("Failed to obtain time in gettime");
    return false;
  }
  *hours = timeinfo.tm_hour;
  *minutes = timeinfo.tm_min;

  return true;
}

bool settime(uint8_t hours, uint8_t minutes) {

  struct tm tm;
  tm.tm_year = 2017 - 1900;
  tm.tm_mon = 11;
  tm.tm_mday = 8;
  tm.tm_sec = 10;
  tm.tm_hour = hours + 1;
  tm.tm_min = minutes;
  tm.tm_sec = 0;
  time_t t = mktime(&tm);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);

  time_set = true;

  return true;  
}

// --------------------------------- Traitement des informations openweathermap

// Retourne la valeur d'une clé du JSON
String keysToStr(String reponse,String chaine)
{
  reponse.setCharAt(reponse.length() - 1, ',');
  reponse.replace("{","");
  reponse.replace("[","");
  reponse.replace("}","");
  reponse.replace("]","");
  reponse.replace("\"","");
  int index = reponse.indexOf(chaine);
  if (index != -1) {
    String TempStr = reponse.substring(index + chaine.length());
    int indexComma = TempStr.indexOf(",");
    if (indexComma != -1) 
      return(TempStr.substring(0, indexComma));
    else
      return("");
  }
  else
    return("");
}

// Retourne l'heure du coucher ou lever du soleil
uint8_t heure_soleil(String type) // type = coucher ou lever
{
  
  char timestamp[12];
  char timezone[10];
  if (type=="lever") 
    prefs_get("meteo","sunrise",timestamp,12,"");
  else
  if ((type=="coucher") || (type=="nuit"))
    prefs_get("meteo","sunset",timestamp,12,"");
  prefs_get("meteo","timezone",timezone,10,"");
  if (timestamp!="") 
    if (type=="nuit") 
      return( (1800+atoi(timestamp)+atoi(timezone)) / 3600 % 24) ;
    else
      return ( (atoi(timestamp)+atoi(timezone)) / 3600 % 24) ;
  else
    return(-1);
}

// Retourne les minutes du coucher ou lever du soleil
uint8_t minute_soleil(String type) // type = coucher ou lever
{
  uint8_t h=0;
  char timestamp[12];
  char timezone[10];
  if (type=="lever") 
    prefs_get("meteo","sunrise",timestamp,12,"");
  else
  if ((type=="coucher") || (type=="nuit"))
    prefs_get("meteo","sunset",timestamp,12,"");
  if (timestamp!="")
    if (type=="nuit")
      return ( (1800+atoi(timestamp))/ 60 % 60 );
    else
      return ( atoi(timestamp)/ 60 % 60 );
  else
    return(-1);
}

String timeToStr(uint8_t hour, uint8_t minute)
{  
  String hourStr = (hour < 10)? "0"+String(hour):String(hour); 
  String minuteStr = (minute < 10)? "0"+String(minute):String(minute); 
  return(hourStr+ ":" + minuteStr);
}

String timeToStr(String type)
{
 uint8_t heure_sol; 
 uint8_t minute_sol; 
 heure_sol=heure_soleil(type);
 minute_sol=minute_soleil(type);
 if ((heure_sol>-1) && (minute_sol>-1))
   return (timeToStr(heure_sol,minute_sol));
  return("");
}

String getWindDirection(float deg) {
  if (deg >= 0 && deg < 22.5) {
    return "N";
  } else if (deg >= 22.5 && deg < 67.5) {
    return "NE";
  } else if (deg >= 67.5 && deg < 112.5) {
    return "E";
  } else if (deg >= 112.5 && deg < 157.5) {
    return "SE";
  } else if (deg >= 157.5 && deg < 202.5) {
    return "S";
  } else if (deg >= 202.5 && deg < 247.5) {
    return "SO";
  } else if (deg >= 247.5 && deg < 292.5) {
    return "O";
  } else if (deg >= 292.5 && deg < 337.5) {
    return "NO";
  } else {
    return "N";
  }
}

// Traitement du JSON 
void extrationJSON(String reponse){
 
  String sunrise_tmp ="";
  String sunset_tmp ="";
  sunrise_tmp = keysToStr(reponse,"sunrise:");
  sunset_tmp = keysToStr(reponse,"sunset:"); 
  timezone = keysToStr(reponse,"timezone:"); //.toInt();
  icon = keysToStr(reponse,"icon:"); 
  temperature = keysToStr(reponse,"temp:"); 
  pressure = keysToStr(reponse,"pressure:");
  humidity = keysToStr(reponse,"humidity:");
  visibility = keysToStr(reponse,"visibility:");
  win_speed = keysToStr(reponse,"speed:");
  win_deg = keysToStr(reponse,"deg:"); 
  
  if (sunset_tmp!="") sunset=sunset_tmp;
  if (sunrise_tmp!="") sunrise=sunrise_tmp;

  
  
  gettime(&hour_maj, &minute_maj);
// Sauvegarde dans la mémoire flash l'heure de coucher et de lever du soleil si modification
  char sunrise_base[12] = "";
  prefs_get("meteo","sunrise",sunrise_base,12,"");
  char sunset_base[12] = "";
  prefs_get("meteo","sunset",sunset_base,12,"");

  if (strcmp(sunrise.c_str(), sunrise_base)!=0)
    prefs_set("meteo","sunrise",sunrise);
  if (strcmp(sunset.c_str(), sunset_base)!=0)
    prefs_set("meteo","sunset",sunset);
}


// Lecture des informations sur le serveur openweathermap
bool get_meteo()
{
  char appid[50];
  char lon[10];
  char lat[10];
  prefs_get("openweathermap","api",appid,50,"");
  prefs_get("openweathermap","lon",lon,10,"2.3333");
  prefs_get("openweathermap","lat",lat,10,"48.8666");

    const String request = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&units=metric&lang=fr&appid=" + String(appid) ; // requête http
    
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(client, request); // on prépare la requête
    int httpCode = http.GET(); // on lance la requête
    if (httpCode == 200 ) { // le serveur a répondu 200 (connexion réussie)
      String reponse = http.getString();
      //Serial.println(reponse);
      extrationJSON(reponse);
      http.end();
      return(true);
    } else {
      Serial.print("Error - HTTP code : ");
      Serial.println(httpCode);
      http.end();
      return(false);
    }
}


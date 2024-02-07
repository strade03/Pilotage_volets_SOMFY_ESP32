#include "WiFi.h"
#include "rs_wifi.h"
#include "DNSServer.h"
#include "prefs.h"
#include "ESP32Ping.h"
#include "misc.h"

#include "rs_webserver.h" 

#define CONNECTION_TIMEOUT 20

IPAddress apIP(10, 10, 10, 1);
DNSServer dnsServer;
// IP to ping
IPAddress IPtoPing(8, 8, 8, 8);
int pingThreshold = 2000;
int failingWifi = 0;
String Liste_reseau;

//------------------------------------------------------------------------------------------------------------------------------------
// Echelle de qualité de reception du Wifi
//------------------------------------------------------------------------------------------------------------------------------------
int getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

//--------------------------------------------------------------------------------------------------
// Scan les bornes Wifi
//------------------------------------------------------------------------------------------------------
void scanner_reseau(void) {
      // Liste_reseau="<div></div>";
      Liste_reseau="";

      int n = WiFi.scanNetworks();

      write_output_ln("Scan reseau");
      if (n == 0) {
          write_output_ln("no networks found");
      } else {
          // Serial.print(n);
          write_output_ln(" networks found");
          // Tri selon puissance
          int indices[n];
          for (int i = 0; i < n; i++) {
            indices[i] = i;
            }
            for (int i = 0; i < n; i++) {
              for (int j = i + 1; j < n; j++) {
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                  std::swap(indices[i], indices[j]);
                }
              }
            }
          
          // Fin tri
          for (int i = 0; i < n; ++i) {

            int rssiperc = getRSSIasQuality(WiFi.RSSI(indices[i]));
            uint8_t enc_type = WiFi.encryptionType(indices[i]);
            Liste_reseau+="<div>\
              <a href=\"#p\" onclick=\"c(this)\" data-ssid=\""+htmlEntities(WiFi.SSID(indices[i]),true)+"\">"+htmlEntities(WiFi.SSID(indices[i]),true)+"</a>\
              <div role=\"img\" aria-label=\""+(String)rssiperc+"%\" title=\""+(String)rssiperc+"%\" class=\"q q-"+(String)int(round(map(rssiperc,0,100,1,4)))+" "+(String)(enc_type != WIFI_AUTH_OPEN ? "l":" ") +" \"></div>\
              <div class=\"q h\">"+(String)rssiperc+"%</div>\
              </div>";
          }
      }
}


//------------------------------------------------------------------------------------------------------
// Connection WIFI
//------------------------------------------------------------------------------------------------------
bool connect_to_wifi() {
  char ssid[ACCESSPOINT_LENGTH];
  prefs_get("wifi","accesspoint",ssid,ACCESSPOINT_LENGTH,"");
  char password[PASSWORD_LENGTH];
  prefs_get("wifi","password",password,ACCESSPOINT_LENGTH,"");
  
  char reseau_ip[IP_LENGTH];
  prefs_get("reseau","ip",reseau_ip,IP_LENGTH,"192.168.1.111");
  char reseau_masque[IP_LENGTH];
  prefs_get("reseau","masque",reseau_masque,IP_LENGTH,"255.255.255.0");
  char reseau_passerelle[IP_LENGTH];
  prefs_get("reseau","passerelle",reseau_passerelle,IP_LENGTH,"192.168.1.254");
  boolean reseau_dhcp=prefs_get_bool("reseau","dhcp",true);

  int timeout_counter = 0;

  write_output_ln("WIFI - Connect to access point");
  write_output_ln(ssid);
    
  if (strlen(ssid)==0) return(false);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
     delay(500);
    //  Serial.print("*");
     timeout_counter++;
     if(timeout_counter >= CONNECTION_TIMEOUT){
       Serial.print("Connexion impossible ! à ");
       write_output_ln(ssid);
      return(false);
   }
  }
  
  // definition des parametres reseau si pas DHCP
  if (reseau_dhcp==false)
   {
      int a, b, c, d;
      sscanf(reseau_ip, "%d.%d.%d.%d", &a, &b, &c, &d);
      IPAddress ip(a, b, c, d);      // Adresse IP fixe
      sscanf(reseau_masque, "%d.%d.%d.%d", &a, &b, &c, &d);
      IPAddress subnet(a, b, c, d);      // masque
      sscanf(reseau_passerelle, "%d.%d.%d.%d", &a, &b, &c, &d);
      IPAddress gateway(a, b, c, d);   // Adresse IP de la passerelle
      IPAddress dns(1, 1, 1, 3);
      WiFi.config(ip, gateway, subnet,dns);
   }
  
   write_output_ln("");
   write_output_ln("WIFI - Connected !");
   Serial.print("WIFI IP Address : ");

  Serial.println(WiFi.localIP());
  
  IPtoPing = WiFi.gatewayIP();
  Serial.print("Gateway IP Address : ");
  write_output_ln(IPtoPing.toString());

  return(true);
}

void check_wifi(void){
  int avg_ms = 2000;
  boolean ping_sucess = Ping.ping(IPtoPing, 9);
  avg_ms = Ping.averageTime();

  write_output_ln("ping:" + String(avg_ms) + "ms");

  if(avg_ms < pingThreshold && ping_sucess){
    failingWifi=0;
  }else{
    write_output_ln("Connection Failed! Reconnecting...");
    WiFi.reconnect();
    delay(2000);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      write_output_ln("Connection failed again.");
      failingWifi=failingWifi+1;
      if (failingWifi>MAX_FAILING_WIFI){
        write_output_ln("ESP will restart now...");
        delay(100);
        ESP.restart();
      }
    }
    write_output_ln("Connection fails but reconnected :)");
  }
}

bool check_internet(void){
  int avg_ms = 2000;
  boolean ping_sucess = Ping.ping(IPtoPing, 9);
  avg_ms = Ping.averageTime();
  if(avg_ms < pingThreshold && ping_sucess){
    return(true);
  }else{
    return(false);
  }
}
//------------------------------------------------------------------------------------------------------
// Mode AP
//------------------------------------------------------------------------------------------------------
void start_softap(void) {
  //scan_reseau(); 
  write_output_ln("WIFI - start_softap - Starting rescue mode");
  Serial.printf("WIFI - IP : ");
  Serial.println(apIP);
  
  
  WiFi.mode(WIFI_AP);
  delay(2000);
  //WiFi.setHostname("voletsroulants"); Inutile
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  //WiFi.softAP("VoletsRoulants");
  WiFi.softAP("VoletsRoulants");
  dnsServer.start(53, "*", apIP);  //Cause a reboot in softAP mode  
}

void handle_dns_requests(void) {
  dnsServer.processNextRequest();
}

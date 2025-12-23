#include "rs_duckdns.h"
#include "prefs.h"
#include "misc.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

bool update_duckdns() {
  char domain[DUCKDNS_DOMAIN_LENGTH];
  char token[DUCKDNS_TOKEN_LENGTH];
  
  prefs_get_str("duckdns", "domain", domain, DUCKDNS_DOMAIN_LENGTH, "");
  prefs_get_str("duckdns", "token", token, DUCKDNS_TOKEN_LENGTH, "");
  
  // Vérifier si DuckDNS est configuré
  if (strlen(domain) == 0 || strlen(token) == 0) {
    write_output_ln("DUCKDNS - Non configuré, mise à jour ignorée");
    return false;
  }
  
  write_output_ln("DUCKDNS - Mise à jour en cours...");
  
  // Construction de l'URL
  String url = "https://www.duckdns.org/update?domains=" + String(domain) + 
               "&token=" + String(token) + "&ip=";
  
  WiFiClientSecure client;
  client.setInsecure(); // Pour éviter les problèmes de certificat SSL
  
  HTTPClient http;
  http.begin(client, url);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String response = http.getString();
    http.end();
    
    if (response.startsWith("OK")) {
      write_output_ln("DUCKDNS - Mise à jour réussie");
      
      // Sauvegarder l'horodatage de la dernière mise à jour
      prefs_set_long("duckdns", "last_update", millis() / 1000);
      
      return true;
    } else {
      write_output_ln("DUCKDNS - Échec : " + response);
      return false;
    }
  } else {
    Serial.print("DUCKDNS - Erreur HTTP : ");
    Serial.println(httpCode);
    http.end();
    return false;
  }
}

bool is_duckdns_configured() {
  char domain[DUCKDNS_DOMAIN_LENGTH];
  char token[DUCKDNS_TOKEN_LENGTH];
  
  prefs_get_str("duckdns", "domain", domain, DUCKDNS_DOMAIN_LENGTH, "");
  prefs_get_str("duckdns", "token", token, DUCKDNS_TOKEN_LENGTH, "");
  
  return (strlen(domain) > 0 && strlen(token) > 0);
}
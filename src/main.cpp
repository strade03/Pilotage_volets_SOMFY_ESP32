#include "WiFi.h"
#include "rs_wifi.h"
#include "rs_webserver.h"
#include "prefs.h"
#include "rs_time.h"
#include "pins.h"
#include "misc.h"
// #include "esp32-hal-cpu.h"
#include "rs_scheduledtasks.h"
#include "prgm.h"
#include "SomfyRTS.h"

int rescue_mode = 0;
bool internet_ok= false;
const long interval = 2000;
unsigned long previousMillis = 0;
extern bool time_set;
bool wifi_connected;

extern SomfyRTS somfy;
void setup() {
  wifi_connected =  false;
  unsigned long RTS_address=prefs_get_long("somfy","rts",0x121340);
  somfy.setRTS_address(RTS_address);
  Serial.begin(115200);
  write_output_ln("\n\n** Boot in progress....");

  setCpuFrequencyMhz(80); // Réduction de la fréquance CPU pour économiser l'energie
  // int cpuSpeed = getCpuFrequencyMhz();
  // Serial.print("CPU Frequency :");
  // Serial.println(cpuSpeed);

  pinMode(RESCUE_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  rescue_mode=(connect_to_wifi()==true)?0:1;   

  if (rescue_mode) {
    start_softap();           // Mode AP
    enable_wifiBlinkTask();   // Clignotement STATUS_LED_PIN 
    // TODO activer une task qui reboot au bout de 5 min. Si la page d'accueil est affichée on annule
    // Cas ou un reboot de la box coupe aussi l'alimentation du boitier, dans ce cas si pas on accède pas à la page de config on retente la connection
    // Gérer peut être un compteur de nombre de reboot pour l'empecher au bout de 5 ou 6 fois !
  }
  else {
    scanner_reseau();
    internet_ok=check_internet();
    Serial.print("Etat internet : ");
    Serial.println(internet_ok);
    
    // enable_checkWifiTask(); // TODO A REMETTRE ? si pas de connection reboot donc perte du temps ! // parametrage de la Task qui appelera check_wifi fonction qui test via ping 8.8.8.8 et renseigne la variable failingWifi = 0 si ok sinon retente la connexion puis reboot en cas d'echec
    if (inittime()) {       // Mise à l'heure
      digitalWrite(STATUS_LED_PIN, LOW);
    } 
    else {
      settime(0,0); // on initialise une heure pour éviter les bugs 
    }
    if (get_meteo()) {// Lecture de la meteo et heure de lever et coucher du soleil

      enable_initMeteoTask(); 
    }
  }
  
  enable_initTimeTask(); // initialise de temps en temps l'heure
  ws_config(rescue_mode); // Configuration initialisation générale avec les liens des pages => fonctions
  
  prefs_loadprgms(); // charge programmes 
  refresh_programTask(); // lance les taches des programmes  

  write_output_ln("End setup function");

// // Si bug pour effacer tous les enregistrements des telecommandes.
// Preferences prefs;
// prefs.begin("SomfyRTS", false);
// // Supprimer tous les enregistrements associés à la clé "SomfyRTS"
// prefs.clear();
// prefs.end();

}

int Etat_bouton = 0;
int Dernier_Etat_bouton = 0;

void loop() {
  
  if (rescue_mode) {
    handle_dns_requests();
  }else
  {
    if (!time_set)  {
      write_output_ln("Main.Loop - L'heure n'est pas définie, j'essaie de l'obtenir...");
      // if (inittime()) {
      //   digitalWrite(STATUS_LED_PIN, LOW);
      //   write_output_ln("Main.Loop - Réglage de l'heure");
      //   time_set = true;
      // }
    }
  }
  // Task scheduler
  execute_runner(); 

// ---- Mode AP si appui sur le bouton pour accèder aux paramètres
  Etat_bouton = digitalRead(RESCUE_PIN);
  if (Etat_bouton == LOW && Dernier_Etat_bouton == HIGH)
  {                             // détection de l'appui sur le bouton 1
    previousMillis = millis(); // initialisation du compteur 1
    if (!rescue_mode) digitalWrite(STATUS_LED_PIN, HIGH);  
  }
  
  if (Etat_bouton == HIGH && !rescue_mode) digitalWrite(STATUS_LED_PIN, LOW);

  if (previousMillis != 0 && Etat_bouton == LOW && Dernier_Etat_bouton == LOW)
  { // le bouton 1 était et est toujours appuyé
    if ((millis() - previousMillis) >= interval)
    { // est-ce que le temps choisi est écoulé ?
      if (rescue_mode==0) {
        previousMillis = 0;
        rescue_mode=1;
        enable_wifiBlinkTask();
        internet_ok=false;
        start_softap();  
      }
      else
      {
        ESP.restart();
      }
    }
  }
  Dernier_Etat_bouton = Etat_bouton; // actualisation de l'état du bouton
}

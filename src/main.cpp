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

int rescue_mode = 0;
bool internet_ok= false;
const long interval = 2000;
unsigned long previousMillis = 0;
extern bool time_set;
bool wifi_connected;
uint8_t previous_hour = 0;
uint8_t previous_minute = 0;


void setup() {
  wifi_connected =  false;
  
  Serial.begin(115200);
  write_output_ln("\n\n** Boot in progress....");

  //setCpuFrequencyMhz(80);
  // int cpuSpeed = getCpuFrequencyMhz();
  // Serial.print("CPU Frequency :");
  // Serial.println(cpuSpeed);

  pinMode(RESCUE_PIN, INPUT_PULLUP);

  pinMode(STATUS_LED_PIN, OUTPUT);
  // pinMode(TX_LED_PIN, OUTPUT);

  digitalWrite(STATUS_LED_PIN, HIGH);
  // digitalWrite(TX_LED_PIN, HIGH);

  rescue_mode=(connect_to_wifi()==true)?0:1;   

  if (rescue_mode) {
    start_softap();           // Mode AP
    enable_wifiBlinkTask();   // Clignotement STATUS_LED_PIN 
  }
  else {
    scanner_reseau();
    setup_syslog();         // Recuperation des parametres pour syslog sur serveur externe si parametre
    // enable_checkWifiTask(); // TODO A REMETTRE ? si pas de connection reboot donc perte du temps ! // parametrage de la Task qui appelera check_wifi fonction qui test via ping 8.8.8.8 et renseigne la variable failingWifi = 0 si ok sinon retente la connexion puis reboot en cas d'echec
    internet_ok=check_internet();
    if (inittime()) {       // Mise à l'heure
      digitalWrite(STATUS_LED_PIN, LOW);
      time_set = true;
      enable_initTimeTask(); // initialise de temps en temps l'heure
    }
  }
  
  ws_config(rescue_mode); // Configuration initialisation générale avec les liens des pages => fonctions
  
  prefs_loadprgms(); // charge programmes 
  refresh_programTask(); // lance les taches des programmes  

  // digitalWrite(TX_LED_PIN, LOW);

  write_output_ln("End setup function");
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
      if (inittime()) {
        digitalWrite(STATUS_LED_PIN, LOW);
        write_output_ln("Main.Loop - Réglage de l'heure");
        time_set = true;
      }
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

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

// Variables pour la logique de reconnexion automatique
const unsigned long initial_retry_interval = 120000UL; // 2 minutes en millisecondes
const unsigned long max_retry_interval = 900000UL; // 15 minutes max

// 
bool internet_ok= false;
const long interval = 2000; // Temps appui sur le bouton pour passer en mode rescue
unsigned long previousMillis = 0;
extern bool time_set;
extern SomfyRTS somfy;

// Énumération pour gérer les différents modes de fonctionnement
enum OperatingMode {
  WIFI_OK,          // Connecté au WiFi, fonctionnement normal
  AUTO_RESCUE,      // Mode AP car échec de connexion au démarrage (doit retenter)
  MANUAL_RESCUE     // Mode AP activé par l'utilisateur (ne doit pas retenter)
};

OperatingMode current_mode; // Variable globale pour suivre l'état actuel
int rescue_mode;
unsigned long last_retry_attempt = 0;
unsigned long retry_interval = initial_retry_interval; 

// Fonction qui regroupe les initialisations à faire après une connexion WiFi
void post_wifi_connect_setup() {
    scanner_reseau(); // Utile pour rafraîchir la liste des réseaux pour l'interface web
    internet_ok = check_internet();
    Serial.print("Etat internet : ");
    Serial.println(internet_ok);

    if (inittime()) { // Mise à l'heure
      digitalWrite(STATUS_LED_PIN, LOW); // Éteint la LED si l'heure est OK
    } else {
      settime(0, 0); // on initialise une heure pour éviter les bugs
    }

    if (get_meteo()) { // Lecture de la météo et heure de lever/coucher du soleil
      enable_initMeteoTask(); 
    }
    
    // Tâches récurrentes à lancer en mode connecté
    enable_initTimeTask(); // Initialise de temps en temps l'heure

    // Si la LED clignotait (cas d'une reconnexion depuis AUTO_RESCUE), on l'arrête
    disable_wifiBlinkTask(); 
    digitalWrite(STATUS_LED_PIN, LOW); // Assure que la LED est bien éteinte
}


void setup() {

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
  
  
  // Tentative de connexion au WiFi au démarrage
  write_output_ln("Tentative de connexion au WiFi enregistré...");
  if (connect_to_wifi()) {
    // Le WiFi s'est connecté avec succès au démarrage
    write_output_ln("Connexion WiFi réussie !");
    current_mode = WIFI_OK;
    rescue_mode = 0;
    // Exécute les initialisations nécessaires après une connexion réussie
    post_wifi_connect_setup();
    
  } else {
    // Échec de la connexion WiFi au démarrage, on passe en mode secours automatique
    current_mode = AUTO_RESCUE;
    rescue_mode = 1;
    write_output_ln("Echec de la connexion WiFi. Passage en mode AUTO_RESCUE.");
    
    // Démarrer le point d'accès pour permettre une configuration manuelle
    start_softap();
    
    // Faire clignoter la LED pour indiquer le mode AP
    enable_wifiBlinkTask();
    
    // Initialiser le minuteur pour la première tentative de reconnexion automatique
    last_retry_attempt = millis();
    Serial.printf("Le mode AP est actif. Prochaine tentative de reconnexion auto dans %lu secondes.\n", retry_interval / 1000);
  }
  
  // Configuration du serveur web. Le paramètre indique si on est en mode "rescue" (AP) ou non.
  ws_config(rescue_mode);
  
  // Chargement des programmes mémorisés
  prefs_loadprgms(); 
  refresh_programTask(); // Lance les tâches des programmes

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

    if (current_mode != WIFI_OK) {
      handle_dns_requests();
    } else {
      if (!time_set)  {
        write_output_ln("Main.Loop - L'heure n'est pas définie, j'essaie de l'obtenir...");
        // if (inittime()) {
        //   digitalWrite(STATUS_LED_PIN, LOW);
        //   write_output_ln("Main.Loop - Réglage de l'heure");
        //   time_set = true;
        // }
      }
    }
  
    if (current_mode == AUTO_RESCUE) {
      // On ne tente la reconnexion que si personne n'est connecté au point d'accès
      if (WiFi.softAPgetStationNum() > 0) {
        // Un client est connecté, on réinitialise le minuteur pour attendre qu'il parte
        last_retry_attempt = millis();
      } else if (millis() - last_retry_attempt > retry_interval) {
        write_output_ln("AUTO_RESCUE: Tentative de reconnexion au WiFi...");
        
        WiFi.softAPdisconnect(true); // Arrête le mode AP
        WiFi.mode(WIFI_STA); // Repasse en mode Station
        
        if (connect_to_wifi()) {
          write_output_ln("Reconnexion réussie ! Passage en mode normal.");
          current_mode = WIFI_OK;
          rescue_mode = 0;
          post_wifi_connect_setup(); // Finalise l'initialisation
          ws_config(rescue_mode); 
          retry_interval = initial_retry_interval; // Réinitialise l'intervalle d'attente
        } else {
          write_output_ln("Échec de la reconnexion. Redémarrage du mode AP.");
          start_softap(); // Redémarre le mode AP
          last_retry_attempt = millis(); // Réinitialise le minuteur
          // Optionnel : augmente l'intervalle pour la prochaine fois
          retry_interval = min(retry_interval * 2, max_retry_interval);
          Serial.printf("Prochaine tentative dans %lu secondes.\n", retry_interval / 1000);
        }
      }
    }
  
  // Task scheduler
  execute_runner(); 

// ---- Mode AP si appui sur le bouton pour accèder aux paramètres
  Etat_bouton = digitalRead(RESCUE_PIN);
  if (Etat_bouton == LOW && Dernier_Etat_bouton == HIGH) {
    previousMillis = millis();
    if (current_mode == WIFI_OK) {
      digitalWrite(STATUS_LED_PIN, HIGH);
    }
  }

  // Éteindre la LED si le bouton est relâché et qu'on est en mode normal
  if (Etat_bouton == HIGH && current_mode == WIFI_OK) {
    digitalWrite(STATUS_LED_PIN, LOW);
  }

  // Détection de l'appui long
  if (previousMillis != 0 && Etat_bouton == LOW && Dernier_Etat_bouton == LOW) {
    if ((millis() - previousMillis) >= interval) {
      previousMillis = 0; // Empêche de redéclencher
      
      if (current_mode == WIFI_OK) {
        // Passage en mode rescue manuel
        write_output_ln("Passage en mode MANUAL_RESCUE via le bouton.");
        current_mode = MANUAL_RESCUE;
        rescue_mode = 1;
        internet_ok = false;
        start_softap();
        disable_wifiBlinkTask(); // S'assurer que l'ancienne tâche est arrêtée
        enable_wifiBlinkTask();  // La faire clignoter
        ws_config(rescue_mode);
      } else {
        // Si on est déjà en mode rescue (auto ou manuel), le bouton force un redémarrage
        write_output_ln("Redémarrage demandé via le bouton.");
        ESP.restart();
      }
    }
  }
  Dernier_Etat_bouton = Etat_bouton; // actualisation de l'état du bouton
}

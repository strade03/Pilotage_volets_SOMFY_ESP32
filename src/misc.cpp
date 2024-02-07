
#include "pins.h"
#include "Arduino.h"
#include "pins.h"
#include "SomfyRTS.h"
#include "WiFi.h"
#include "prefs.h"
#include "misc.h"
#include "rs_scheduledtasks.h"


SomfyRTS somfy(RF433_PIN);
// Une instance UDP pour permettre d'envoyer et de recevoir des paquets via UDP
WiFiUDP udpClient;


void write_output_ln(String message){
   Serial.println(message);
}

void moveup(int roller) {
  TXLEDOnfor1s();
  somfy.moveup(roller);
}


void movedown(int roller) {
  TXLEDOnfor1s();
  somfy.movedown(roller);  
}


void prog(int roller) {

  // digitalWrite(TX_LED_PIN, HIGH);
  somfy.prog(roller);
  delay(500);
  // digitalWrite(TX_LED_PIN, LOW);
  
}


void stop(int roller) {
  TXLEDOnfor1s();
  somfy.stop(roller);  
}

/// Controle de l'espace mémoire
// Fonction pour obtenir la taille de la RAM
int getTotalRam() {
  return ESP.getHeapSize();
}

// Fonction pour obtenir la RAM libre
int getFreeRam() {
  return ESP.getFreeHeap();
}

// Fonction pour obtenir la taille de la mémoire flash
int getTotalFlash() {
  return ESP.getFlashChipSize();
}

// Fonction pour obtenir la mémoire flash libre
int getFreeFlash() {
  return ESP.getFreeSketchSpace();
}

// Fonction pour obtenir Vitesse du CPU
int getCpuFrequency() {
  return getCpuFrequencyMhz();
}

// Fonction pour regler Vitesse du CPU
void setCpuFrequency(int f) {
  setCpuFrequencyMhz(f);
}


  
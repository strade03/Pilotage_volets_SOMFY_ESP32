
#include "pins.h"
#include "Arduino.h"
#include "pins.h"
#include "SomfyRTS.h"
#include "Syslog.h"
#include "WiFi.h"
#include "prefs.h"
#include "misc.h"
#include "rs_scheduledtasks.h"


SomfyRTS somfy(RF433_PIN);
// Une instance UDP pour permettre d'envoyer et de recevoir des paquets via UDP
WiFiUDP udpClient;
boolean syslog_working=false;
Syslog syslog(udpClient);


void write_output_ln(String message){
  if(syslog_working){
    syslog.log(LOG_INFO, message);
  }
   Serial.println(message);
}

void setup_syslog(){
  boolean syslog_state = false;
  char syslog_ip[IP_LENGTH];
  int16_t syslog_port;
  syslog_state = prefs_get_syslog_state();
  prefs_get_syslog_ip(syslog_ip);
  syslog_port = prefs_get_syslog_port();

  IPAddress syslog_server_ip;
  if (syslog_state && syslog_server_ip.fromString(syslog_ip)) {
    write_output_ln("MISC - Enabling syslog");
    syslog = Syslog(udpClient, syslog_server_ip, syslog_port, "SomfyRTS", "SomfyRTS", LOG_KERN);
    delay(500);
    syslog_working=true;
    write_output_ln("MISC - setup_syslog done");
  }
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
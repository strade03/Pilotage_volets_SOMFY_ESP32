#include "prgm.h"
#include "prefs.h"
#include "misc.h"

extern prgm_t * prgms;
extern const char *remote_name;

int getavailableprgm(void) {
  for (int i=0; i < PRGM_COUNT ; i++){
    if(prgms[i].used!=true){
      return i;
    }
  }
  return -1;
}


bool addprgm(uint8_t roller,uint8_t hour, uint8_t minute, uint8_t command) {
   
  int available_prgm = getavailableprgm();
  if (available_prgm == -1) {
    return false; //No slot available
  }

  prgms[available_prgm].roller = roller;
  prgms[available_prgm].actif = 1;
  prgms[available_prgm].hour = hour;
  prgms[available_prgm].minute = minute;
  prgms[available_prgm].command = command;
  prgms[available_prgm].used = true;

  prefs_storeprgms();
  
  return true;
}

bool updateprgm(int available_prgm, uint8_t roller,uint8_t hour, uint8_t minute, uint8_t command) { //  Modifier un programme

  if (prgms[available_prgm].used==true) {
    prgms[available_prgm].roller = roller;
    prgms[available_prgm].hour = hour;
    prgms[available_prgm].minute = minute;
    prgms[available_prgm].command = command;

    prefs_storeprgms();
    return true;
  }
  return false; //No slot available
}

bool delprgm(uint8_t prgm) {

  if (prgm >= PRGM_COUNT) {
    return false;
  }
  prgms[prgm].used = false;

  prefs_storeprgms();
  
  return true;
}



bool getprgm(uint8_t prgm, uint8_t * actif,uint8_t * roller, uint8_t * hour, uint8_t * minute, uint8_t * command) {

  // Serial.print("PRGM - getprgm - Loading prgm : ");
  // Serial.println(prgm);
  // Serial.print("PRGM - getprgm - Used : ");
  // Serial.println(prgms[prgm].used);
  
  if (prgm >= PRGM_COUNT || prgms[prgm].used ==false) {
    return false;
  }  
  
  *roller = prgms[prgm].roller;
  *actif = prgms[prgm].actif;
  *hour = prgms[prgm].hour;
  *minute = prgms[prgm].minute;
  *command = prgms[prgm].command;

  return true;
}


void checkprgms(int cur_h, int cur_m) {

  uint8_t actif = 1;
  uint8_t roller = 0;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t command = 0;

  // Serial.print("PRGM - checkprgms - Current hour : ");
  // Serial.println(cur_h);
  // Serial.print("PRGM - checkprgms - Current minute : ");
  // Serial.println(cur_m);

  for (int i=0; i<PRGM_COUNT; i++) {
    /*result = getprgm(i, &roller, &hour, &minute, &command);
    // Serial.print("PRGM - checkprgms - Prgm hour : ");
    // Serial.println(hour);
    // Serial.print("PRGM - checkprgms - Prgm minute : ");
    // Serial.println(minute);*/

    if (getprgm(i, &actif, &roller, &hour, &minute, &command)) { // Si l'heure du programme et actif on lance le programme
      if (actif==1 && cur_h==hour && cur_m==minute) {
        // Serial.print("PRGM - checkprgms - Prgm matches : ");
        // Serial.println(i);
        if (command==0) {
          write_output_ln("PRGM - checkprgms - Closing ");
          movedown(roller);

        }
        else if (command==1)
        {
          write_output_ln("PRGM - checkprgms - Opening ");
          moveup(roller);
        }    
      }
#ifdef delay_after_move
      delay(3000);
#endif
    }
  }

}

int getprgmcount(void) {
  int cnt = 0;
  for (int i=0; i < PRGM_COUNT ; i++){
    if(prgms[i].used==true){
      cnt++;
    }
  }
  return cnt;
}

// Active ou désactive le programme sans le supprimer (bouton switch activation sur la page web)
bool activeprgm(uint8_t prgm,  uint8_t actif) {
  
  // Serial.print("PRGM - activeprgm - Loading prgm : ");
  // Serial.println(prgm);
  // Serial.print("PRGM - activeprgm - Actif : ");
  // Serial.println(prgms[prgm].actif);
  
  if (prgm >= PRGM_COUNT || prgms[prgm].used ==false) {
    return false;
  }  
  
  prgms[prgm].actif=actif;
  prefs_storeprgms();
  return true;
}
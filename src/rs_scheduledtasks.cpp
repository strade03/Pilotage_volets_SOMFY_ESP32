

#include "TaskScheduler.h"
#include "prgm.h"
#include "pins.h"
#include "rs_scheduledtasks.h"
#include "rs_time.h"
#include "rs_wifi.h"
#include "misc.h"

// Timer/Scheduler
Scheduler runner;
// Vérification si un programme démarre (toutes les minutes)
Task programTask(TASK_MINUTE, TASK_FOREVER, &checkTriggerProgram, &runner, false); 
// Vérification Wifi (toutes les heures)
Task checkWifiTask(TASK_HOUR, TASK_FOREVER, &check_wifi, &runner, false);
// Clignotement LED Wifi mode paramétrage (toutes les secondes)
Task wifiBlinkTask(1 * TASK_SECOND, TASK_FOREVER, &wifiLEDOn, &runner, false);
// TX LED des tâches Allume la LED pendant 1 sec lors de l'execution de la tache
Task TXLEDTask(1 * TASK_SECOND, TASK_FOREVER, &TXLEDOff, &runner, false);
// Ajustement de l'heure une fois par jour
Task initTimeTask(24 * TASK_HOUR, TASK_FOREVER, &inittimeTask, &runner, false);
// Lecture de la méteo toutes les heures
Task initMeteoTask( TASK_HOUR, TASK_FOREVER, &initmeteoTask, &runner, false);

void checkTriggerProgram(){

  write_output_ln("Main - checkTriggerProgram - Start");

  uint8_t hour=0;
  uint8_t minute=0;
  
  gettime(&hour, &minute);
  checkprgms(hour, minute);
}

void wifiLEDOn(void) {
  digitalWrite(STATUS_LED_PIN, HIGH);
  wifiBlinkTask.setCallback(&wifiLEDOff);
}

void wifiLEDOff(void) {
  digitalWrite(STATUS_LED_PIN, LOW);
  wifiBlinkTask.setCallback(&wifiLEDOn);
}

void enable_wifiBlinkTask(void) {
  wifiBlinkTask.enable();
}

void disable_wifiBlinkTask(void) {
  wifiBlinkTask.disable();
}

void inittimeTask(void) {
  inittime();
}
void enable_initTimeTask(void) {
  initTimeTask.enable();
}

void disable_initTimeTask(void) {
  initTimeTask.disable();
}

void initmeteoTask(void) {
  get_meteo();
}
void enable_initMeteoTask(void) {
  initMeteoTask.enable();
}

void disable_initMeteoTask(void) {
  initMeteoTask.disable();
}

void enable_checkWifiTask(void) {
  checkWifiTask.enable();
}

void refresh_programTask(void) {
  write_output_ln("rs_scheduledtasks - refresh_programTask - getprgmcount : ");
  write_output_ln(String(getprgmcount()));
  if(getprgmcount() > 0){
    write_output_ln("rs_scheduledtasks - refresh_programTask - Enabling programTask");
    programTask.enable();
  }
  else {
    write_output_ln("rs_scheduledtasks - refresh_programTask - Disabling programTask");
    programTask.disable();
  }
}

void TXLEDOnfor1s(void) {
  // Cancel task in case already running
  TXLEDTask.cancel();
  TXLEDTask.setCallback(&TXLEDOff);
  TXLEDTask.enableDelayed(TASK_SECOND);
  // digitalWrite(TX_LED_PIN, HIGH);
}

void TXLEDOff(void) {
  // digitalWrite(TX_LED_PIN, LOW);
  TXLEDTask.cancel();
}

void execute_runner(void) {
  runner.execute();
}


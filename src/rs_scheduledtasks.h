#ifndef SCHEDULEDTASKS_H_
#define SCHEDULEDTASKS_H_

// Scheduler variables
#define _TASK_SLEEP_ON_IDLE_RUN

void checkTriggerProgram();
void wifiLEDOn();
void wifiLEDOff();
void enable_wifiBlinkTask(void);
void disable_wifiBlinkTask(void);
void enable_checkWifiTask(void);
void inittimeTask(void);
void disable_initTimeTask(void);
void enable_initTimeTask(void);
void initmeteoTask(void);
void disable_initMeteoTask(void);
void enable_initMeteoTask(void);
void refresh_programTask(void);
void TXLEDOnfor1s(void);
void TXLEDOff(void);
void execute_runner(void);

#endif //SCHEDULEDTASKS_H_
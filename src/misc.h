#ifndef MISC_H_
#define MISC_H_

#include "Arduino.h"

#define VERSION "Version 1.5"

void write_output_ln(String message);
void wifiblink(void);
void moveup(int roller);
void movedown(int roller);
void prog(int roller);
void stop(int roller);
unsigned int getRolling(int roller);
void setRolling(int roller, unsigned int rolling);

int getTotalRam();
int getFreeRam();
int getTotalFlash();
int getFreeFlash();
int getCpuFrequency();
void setCpuFrequency(int f);

extern int rescue_mode; 

#endif //MISC_H_
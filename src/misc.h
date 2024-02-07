#ifndef MISC_H_
#define MISC_H_

#include "Arduino.h"

#define VERSION "V1.0"

void write_output_ln(String message);
void wifiblink(void);
void moveup(int roller);
void movedown(int roller);
void prog(int roller);
void stop(int roller);

int getTotalRam();
int getFreeRam();
int getTotalFlash();
int getFreeFlash();
int getCpuFrequency();
void setCpuFrequency(int f);


#endif //MISC_H_
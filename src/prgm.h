#ifndef PRGM_H_
#define PRGM_H_

#include "Arduino.h"

#define OPEN true
#define PRGM_COUNT 30

typedef struct  {
  public:
    uint8_t used = false; // False => programme libre non utilisée
    uint8_t actif = false; // True le prog est actif
    uint8_t roller = 0; // N° volet
    uint8_t hour = 0;
    uint8_t minute = 0;    
    uint8_t command = false; // 0 fermer, 1 ouvrir
}prgm_t;

int getavailableprgm(void);
bool addprgm(uint8_t roller, uint8_t hour, uint8_t minute, uint8_t command);
bool updateprgm(int available_prgm, uint8_t roller,uint8_t hour, uint8_t minute, uint8_t command);
bool delprgm(uint8_t prgm);
bool getprgm(uint8_t prgm, uint8_t * actif, uint8_t * roller, uint8_t * hour, uint8_t * minute, uint8_t * command);
void checkprgms(int cur_h, int cur_m);
int getprgmcount(void);
bool activeprgm(uint8_t prgm,  uint8_t actif);
#endif //PRGM_H_

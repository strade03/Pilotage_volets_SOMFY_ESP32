#ifndef RS_TIME_H_
#define RS_TIME_H_

bool inittime(void);
bool gettime(uint8_t * hours, uint8_t * minutes);
bool settime(uint8_t hours, uint8_t minutes);


bool get_meteo(); // Lecture des données méteo et lever  et coucher du soleil

uint8_t minute_soleil(String type);
uint8_t heure_soleil(String type);
String getWindDirection(float deg);

#endif /* RS_TIME_H_ */

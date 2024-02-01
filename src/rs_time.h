#ifndef RS_TIME_H_
#define RS_TIME_H_

bool inittime(void);
bool gettime(uint8_t * hours, uint8_t * minutes);
bool getfulltime(uint8_t * hours, uint8_t * minutes,uint8_t * secondes);
bool settime(uint8_t hours, uint8_t minutes);



#endif /* RS_TIME_H_ */

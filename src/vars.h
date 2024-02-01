#ifndef VARS_H_
#define VARS_H_

#define REMOTES_COUNT 11

const char *remote_name[REMOTES_COUNT] = { 
  (char *)"Coin Jour",        // 0
  (char *)"Sud",              // 1
  (char *)"Coin Nuit",        // 2
  (char *)"Bureau",           // 3
  (char *)"Cuisine",          // 4
  (char *)"Salle",            // 5
  (char *)"Salon",            // 6
  (char *)"Chambre Parents",  // 7
  (char *)"Chambre Camille",  // 8
  (char *)"Chambre Emile",    // 9
  (char *)"Cuisine Porte F."  // 10
};

const int remote_order[REMOTES_COUNT] = { 0,1,2,5,6,3,4,10,7,8,9 };

#endif /* VARS_H_ */
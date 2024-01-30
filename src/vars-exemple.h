// Exemple de fichier vars.h 
#ifndef VARS_H_
#define VARS_H_

// #define delay_after_move // Si actif fait une pause entre les programmes

#define REMOTES_COUNT 10

const char *remote_name[REMOTES_COUNT] = { 
  (char *)"Cuisine", 
  (char *)"Baie vitr&eacute;e",
  (char *)"Buffet",
  (char *)"Porte fen&ecirc;tre",
  (char *)"Bureau",
  (char *)"Chambre 1",
  (char *)"Chambre 2",
  (char *)"Chambre 3",
  (char *)"Tout bas",
  (char *)"Tout haut",
};

// Ordre d'affichage des télécommandes permettant d'en ajouter facilement 
const int remote_order[REMOTES_COUNT] = { 0,1,2,5,6,3,4,7,8,9 };

#endif /* VARS_H_ */
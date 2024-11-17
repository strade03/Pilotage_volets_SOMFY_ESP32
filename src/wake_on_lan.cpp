#include "wake_on_lan.h"
#include <ctype.h>  // Pour la fonction isxdigit()
// #include <cstring> // Pour strlen et strcpy

// Fonction pour supprimer les espaces d'une chaîne
void cleanMac(char* str) {
    int length = strlen(str); 
    int j = 0;  

    // Parcourir chaque caractère de la chaîne d'origine
    for (int i = 0; i < length; i++) {
        if (str[i] != ':') {  // Si le caractère n'est pas un espace
            str[j++] = str[i];  // Déplacer le caractère dans la position appropriée
        }
    }

    // Ajouter le caractère nul à la fin pour marquer la fin de la nouvelle chaîne
    str[j] = '\0';
}

bool isValidMacAddress(const char* mac) {
     // Vérifier si la chaîne est exactement de 12 caractères
    if (strlen(mac) != 12) {
        return false;
    }

    // Vérifier chaque caractère : il doit être un caractère hexadécimal
    for (int i = 0; i < 12; i++) {
        if (!isxdigit(mac[i])) {  // isxdigit() vérifie si le caractère est hexadécimal
            return false;
        }
    }
    return true;
}

void sendWOL(char* mac,char* serveur_ip) {
    WiFiUDP udp;
    // Traitement de l'adresse MAC qui est une chaine de 12 caratères
    byte macBytes[6];
       
    size_t macLength = strlen(mac);
    char macLocal[macLength + 1];  
    strcpy(macLocal, mac);
    cleanMac(macLocal);
    if (isValidMacAddress(macLocal)) {
        // Conversion des 12 caractères hexadécimaux en 6 octets
        for (int i = 0; i < 6; i++) {
            char byteStr[3]; // Pour stocker 2 caractères hexadécimaux + '\0'
            byteStr[0] = macLocal[i * 2];     // Premier caractère de l'octet
            byteStr[1] = macLocal[i * 2 + 1]; // Deuxième caractère de l'octet
            byteStr[2] = '\0';           // Terminateur de chaîne
            macBytes[i] = (byte)strtol(byteStr, NULL, 16); // Convertir en octet
        }

        // préparation de l'adresse IP
        int a, b, c, d;
        sscanf(serveur_ip, "%d.%d.%d.%d", &a, &b, &c, &d);
        IPAddress ip(a, b, c, d); 

      // routine WOL 
        udp.beginPacket(ip, 9); // Adresse de diffusion pour le réseau voir 250 ou 255
        for (int i = 0; i < 6; i++) {
            udp.write(0xFF); // 6 fois 0xFF
        }
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 6; j++) {
                udp.write(macBytes[j]); // Envoie l'adresse MAC 16 fois
            }
        }
        udp.endPacket();
    }
}

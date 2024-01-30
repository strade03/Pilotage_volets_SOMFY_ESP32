La base du projet provient :
https://github.com/Tahitibob35/roller-shutter
Des améliorations et adaptations concernent principalement les paramètrages Wifi, IP et comptes.

# Volet roulant
Volet roulant Somfy piloté par un ESP32

# Utilisation
## Démarrage 

Pendant la phase de d'initialisation la LED reste allumée. Elle s'éteindra en mode normal ou clignotera en mode secours.

## Premiere utilisation ou mode rescue

L'ESP32 démarre en mode secours en tant que point d'accès s'il ne peut pas se connecter à une borne Wifi, la LED clignote pour indiquer le mode secours 

* Access point : "VoletsRoulants"

* Default IP : 10.10.10.1

Configuration URL : http://10.10.10.1

Identifiant et Mot de passe par defaut : pas de mot de passe et d'identifiant en mode rescue.


## Mode normal

Pour passer en mode secours quand il est en mode normal, appuyer sur le bouton 5sec la LED clignote pour indiquer le mode secours.
Pour vérifier que l'ESP32 est bien en fonction, vous pouvez appuyer sur le bouton la LED s'allumera.

* Page de l'application URL : http://X.X.X.X => X.X.X.X sera l'adresse attribuée par DHCP ou choisie dans les paramètrages.

# Mise en oeuvre 
## Plan de montage 

![Schema de montage ESP32](/images/Schema_de_montage_ESP32.jpg)

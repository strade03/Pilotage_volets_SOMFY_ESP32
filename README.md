La base du projet provient :
https://github.com/Tahitibob35/roller-shutter

Des améliorations et adaptations concernent principalement les paramètrages Wifi, IP, comptes et gestion des programmes.

# Pilotage Volets roulants système RTS

Volet roulant Somfy RTS piloté par un ESP32.
(Ne fonctionne pas avec la technologie io de Somfy.) 

# Utilisation
## Démarrage 

Pendant la phase de d'initialisation la LED reste allumée. Elle s'éteindra en mode normal ou clignotera en mode secours.


## Premiere utilisation ou mode rescue

L'ESP32 démarre en mode secours en tant que point d'accès s'il ne peut pas se connecter à une borne Wifi, la LED clignote pour indiquer le mode secours. 

* Access point : "VoletsRoulants"

* Configuration URL : http://10.10.10.1

Identifiant et Mot de passe par defaut : pas de mot de passe et d'identifiant en mode rescue. 


## Associer les télécommandes virtuelles aux volets
![Telecommande](/images/telecommande.jpg)

Sur une télécommande déjà associée à un volet roulant, appuyez 3 secondes sur le bouton "Prog" à l'arrière. Le volet va descendre et monter.
A partir de la page Attacher du menu de configuration, choisir le volet que vous voulez piloter, le volet va descendre et monter, l'association est terminée.

>Si jamais vous vouliez annuler l'association, répéter l'opération, la télécommande virtuelle ne sera liée au volet.
>Vous pouvez associer plusieurs volets à une télécommande virtuelle il suffit de répéter l'opération pour chacun des volets en choisissant la même commande virtuelle.
>Ainsi, avec un clic vous pouvez piloter plusieurs volets !


## Mode normal

>Pour passer en mode secours quand il est en mode normal, appuyer sur le bouton 5 sec la LED clignote pour indiquer le mode secours.

>Pour vérifier que l'ESP32 est bien en fonction, vous pouvez appuyer sur le bouton la LED s'allumera.
Si l'identification est activée dans les paramètres, il faudra se logguer pour se connecter.

* Page de l'application URL : http://X.X.X.X => X.X.X.X sera l'adresse attribuée par DHCP ou choisie dans les paramètrages.

# Mise en oeuvre 
## Matériel 
ESP32

![ESP32](/images/esp32.png)

RF433 module

![RF433](/images/rf433-module.jpg)

433.42MHz oscillateur

![oscillateur](/images/oscillator.jpg)


## Plan de montage 

Remplacer l'oscillateur du module rf433 par celui en 433.42Mhz.

![Schema de montage ESP32](/images/Schema_de_montage_ESP32.jpg)

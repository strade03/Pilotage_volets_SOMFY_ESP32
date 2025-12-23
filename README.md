<!-- language-all: lang-html -->

La base du projet provient :
https://github.com/Tahitibob35/roller-shutter

Des améliorations et adaptations concernent principalement les paramétrages Wifi, IP, comptes et gestion des programmes (lever coucher du soleil).

<p align="center" width="100%"><img  src="/images/by-nc-sa.png"></p>

# Pilotage Volets roulants système RTS

Volets roulants Somfy RTS pilotés par un ESP32.
(Ne fonctionne pas avec la technologie io de Somfy.) 

# **Utilisation**
## Démarrage 

Pendant la phase de d'initialisation la LED reste allumée. Elle s'éteindra en mode normal ou clignotera en mode secours.


## Première utilisation ou mode secours

L'ESP32 démarre en mode secours en tant que point d'accès s'il ne peut pas se connecter à une borne Wifi, la LED clignote pour indiquer le mode secours. 

* Access point : "VoletsRoulants"

* Configuration URL : http://10.10.10.1

Identifiant et Mot de passe par défaut : pas de mot de passe et d'identifiant en mode secours. 


## Associer les télécommandes virtuelles aux volets
![Telecommande](/images/telecommande.jpg)

Sur une télécommande déjà associée à un volet roulant, appuyez 3 secondes sur le bouton "Prog" à l'arrière. Le volet va descendre et monter.
A partir de la page Attacher du menu de configuration, choisir le volet que vous voulez piloter, le volet va descendre et monter, l'association est terminée.

>Si jamais vous vouliez annuler l'association, répéter l'opération, la télécommande virtuelle ne sera plus liée au volet.
>Vous pouvez associer plusieurs volets à une télécommande virtuelle il suffit de répéter l'opération pour chacun des volets en choisissant la même commande virtuelle.
>Ainsi, avec un clic vous pouvez piloter plusieurs volets !


## Mode normal

>Pour passer en mode secours quand il est en mode normal, appuyer sur le bouton 5 sec la LED clignote pour indiquer le basculement en mode secours.

>Pour vérifier que l'ESP32 est bien en fonction, vous pouvez appuyer sur le bouton la LED s'allumera.
Si l'identification est activée dans les paramètres, il faudra se loguer pour se connecter.

* Page de l'application URL : http://X.X.X.X => X.X.X.X sera l'adresse attribuée par DHCP ou choisie dans les paramétrages.

Pour accéder au module de l'extérieur du réseau vous devrez sur votre box rediriger un port sur IP port 80 du module. 
A partir d'une freebox vous avez également la possibilité d'obtenir un nom de domaine pour plus de facilité.

**Attention, la recherche des bornes Wifi est effectuée au demarrage de l'ESP32.**
> Si vous ne voyez pas votre borne tenter de le redemarrer ou saisir à la main les informations.

## API 

> Il est possible d'utliser l'API si un token est défini dans la page de configuration Options avancées
Exemple d'appel :
http://X.X.X.X/api?roller=3&command=0&token=1234

roller : numéro de la télécommande virtuelle 0 étant la première
command : 0 ferme, 1 ouvre, 2 arrête
token : valeur du token


# **Mise en oeuvre** 
## Matériel 
ESP32

![ESP32](/images/esp32.png)

RF433 module

![RF433](/images/rf433-module.jpg)

433.42MHz oscillateur

![oscillateur](/images/oscillator.jpg)

Antenne (longueur quart d'onde 17cm env.)

![antenne](/images/antenne.jpg)

## Plan de montage 

Remplacer l'oscillateur du module rf433 par celui en 433.42Mhz.

![Schema de montage ESP32](/images/Schema_de_montage_ESP32.jpg)


# Produit fini 

L'électronique a été réalisé sur des cartes de prototypage ce qui permet de réduire l'encombrement.

- Electronique

![bouton](/images/boite/electronique1.jpg)

- Voici la réalisation dans sa boite, le fichier 3D est disponible.

![boite](/images/boite/boite1.jpg)

![boite](/images/boite/boite2.jpg)

![boite](/images/boite/boite3.jpg)

![boite](/images/boite/boite4.jpg)

![boite](/images/boite/boite5.jpg)

# Compilation

* vars.h 

Renommer le fichier vars-exemple.h en vars.h
renseigner le fichier en nommant vos télécommandes et définir l'ordre d'affichage.
 

# Captures d'écran

<p align="center" width="100%"><img  src="/images/captures/home.jpg"></p>

<p align="center" width="100%"><img  src="/images/captures/telecommandes.jpg"></p>

<p align="center" width="100%"><img  src="/images/captures/programmes.jpg"></p>

<p align="center" width="100%"><img  src="/images/captures/configuration.jpg"></p>


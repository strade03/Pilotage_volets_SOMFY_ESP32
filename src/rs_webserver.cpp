#include "esp_task_wdt.h"
#include "vars.h"
#include "ESPAsyncWebServer.h"

#include "rs_webserver.h"
#include "prgm.h"
#include "prefs.h"
#include "misc.h"
#include "rs_time.h"
#include "misc.h"
#include "rs_scheduledtasks.h"
#include "rs_web_header_footer.h"
#include "SomfyRTS.h"
#include "rs_wifi.h"
#include "wake_on_lan.h"

char key[10];
char token[16]; // Token pour valider l'appel de l'API

AsyncWebServer server(80);
extern prgm_t prgms[PRGM_COUNT];

extern int rescue_mode;
extern bool internet_ok;

extern bool time_set;

extern String Liste_reseau;

extern SomfyRTS somfy;
//------------------------------------------------------------------------------------------------------------------------------------
// Page non trouvée => erreur 404
//------------------------------------------------------------------------------------------------------------------------------------
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


//------------------------------------------------------------------------------------------------------------------------------------
// Identification 
//------------------------------------------------------------------------------------------------------------------------------------

void identification(AsyncWebServerRequest *request) {
  char admin_mdp[PASSWORD_LENGTH];
  prefs_get_str("admin","mdp",admin_mdp,PASSWORD_LENGTH,"admin");
  char admin_id[PASSWORD_LENGTH];
  prefs_get_str("admin","identifiant",admin_id,PASSWORD_LENGTH,"admin");
  boolean admin_actif=prefs_get_bool("admin","active",false);
  if ((admin_actif==true) and (!rescue_mode))
    if(!request->authenticate(admin_id,admin_mdp))
        return request->requestAuthentication();
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page paramètres reboot
//------------------------------------------------------------------------------------------------------------------------------------
void rebootESP32(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_GET) {
      //request->send(200, "text/html, charset=UTF-8", "reboot en cours...");
      request->redirect("/");
      ESP.restart();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Correction chaine texte pour html
//------------------------------------------------------------------------------------------------------------------------------------
String htmlEntities(String str, bool whitespace) {
  str.replace("&","&amp;");
  str.replace("<","&lt;");
  str.replace(">","&gt;");
  if(whitespace) str.replace(" ","&#160;");
  // str.replace("-","&ndash;");
  // str.replace("\"","&quot;");
  // str.replace("/": "&#x2F;");
  // str.replace("`": "&#x60;");
  // str.replace("=": "&#x3D;");
return str;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Footer avec heure, état internet et usage mémoire
//------------------------------------------------------------------------------------------------------------------------------------
String baspage(){
  // if ((!rescue_mode) && (time_set==true)){
  if (!rescue_mode) {
  uint8_t hour=0;
  uint8_t minute=0;
  String page;
  gettime(&hour, &minute);
  page += R"rawliteral(<footer class="w3-container w3-teal w3-xlarge w3-theme w3-display-bottommiddle" style="width:100%; position:fixed; bottom:0; display: flex; justify-content: space-between;">
    <div class="w3-half w3-left-align">
      Heure ESP32 : )rawliteral";
  page+= timeToStr(hour,minute);
  page += R"rawliteral(</div>
    <div class="w3-half w3-center">)rawliteral";
    page +="<small>Free RAM: "+ String((100*getFreeRam()/getTotalRam()))+"%"+" Flash: "+String((100*getFreeFlash()/getTotalFlash()))+"% CPU: "+String(getCpuFrequency())+"</small>";
    page += R"rawliteral(</div>
    <div class="w3-half w3-right-align">
      Etat Internet <span class="w3-circle  )rawliteral";
  page+=(internet_ok==false)?"w3-red ":"w3-teal ";
  page+=R"rawliteral(w3-large" id="etatConnexion">&nbsp;&nbsp;&nbsp;&nbsp;</span>
    </div></footer>)rawliteral";
  return(page);
  }
  return("");
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Menu d'accueil
//------------------------------------------------------------------------------------------------------------------------------------
void handleMain(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleMain - Page d'accueil");

  String page;
  page = HEADER ;
  if (internet_ok==true)
      page= page + STYLE_w3 + STYLE_Meteo + HEADER_close;
    else
      page= page + STYLE_w3_light + STYLE_Meteo + HEADER_close;
  char ip_serve[18];
  char adr_mac[20];
  prefs_get_str("wol","adr_mac",adr_mac,20,"");
  prefs_get_str("wol","ip_serve",ip_serve,18,"");
  //if (rescue_mode==1) {
  page+=R"rawliteral(<header class="w3-container w3-card w3-theme">
  <h1>Gestion Volets Roulants</h1>
  </header>
  <div class="w3-container" style="margin-bottom:50px">
  <br/>
  <a href="command" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Télécommandes</a><br/>
  <a href="prgmlist" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Programmation</a><br/>)rawliteral";
  if (strlen(ip_serve) > 0 && strlen(adr_mac) > 0) {
    page+=R"rawliteral(<a href="actionwol" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Wake On LAN Serveur</a><br/>)rawliteral";
  }
  page+=R"rawliteral(<br/>
  <a href="config" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Configurer</a><br/>  
  )rawliteral";


  if (!rescue_mode){
    char sunrise[12] = "";
    prefs_get_str("meteo","sunrise",sunrise,12,"");
    char sunset[12] = "";
    prefs_get_str("meteo","sunset",sunset,12,"");
    
    extern String   temperature;
    extern String  pressure;
    extern String  humidity;
    extern String  visibility;
    extern String   win_speed;
    extern String  win_deg;
    extern String icon;
    
  page += R"rawliteral(<table class="w3-table meteo">
  <tr><td class="w3-theme w3-card"> 
  <img src=")rawliteral";
  page+="https://openweathermap.org/img/wn/"+icon+".png\" alt=\"meteo\">";
  // page+="https://openweathermap.org/img/wn/"+icon+"@2x.png\" alt=\"meteo\">"; // Icone plus grande
  page += "&#x1F321;&nbsp;"+ temperature+"<small>°</small></td>\
    <td class=\"w3-theme w3-card\">&#x1F4A8;&nbsp;"+win_speed+ "<small>m/s</small><br>&#129517;&nbsp;<b>"+getWindDirection(win_deg.toFloat())+"</b></td>\
    </tr><tr><td class=\"w3-theme w3-card\">Pression<br>"+pressure+"<small>hPa</small></td>\
    <td class=\"w3-theme w3-card\">Humidité<br>"+humidity+"<small>%</small> </td></tr>\
    <tr><td class=\"w3-theme w3-card\">&#x1F506; lever: "+ timeToStr("lever") +"<br>&#x1F506; coucher: "+ timeToStr("coucher") +"</td>\
    <td class=\"w3-theme w3-card\">&#x2606 jour: "+ timeToStr("jour")+"<br>&#x2606 nuit: "+ timeToStr("nuit")+" </td></tr></table>";

  page += "</td></tr></table>";
  }
  page += "</div>";

  page +=baspage();
  page+="</body></html>";

  
// TODO si mode rescue_mode on annule la task de reboot automatique comme la page c'est affiché

  request->send(200, "text/html", page);
}


//------------------------------------------------------------------------------------------------------------------------------------
// Page Menu de Configuration
//------------------------------------------------------------------------------------------------------------------------------------
void handleConfig(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleConfig - Page configuration");

  String page;
  page = HEADER ;
  if (internet_ok==true)
      page= page + STYLE_w3 + HEADER_close;
  else
      page= page + STYLE_w3_light + HEADER_close;
  //if (rescue_mode==1) {
  page+=R"rawliteral(<header class="w3-container w3-card w3-theme">
  <h1>Configuration</h1>
  </header>
  <div class="w3-container"> 
  <br/>
  <a href="attach" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Appairer télécommande</a><br>
  <a href="wifi" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Wifi</a><br>
  <a href="reseau" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Réseau</a><br>
  <a href="securite" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Sécurité</a><br>
  <a href="clock" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Heure</a><br>
  <a href="application" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Options avancées</a><br>
  <a href="wol" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Option Wake On Lan</a><br>
  <br>)rawliteral";
  page+=R"rawliteral(<a href="debug" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Mode debug telecommande</a><br>
  <br>)rawliteral";
  page+=R"rawliteral(<a href="\" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Retour</a><br/>
  <a href="reboot" class="w3-button w3-red w3-xxlarge w3-round-large w3-block"><b>Reboot</b></a><br>
  )rawliteral";
  //}
  
  page +=baspage();
  page+=  FOOTER;

  request->send(200, "text/html", page);
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Commande = télécommandes virtuelles => commandes
//------------------------------------------------------------------------------------------------------------------------------------
void handleCommand(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleCommand - Serving command page");
  // Traitement de la commande roller=1&command=2 (N° volet, Commande 0 descendre, 1: monte, 2: Stop/My)
  if (request->hasArg("roller") && request->hasArg("command")) {
    String roller_str = request->arg("roller");
    String command_str = request->arg("command");
    int roller = roller_str.toInt();
    int command = command_str.toInt();
    if (command == 0) {
        write_output_ln("WEBSERVER - handleCommand - Descente Volet : " + String(roller));
        movedown(roller);
      }
      else if (command == 1) {
        write_output_ln("WEBSERVER - handleCommand - Monte Volet : " + String(roller));
        moveup(roller);
      }
      else if (command == 2) {
        write_output_ln("WEBSERVER - handleCommand - Stop Volet : " + String(roller));
        stop(roller);
    }
    redirect(request,(char*)"/command"); // rappel de la fonction pour afficher la page
    return;
  }
  
  if (request->method() == HTTP_GET) { 
    // String page;
    // page= String(HEADER) ;
    //page= page + STYLE_w3 +STYLE_Commande ;
    //page= page + String(STYLE_w3) ;
    // page= page + String(STYLE_Commande) ;
    String page1=R"rawliteral(<div class="w3-container"><table  class="w3-table commandes">)rawliteral";

    // Boucle sur les commandes 
    for (size_t i = 0; i < REMOTES_COUNT; i++)
    {
      String val_i=String(remote_order[i]);
      page1+=R"rawliteral(<tr><td><header class="w3-container w3-card w3-theme"><h1>)rawliteral";
      page1+=String(remote_name[remote_order[i]]);
      page1+=R"rawliteral(</h1></header></td> 
            <td><a href="command?roller=)rawliteral";
      page1+=val_i;
      page1+=R"rawliteral(&command=0" class="w3-button w3-red w3-xlarge w3-round-large" style="width:100%"><span><div class="tourne90">&#10144;</div></span></a></td>
            <td><a href="command?roller=)rawliteral";
      page1+=val_i;
      page1+=R"rawliteral(&command=2" class="w3-button w3-grey w3-xlarge w3-round-large" style="width:100%"><span style="color:white;">&#9634;</span></a></td>
            <td><a href="command?roller=)rawliteral";
      page1+=val_i;
      page1+=R"rawliteral(&command=1" class="w3-button w3-teal w3-xlarge w3-round-large" style="width:100%"><span><div class="tourne270">&#10144;</div></span></a></td></tr>)rawliteral";
     
    }
    size_t totalLength;
    if (internet_ok==true)
      totalLength = strlen(HEADER) + strlen(STYLE_w3) + strlen(STYLE_Commande) + page1.length() +  150;      
    else
      totalLength = strlen(HEADER) + strlen(STYLE_w3_light) + strlen(STYLE_Commande) + page1.length() +  150;      

    char* lapage = new char[totalLength];
    memset(lapage, 0, totalLength);
    strcat(lapage, HEADER);
    if (internet_ok==true)
      strcat(lapage, STYLE_w3);
    else
      strcat(lapage, STYLE_w3_light);
    strcat(lapage, STYLE_Commande);
    strcat(lapage, page1.c_str());
    strcat(lapage, "</table><br><a href=\"\\\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a></div>");
    String tmp= String(lapage);
    tmp+=baspage();
    tmp+=FOOTER;
    // strcat(lapage, FOOTER);
    // Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    // request->send(200, "text/html",  String(lapage));
    request->send(200, "text/html",  tmp);
    delete[] lapage;
   }      
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page DEBUG Commande = télécommandes virtuelles => commandes Affiche la valeur du Rolling que l'on peut modifier
//------------------------------------------------------------------------------------------------------------------------------------
void handleDebug(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleDebug - Serving command page");
  // Traitement de la commande roller=1&command=2 (N° volet, Commande 0 descendre, 1: monte, 2: Stop/My)
  if (request->hasArg("roller") && request->hasArg("command")) {
    String roller_str = request->arg("roller");
    String command_str = request->arg("command");
    int roller = roller_str.toInt();
    int command = command_str.toInt();
    if (command == 0) {
        write_output_ln("WEBSERVER - handleDebug - Descente Volet : " + String(roller));
        movedown(roller);
      }
      else if (command == 1) {
        write_output_ln("WEBSERVER - handleDebug - Monte Volet : " + String(roller));
        moveup(roller);
      }
      else if (command == 2) {
        write_output_ln("WEBSERVER - handleDebug - Stop Volet : " + String(roller));
        stop(roller);
    }
    redirect(request,(char*)"/debug"); // rappel de la fonction pour afficher la page
    return;
  }  else if (request->hasArg("roller") && request->hasArg("rolling_value")) {
        String roller_str = request->arg("roller");
        String rollingVal = request->arg("rolling_value");
        int roller = roller_str.toInt();
        unsigned int rollingValue = rollingVal.toInt();

        setRolling(roller,rollingValue);

        redirect(request,(char*)"/debug"); // rappel de la fonction pour afficher la page
        return;
  }
  
  if (request->method() == HTTP_GET) { 
    // String page;
    // page= String(HEADER) ;
    //page= page + STYLE_w3 +STYLE_Commande ;
    //page= page + String(STYLE_w3) ;
    // page= page + String(STYLE_Commande) ;
    String page1=R"rawliteral(<div class="w3-container">Vous pouvez ajuster le Rolling si besoin. Utiliser avec précaution<table  class="w3-table commandes">)rawliteral";

    // Boucle sur les commandes 
    for (size_t i = 0; i < REMOTES_COUNT; i++)
    {
      String val_i=String(remote_order[i]);
      String rollingValue = String(getRolling(remote_order[i]));
      page1+=R"rawliteral(<tr><td><header class="w3-container w3-card w3-theme"><h1>)rawliteral";
      page1+=String(remote_name[remote_order[i]]);
      page1+=R"rawliteral(</h1></header></td> 
            <td><a href="debug?roller=)rawliteral";
      page1+=val_i;
      page1+=R"rawliteral(&command=0" class="w3-button w3-red w3-xlarge w3-round-large" style="width:100%"><span><div class="tourne90">&#10144;</div></span></a></td>
            <td><a href="debug?roller=)rawliteral";
      page1+=val_i;
      page1+=R"rawliteral(&command=2" class="w3-button w3-grey w3-xlarge w3-round-large" style="width:100%"><span style="color:white;">&#9634;</span></a></td>
            <td>)rawliteral";

      page1 += R"rawliteral(<form action="/debug" method="post" style="display:flex;"><input type="text" name="rolling_value" value=")rawliteral";
      page1 += rollingValue;
      page1 += R"rawliteral(" style="flex:1;"><input type="hidden" name="roller" value=")rawliteral";
      page1 += val_i;
      page1 += R"rawliteral("><button type="submit" class="w3-button w3-green w3-xlarge w3-round-large" style="flex-shrink:0;">Valider</button></form></td></tr>)rawliteral";

      page1+=R"rawliteral(</td></tr>)rawliteral";
     
    Serial.print("Rolling : ");
    Serial.println(getRolling(remote_order[i]));
    }
    size_t totalLength;
    if (internet_ok==true)
      totalLength = strlen(HEADER) + strlen(STYLE_w3) + strlen(STYLE_Commande) + page1.length() +  150;      
    else
      totalLength = strlen(HEADER) + strlen(STYLE_w3_light) + strlen(STYLE_Commande) + page1.length() +  150;      

    char* lapage = new char[totalLength];
    memset(lapage, 0, totalLength);
    strcat(lapage, HEADER);
    if (internet_ok==true)
      strcat(lapage, STYLE_w3);
    else
      strcat(lapage, STYLE_w3_light);
    strcat(lapage, STYLE_Commande);
    strcat(lapage, page1.c_str());
    strcat(lapage, "</table><br><a href=\"\\\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a></div>");
    String tmp= String(lapage);
    tmp+=baspage();
    tmp+=FOOTER;
    // strcat(lapage, FOOTER);
    // Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    // request->send(200, "text/html",  String(lapage));
    request->send(200, "text/html",  tmp);
    delete[] lapage;
   }      
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Programmation avec liste des programmes 
//------------------------------------------------------------------------------------------------------------------------------------
void handlePrgmList(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handlePrgmList - Serving programs list page");

  uint8_t actif = true;
  uint8_t roller = 0;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t command = 0;
  uint8_t result = 0;
  char commandname[20];

if (request->method() == HTTP_GET) { 
    String page;
    page= HEADER ;
  if (internet_ok==true)
      page= page + STYLE_w3 + STYLE_Programme;
  else
      page= page + STYLE_w3_light + STYLE_Programme;

    page+="<header class=\"w3-container w3-card w3-theme\">\
          <h1>Programmes</h1>\
          </header>\
          <div class=\"w3-container\" >\
          <div id=\"new\">  \
          <br/>\
          <a href=\"prgmadd\"  class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Nouveau</a>\
          </div>\
          <br/>\
          <form action=\"prgmactive\" method=\"post\">\
          <table class=\"w3-table w3-large w3-striped\">\
          <tr class=\"w3-teal\"><th>#</th><th>Volet</th><th>h</th><th>m</th><th>cmd</th><th>actif</th><th></th><th></th>\
          </tr>";

// Partie liste programme
    int i = 1;
    for (int prgm=0; prgm<PRGM_COUNT;prgm++) 
    {
      result = getprgm(prgm, &actif,&roller, &hour, &minute, &command);
      if (result) 
      {
        // Serial.print("Roller id : ");
        // Serial.println(roller);
        getcommandname(command, commandname);
        
        page+="<tr><th scope=\"row\">";
        page+=String(i);
        page+="</th><td>";
        page+=String(remote_name[roller]);
        page+="</td><td>";
        page+=String(hour);
        page+="</td><td>";
        page+=String(minute);
        page+="</td><td>";
        page+=String(commandname);
        page+="</td>\
              <td> <label class=\"switch\">\
              <input type=\"checkbox\" name=\"1\" ";
        String chck = (actif==true)?" checked ":"";      
        page+=chck;
        page+=" onclick=\"c(this,";
        page+=prgm;
       
        page+=")\">\
              <span class=\"slider round\"></span>\
              </label></td>\
              <td><a class=\"w3-button w3-circle w3-green w3-button w3-medium\" href=\"prgmupdate?prgm=";
        page+=prgm;
        page+="\">&#9998;</a></td>";
        page+="<td><a class=\"w3-button w3-circle w3-red w3-button w3-medium\" href=\"prgmdelete?prgm=";
        page+=prgm;
        page+="\">&times;</a></td>\
              </td></tr>";
        i++;
      }   
    }
     
    page+="</table><br/>\
            <input type=\"hidden\" id=\"enregistre\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
            </form>\
            <br/>\
            <a href=\"\\\" id=\"retour\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a><br/></div>";

    page+=baspage();
    page+=FOOTER;
    request->send(200, "text/html", page);
   }   
}

//------------------------------------------------------------------------------------------------------------------------------------
// Efface programme
//------------------------------------------------------------------------------------------------------------------------------------
void handlePrgmDelete(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handlePrgmDelete - Serving programs delete page");

  if(request->hasArg("prgm")){
    String prgm = request->arg("prgm");
    // Serial.print("WEBSERVER - handlePrgmDelete - Deleting prgm : ");
    // Serial.println(prgm);
    delprgm(prgm.toInt());
    refresh_programTask();
  }
  redirect(request, (char*)"/prgmlist");
}

//------------------------------------------------------------------------------------------------------------------------------------
// Active programme
//------------------------------------------------------------------------------------------------------------------------------------
void handlePrgmActive(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handlePrgmActive - Serving programs Active");

  if(request->hasArg("prgm")){
    String prgm = request->arg("prgm");
    String check = request->arg("check");
    int checked = (check=="true")?1:0;
    // Serial.print("WEBSERVER - handlePrgmActive - Active ou desactive prgm : ");
    // Serial.print(prgm);
    // Serial.print("parametre checked : ");
    // Serial.println(checked);
    activeprgm(prgm.toInt(),checked);
    refresh_programTask();
  }
  redirect(request, (char*)"/prgmlist");
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Ajout programme
//------------------------------------------------------------------------------------------------------------------------------------
void handlePrgmAdd(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handlePrgmAdd - Serving program add page");

  if (request->method() == HTTP_POST) {
    String roller = request->arg("roller");
    // Serial.print("WEBSERVER - handlePrgmAdd - Roller : ");
    // Serial.println(roller);
    String hour = request->arg("hour");
    // Serial.print("WEBSERVER - handlePrgmAdd - Hour : ");
    // Serial.println(hour);
    String minute = request->arg("minute");
    // Serial.print("WEBSERVER - handlePrgmAdd - Minute : ");
    // Serial.println(minute);
    String command = request->arg("command");
    // Serial.print("WEBSERVER - handlePrgmAdd - Command : ");
    // Serial.println(command);

    if (roller.length() != 0 && hour .length() != 0 && minute.length() != 0 && command.length() != 0) {
      addprgm(roller.toInt(), hour.toInt(), minute.toInt(), command.toInt());
      refresh_programTask();
    }
    redirect(request, (char*)"/prgmlist");
    return;
  }

  if (request->method() == HTTP_GET) {
    String page;
    page= HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + HEADER_close;
    else
        page= page + STYLE_w3_light + HEADER_close;
    page+="<header class=\"w3-container w3-card w3-theme\">\
            <h1>Nouveau</h1>\
          </header>\
          <div class=\"w3-container\">\
          <br/>\
          <form action=\"prgmadd\" method=\"post\">\
          <div class=\"form-group\">\
          <select class=\"w3-select w3-xxlarge\" id=\"roller\" name=\"roller\">\
          <option value=\"\" disabled selected>Volet</option>";

    for (size_t i = 0; i < REMOTES_COUNT; i++) {
        page+="<option value=\"";
        page+=String(remote_order[i]);
        page+="\">";
        page+=String(remote_name[remote_order[i]]);
        page+="</option>";
    }

    page+="</select><br/><br/>\
        <select class=\"w3-select w3-xxlarge\" id=\"command\" name=\"command\">\
        <option value=\"\" disabled selected>Commande</option>\
        <option value=\"0\">Fermer</option>\
        <option value=\"1\">Ouvrir</option>\
        <option value=\"4\">Fermer &#9728; au coucher du soleil</option>\
        <option value=\"2\">Fermer &#x2606 au début de la nuit</option>\
        <option value=\"3\">Ouvrir &#9728; au lever du soleil</option>\
        <option value=\"5\">Ouvrir &#x2606 au début du jour</option>\
      </select>";
    
    // Heures/minutes
    page+="</select><br/><br/>\
    <select class=\"w3-select w3-xxlarge\" id=\"hour\" name=\"hour\">\
    <option value=\"\" disabled selected>Heures</option>";

    for (size_t i = 6; i < 24; i++) { // Heures
            page+="<option value=\"";
            page+=String(i);
            page+="\">";
            page+=String(i);
            page+="</option>\"";
    }

    page+="</select><br/><br/>\
            <select class=\"w3-select w3-xxlarge\" id=\"minute\" name=\"minute\">\
            <option value=\"\" disabled selected>Minutes</option>";
    for (size_t i = 0; i < 60; i=i+5) {  // minutes
            page+="<option value=\"";
            page+=String(i);
            page+="\">";
            page+=String(i);
            page+="</option>\"";
    }
          
    page+="</div><br/>\
          <input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
        </form><br/>\
        <a href=\"prgmlist\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>\
    </a>\
    <br/>";
    page+="<i>Si l'heure de lever ou coucher de soleil inconnue, l'heure indiquée sera utilisée. Réglage nuit/jour dans paramètrage heure.</i></div>";
    page+=baspage();
    page+=FOOTER;
    request->send(200, "text/html", page);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Modification programme
//------------------------------------------------------------------------------------------------------------------------------------
void handlePrgmUpdate(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handlePrgmUpdate - Serving program add page");
    String numprog = request->arg("prgm");
  if (request->method() == HTTP_POST) {
    String roller = request->arg("roller");
    // Serial.print("WEBSERVER - handlePrgmUpdate - Roller : ");
    // Serial.println(roller);
    String hour = request->arg("hour");
    // Serial.print("WEBSERVER - handlePrgmUpdate - Hour : ");
    // Serial.println(hour);
    String minute = request->arg("minute");
    // Serial.print("WEBSERVER - handlePrgmUpdate - Minute : ");
    // Serial.println(minute);
    String command = request->arg("command");
    // Serial.print("WEBSERVER - handlePrgmUpdate - Command : ");
    // Serial.println(command);
    

    if (roller.length() != 0 && hour.length() != 0 && minute.length() != 0 && command.length() != 0 && numprog.length() != 0)  {
      updateprgm(numprog.toInt(),roller.toInt(), hour.toInt(), minute.toInt(), command.toInt());
      refresh_programTask();
    }
    redirect(request, (char*)"/prgmlist");
    return;
  }

  if (request->method() == HTTP_GET) {
    uint8_t actif;
    uint8_t roller;
    uint8_t hour;
    uint8_t minute;
    uint8_t command;
    getprgm(numprog.toInt(), &actif, &roller, &hour, &minute, &command);
    String page;
    page= HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + HEADER_close;
    else
        page= page + STYLE_w3_light + HEADER_close;
    page+="<header class=\"w3-container w3-card w3-theme\">\
            <h1>Modification programme</h1>\
          </header>\
          <div class=\"w3-container\">\
          <br/>\
          <form action=\"prgmupdate?prgm=";
    page+= numprog;   
    page+="\" method=\"post\">\
          <input id=\"numprog\" name=\"numprog\" type=\"hidden\" value=\"";
    page+=numprog;
    page+="\"><div class=\"form-group\">\
          <select class=\"w3-select w3-xxlarge\" id=\"roller\" name=\"roller\">\
          <option value=\"\" disabled>Volet</option>";

    for (size_t i = 0; i < REMOTES_COUNT; i++) {
        page+="<option value=\"";
        page+=String(remote_order[i]);
        if (roller==remote_order[i])
          page+="\" selected>";
        else
          page+="\">";
        page+=String(remote_name[remote_order[i]]);
        page+="</option>";
    }

    page+="</select><br/><br/>\
        <select class=\"w3-select w3-xxlarge\" id=\"command\" name=\"command\">\
        <option value=\"\" disabled>Commande</option>\
        <option value=\"0\"";
    if (command==0) page+=" selected ";
    page+=">Fermer</option>\
            <option value=\"1\"";
    if (command==1) page+=" selected ";            
    page+=">Ouvrir</option>\
            <option value=\"4\"";
    if (command==4) page+=" selected ";            
    page+=">Fermer &#9728; au coucher du soleil</option>\
            <option value=\"2\"";
    if (command==2) page+=" selected ";            
    page+=">Fermer &#x2606; au début de la nuit</option>\
            <option value=\"3\"";
    if (command==3) page+=" selected ";            
    page+=">Ouvrir &#9728; au lever du soleil</option>\
            <option value=\"5\"";
    if (command==5) page+=" selected ";            
    page+=">Ouvrir &#x2606; au début du jour</option>\
            </select>";

    // Heures/minutes
    page+="</select><br/><br/>\
    <select class=\"w3-select w3-xxlarge\" id=\"hour\" name=\"hour\">\
    <option value=\"\" disabled>Heures</option>";

    for (size_t i = 6; i < 24; i++) { // Heures
            page+="<option value=\"";
            page+=String(i);
            if (hour==i)
              page+="\" selected>";
            else
              page+="\">";
            page+=String(i);
            page+="</option>\"";
    }

    page+="</select><br/><br/>\
            <select class=\"w3-select w3-xxlarge\" id=\"minute\" name=\"minute\">\
            <option value=\"\" disabled>Minutes</option>";
    for (size_t i = 0; i < 60; i=i+5) {  // minutes
            page+="<option value=\"";
            page+=String(i);
            if (minute==i)
              page+="\" selected>";
            else
              page+="\">";
            page+=String(i);
            page+="</option>\"";
    }
          
    page+="</div><br/>\
          <input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
        </form><br/>\
        <a href=\"prgmlist\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>\
    </a>\
    <br/>";
    page+="<i>Si l'heure de lever ou coucher de soleil inconnue, l'heure indiquée sera utilisée. Réglage nuit/jour dans paramètrage heure.</i></div>";
    page+=baspage();
    page+=FOOTER;
    request->send(200, "text/html", page);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Appairer les télécommandes aux volets V0
//------------------------------------------------------------------------------------------------------------------------------------
void handleAttach(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleAttach - Programmation télécommandes");

  if (request->hasArg("roller")) {
    String roller_str = request->arg("roller");
    int roller = roller_str.toInt();

    // Serial.print("WEBSERVER - handleAttach - Attaching roller : ");
    // Serial.println(roller);
    prog(roller);

    redirect(request, (char*)"/attach");
    return;
  }

  String page;
  page = HEADER ;
  if (internet_ok==true)
    page= page + STYLE_w3 + HEADER_close;
  else
    page= page + STYLE_w3_light + HEADER_close;

  page+="<header class=\"w3-container w3-card w3-theme\">\
  <h1>Appairer télécommandes aux Volets</h1>\
  </header>\
  <div class=\"w3-container\">\
  <br/>";
  // Boucle sur les commandes
  for (size_t i = 0; i < REMOTES_COUNT; i++)
  {
      page+="<a href=\"attach?roller=";
      page+= String(remote_order[i]) ;
      page+="\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">";
      page+=remote_name[remote_order[i]];
      page+="</a><br/>";
  }
  page+="<a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
  page+=baspage();
  page+= FOOTER;
  request->send(200, "text/html", page);
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Appairer les télécommandes aux volets V1
//------------------------------------------------------------------------------------------------------------------------------------
// void handleAttach(AsyncWebServerRequest *request) {
//   identification(request);
//   write_output_ln("WEBSERVER - handleAttach - Programmation télécommandes");

//   if (request->hasArg("roller")) {
//     String roller_str = request->arg("roller");
//     int roller = roller_str.toInt();

//     // Serial.print("WEBSERVER - handleAttach - Attaching roller : ");
//     // Serial.println(roller);
//     prog(roller);

//     redirect(request, (char*)"/attach");
//     return;
//   }

//   String page;
//   page = HEADER ;
//   if (internet_ok==true)
//     page= page + STYLE_w3 + HEADER_close;
//   else
//     page= page + STYLE_w3_light + HEADER_close;

//   page+="<header class=\"w3-container w3-card w3-theme\">\
//   <h1>Appairer télécommande aux Volets</h1>\
//   </header>\
//   <div class=\"w3-container\">\
//   <br/>";
//   // Boucle sur les commandes

//     page+=R"rawliteral(<div class="w3-container"><table  class="w3-table commandes">)rawliteral";

//     // Boucle sur les commandes 
//     for (size_t i = 0; i < REMOTES_COUNT; i++)
//     {
//       String val_i=String(remote_order[i]);
//       page+=R"rawliteral(<tr><td><header class="w3-container w3-card w3-theme"><h1>)rawliteral";
//       page+=String(remote_name[remote_order[i]]);
//       page+=R"rawliteral(</h1></header></td> 
//             <td><a href="attach?roller=)rawliteral";
//       page+=val_i;
//       page+=R"rawliteral(&command=0" class="w3-button w3-red w3-xlarge w3-round-large" ><span><div class="tourne90">&#10144;</div></span></a></td>
//             <td><a href="command?roller=)rawliteral";
//       page+=val_i;
//       page+=R"rawliteral(&command=2" class="w3-button w3-grey w3-xlarge w3-round-large" style="width:100%"><span style="color:white;">&#9634;</span></a></td>
//             <td><a href="command?roller=)rawliteral";
//       page+=val_i;
//       page+=R"rawliteral(&command=1" class="w3-button w3-teal w3-xlarge w3-round-large" style="width:100%"><span><div class="tourne270">&#10144;</div></span></a></td></tr>)rawliteral";
     
//     }


//   // for (size_t i = 0; i < REMOTES_COUNT; i++)
//   // {
//   //     page+="<a href=\"attach?roller=";
//   //     page+= String(remote_order[i]) ;
//   //     page+="\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">";
//   //     page+=remote_name[remote_order[i]];
//   //     page+="</a><br/>";
//   // }


//   page+="<a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
//   page+=baspage();
//   page+= FOOTER;
//   request->send(200, "text/html", page);
// }

//------------------------------------------------------------------------------------------------------------------------------------
// Page paramètres WIFI parametrage
//------------------------------------------------------------------------------------------------------------------------------------
void handleWifi(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleWifi - Configuration wifi");

  if (request->method() == HTTP_POST) { 
    // Bug l'ESP reboot !
    if (request->hasArg("refresh"))
    {
      // Scan reseau  
        esp_task_wdt_init(30, true); //délai d'attente est désactivé pendant 20 sec. pour eviter le redemarrage de l'EP32
        esp_task_wdt_add(NULL);
      scanner_reseau();
         esp_task_wdt_reset();
      Serial.println("scan reseau fait");
      redirect(request, (char*)"/wifi");
    }
    else
    if (request->hasArg("save")) {
      String accesspoint = request->arg("accesspoint");
      prefs_set_str("wifi","accesspoint",accesspoint);
      String password = request->arg("password");
      prefs_set_str("wifi","password",password); 
      request->redirect("/config");
    }
    return;
    }

  // GET Method
  if (request->method() == HTTP_GET) {
    String page;
    char accesspoint_str[ACCESSPOINT_LENGTH];
    prefs_get_str("wifi","accesspoint",accesspoint_str,ACCESSPOINT_LENGTH,"");
    // Construction de la page	
    page = HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + STYLE_Wifi;
    else
        page= page + STYLE_w3_light + STYLE_Wifi;
        
    page+="<header class=\"w3-container w3-card w3-theme\"><h1>Wifi</h1></header>\
    <header class=\"wifi w3-container w3-card \">\
    <div class=\"center\">\
    <div class=\"wrap no\"><div id=\"aplist\">"; 
    page+=Liste_reseau;
    page+="</div><div><div class=\"loading\" style=\"display:none\"></div></div>\
          <form action='/wifi?refresh=1' method='POST'><button onclick=\"document.querySelector('#aplist').style.display='none';document.querySelector('.loading').style.display='block';\"  class=\"w3-button w3-teal w3-xxlarge w3-round-large \" name='refresh' value='1'>Scan AP</button></form>";
    // Formulaire saisie SSID mot de passe et boutons 
    page+="</div></div></header>\
        <div class=\"w3-container\">\
        <form action=\"wifi?save\" method=\"POST\">\
        <p>\
        <label class=\"w3-text-teal w3-xxlarge\"><b>Point d'acc&egrave;s</b></label>\
        <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"accesspoint\" name=\"accesspoint\" type=\"text\" value=\"";
    page+=accesspoint_str;
    page+="\"></p>\
        <p>\
        <p>\
        <label class=\"w3-text-teal w3-xxlarge\"><b>Mot de passe</b></label>\
        <input class=\"w3-input w3-border w3-light-grey w3-xxlarge \" id=\"password\" name=\"password\" type=\"text\"></p>\
        <p>\
        <input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
        </form>\
        <br/>\
        <a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
      page+=baspage();
      page+=FOOTER;

   request->send(200, "text/html, charset=UTF-8", page);
    
  
  }
}


//------------------------------------------------------------------------------------------------------------------------------------
// Page parametre reseau
//------------------------------------------------------------------------------------------------------------------------------------
void handleReseau(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleReseau - Serving reseau page");

  if (request->method() == HTTP_POST) {
    if(request->hasArg("ip")){
      String reseau_ip = request->arg("ip");
      prefs_set_str("reseau","ip",reseau_ip);
    }

    if(request->hasArg("masque")){
      String reseau_masque = request->arg("masque");
      prefs_set_str("reseau","masque",reseau_masque);
    }

    if(request->hasArg("passerelle")){
      String reseau_passerelle = request->arg("passerelle");
      prefs_set_str("reseau","passerelle",reseau_passerelle);
    }

    if(request->hasArg("dhcp")){
      prefs_set_bool("reseau","dhcp",true);
    }
    else{
      prefs_set_bool("reseau","dhcp",false);
    }

    request->redirect("/config");
    return;
  }


  // GET Method
  if (request->method() == HTTP_GET) {
     char reseau_ip[IP_LENGTH];
     prefs_get_str("reseau","ip",reseau_ip,IP_LENGTH,"192.168.1.111");
     char reseau_masque[IP_LENGTH];
     prefs_get_str("reseau","masque",reseau_masque,IP_LENGTH,"255.255.255.0");
     char reseau_passerelle[IP_LENGTH];
     prefs_get_str("reseau","passerelle",reseau_passerelle,IP_LENGTH,"192.168.1.254");
     boolean reseau_dhcp=prefs_get_bool("reseau","dhcp",true);

    String page;
    page = HEADER ;
    if (internet_ok==true)
        page+= STYLE_w3;
    else
        page+= STYLE_w3_light;

    page += STYLE_reseau;
    page += R"rawliteral(<header class="w3-container w3-card w3-theme">
    <h1 >Réseau</h1>
    </header>
    <script>
      document.addEventListener("DOMContentLoaded", function() {activeDHCP();});
    function activeDHCP(event) {
        var checkBox = document.getElementsByName("dhcp")[0];
        var paramDiv = document.getElementById("param");
        if (checkBox.checked) 
            paramDiv.style.display = "none";
        else 
            paramDiv.style.display = "block";
    }
      </script>
      <div class="w3-container">
      <form action="reseau" method="post">
          <label class="w3-text-teal w3-xxlarge" for="fname"><b>DHCP</b></label><br/>
          <label class="switch"><input type="checkbox" onclick="activeDHCP(event)" id="dhcp" name="dhcp")rawliteral";
      page +=(reseau_dhcp == true)?"checked":"";
          
      page += R"rawliteral(><span class="slider round"></span></label>

      <div id="param">
      <p>
        <label class="w3-text-teal w3-xxlarge"><b>IP</b></label>
        <input class="w3-input w3-border w3-light-grey w3-xxlarge" id="ip" name="ip" type="text" value=")rawliteral";
      page+=reseau_ip;
      page += R"rawliteral("></p>
      <p>
      <p>
        <label class="w3-text-teal w3-xxlarge"><b>Masque</b></label>
        <input class="w3-input w3-border w3-light-grey w3-xxlarge" id="masque" name="masque" type="text" value=")rawliteral";
      page+=reseau_masque;
      page += R"rawliteral("></p>
      <p>
      <p>
        <label class="w3-text-teal w3-xxlarge"><b>Passerelle</b></label>
        <input class="w3-input w3-border w3-light-grey w3-xxlarge" id="passerelle" name="passerelle" type="text" value=")rawliteral";
      page+=reseau_passerelle;
      page += R"rawliteral("></p>
      <p>
      </div>
      </p>
      <input type="submit" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block" value="Enregistrer">
      </form>
      <br/>
      <a href="config" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Retour</a>
      )rawliteral";
      page+=baspage();
page+=FOOTER;
//Serial.print(page);
    request->send(200, "text/html, charset=UTF-8", page);
    return;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page parametre sécurité activation mot de passe administrateur 
//------------------------------------------------------------------------------------------------------------------------------------
void handleSecurite(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleReseau - Serving reseau page");

  if (request->method() == HTTP_POST) {

    if(request->hasArg("activer")){
      if(request->hasArg("mdp")){
        String admin_mdp = request->arg("mdp");
        prefs_set_str("admin","mdp",admin_mdp);
      }
      if(request->hasArg("identifiant")){
        String admin_id = request->arg("identifiant");
        prefs_set_str("admin","identifiant",admin_id);
      }      
      prefs_set_bool("admin","active",true);
    }
    else{
      prefs_set_bool("admin","active",false);
    }
    request->redirect("/config");
    return;
  }
  
  // GET Method
  if (request->method() == HTTP_GET) {
     char admin_mdp[PASSWORD_LENGTH];
     prefs_get_str("admin","mdp",admin_mdp,PASSWORD_LENGTH,"admin");
     char admin_id[PASSWORD_LENGTH];
     prefs_get_str("admin","identifiant",admin_id,PASSWORD_LENGTH,"admin");

     boolean admin_actif=prefs_get_bool("admin","active",false);

    String page;
    page = HEADER ;
    if (internet_ok==true)
        page+= STYLE_w3;
    else
        page+= STYLE_w3_light;

    page+=STYLE_password;
    page += STYLE_reseau;
    page += R"rawliteral(<header class="w3-container w3-card w3-theme">
    <h1 >Compte Administrateur</h1>
    </header>
    <script>
      document.addEventListener("DOMContentLoaded", function() {activeCompte();});
    function activeCompte(event) {
        var checkBox = document.getElementsByName("activer")[0];
        var paramDiv = document.getElementById("param");
        if (checkBox.checked) 
            paramDiv.style.display = "block";
        else 
            paramDiv.style.display = "none";
    }
      </script>
      <div class="w3-container">
      <form action="securite" method="post">
          <label class="w3-text-teal w3-xxlarge" for="fname"><b>Activer identification</b></label><br/>
          <label class="switch"><input type="checkbox" onclick="activeCompte(event)" id="activer" name="activer")rawliteral";
      page +=(admin_actif == true)?"checked":"";
          
      page += R"rawliteral(><span class="slider round"></span></label>

      <div id="param">
      <p>
        <label class="w3-text-teal w3-xxlarge"><b>Identifiant</b></label>
        <input class="w3-input w3-border w3-light-grey w3-xxlarge" id="identifiant" name="identifiant" type="text" value=")rawliteral";
      page+=admin_id;
      page += R"rawliteral("></p>
      <p>
      <p>
      <div class="passdiv">
        <label class="w3-text-teal w3-xxlarge"><b>Mot de passe</b></label>
        <input class="w3-input w3-border w3-light-grey w3-xxlarge" id="mdp" name="mdp" type="password" value=")rawliteral";
      page+=admin_mdp;
      page += R"rawliteral(">
        <div class="oeil" onmousedown="document.getElementById('mdp').type ='text'" onmouseup="document.getElementById('mdp').type ='password'"></div>
      </div>
      </p>
      <p>
      </div>
      </p>
      <input type="submit" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block" value="Enregistrer">
      </form>
      <br/>
      <a href="config" class="w3-button w3-teal w3-xxlarge w3-round-large w3-block">Retour</a>
      )rawliteral";
      page+=baspage();
page+=FOOTER;
//Serial.print(page);
    request->send(200, "text/html, charset=UTF-8", page);
    return;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page paramètres avancés
//------------------------------------------------------------------------------------------------------------------------------------
void handleApplication(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleApplication - Serving application page");

  if (request->method() == HTTP_POST) {
    if(request->hasArg("key")){
      String key_new = request->arg("key");
      write_output_ln("WEBSERVER - handleApplication - Storing key : " + key_new);
      prefs_set_str("application","key",key_new);
    }

    if(request->hasArg("token")){
      String token_arg = request->arg("token");
      write_output_ln("WEBSERVER - handleApplication - Storing token : " + token_arg);
      prefs_set_str("token","token",token_arg);
    }

    if(request->hasArg("RTSaddress")){
      String RTSaddress = request->arg("RTSaddress");
      write_output_ln("WEBSERVER - handleApplication - Storing RTSaddress : " + RTSaddress);
      prefs_set_long("somfy","rts",(unsigned long) RTSaddress.toInt());
      somfy.setRTS_address((unsigned long) RTSaddress.toInt());
    }

    redirect(request, (char*)"/config");
    return;
  }


  // GET Method
  if (request->method() == HTTP_GET) {
    char key_str[KEY_LENGTH];
    prefs_get_str("application","key",key_str,KEY_LENGTH,"");
    prefs_get_str("token","token",token,16,"");

    unsigned long RTS_address=prefs_get_long("somfy","rts",0x121340);

    String page;
    page = HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + HEADER_close;
    else
        page= page + STYLE_w3_light + HEADER_close;

    page= page + "<header class=\"w3-container w3-card w3-theme\">\
<h1>Options avancées</h1>\
</header>\
<div class=\"w3-container\">\
<form action=\"application\" method=\"post\">\
  <p>\
  <label class=\"w3-text-teal w3-xxlarge\"><b>Cl&eacute; obfuscation des requêtes</b></label>\
  <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"key\" name=\"key\" type=\"text\" value=\""+key_str+"\"></p>\
<p>\
<hr>\
  <label class=\"w3-text-teal w3-xxlarge\"><b>Token d'accéder à l'API</b></label>\
  <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"token\" name=\"token\" type=\"text\" value=\""+token+"\"></p>\
<br>\
<hr>\
  <label class=\"w3-text-teal w3-xxlarge\"><b>Adresse base télécommande RTS</b></label>\
  <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"RTSaddress\" name=\"RTSaddress\" type=\"text\" value=\""+RTS_address+"\"></p>\
<br>\
<input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
</form>\
<br/>\
<a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
page+=baspage();
page+=FOOTER;

    request->send(200, "text/html", page);
    return;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page paramètres Wake On LAN permettant d'ajouter un bouton à l'accueil pour reveiller une machine via le réseau
//------------------------------------------------------------------------------------------------------------------------------------
void handleWOL(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleWOL - Serving WOL page");

  if (request->method() == HTTP_POST) {
    if(request->hasArg("adr_mac")){
      String adr_mac = request->arg("adr_mac");
      write_output_ln("WEBSERVER - handleWOL - Storing adr_mac : " + adr_mac);
      prefs_set_str("wol","adr_mac",adr_mac);
    }

    if(request->hasArg("ip_serve")){
      String ip_arg = request->arg("ip_serve");
      write_output_ln("WEBSERVER - handleWOL - Storing IP : " + ip_arg);
      prefs_set_str("wol","ip_serve",ip_arg);
    }

    redirect(request, (char*)"/config");
    return;
  }


  // GET Method
  if (request->method() == HTTP_GET) {
    char ip_serve[18];
    char adr_mac[20];
    prefs_get_str("wol","adr_mac",adr_mac,20,"");
    prefs_get_str("wol","ip_serve",ip_serve,18,"");

    String page;
    page = HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + HEADER_close;
    else
        page= page + STYLE_w3_light + HEADER_close;

    page= page + "<header class=\"w3-container w3-card w3-theme\">\
<h1>Wake on LAN - Paramètres de la machine à réveiller</h1>\
</header>\
<div class=\"w3-container\">\
<form action=\"wol\" method=\"post\">\
  <p>\
  <label class=\"w3-text-teal w3-xxlarge\"><b>Adresse MAC (xx:xx:xx:xx:xx:xx)</b></label>\
  <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"adr_mac\" name=\"adr_mac\" type=\"text\" value=\""+adr_mac+"\"></p>\
<p>\
<hr>\
  <label class=\"w3-text-teal w3-xxlarge\"><b>Adresse IP de diffusion pour le réseau (ex: 192.168.1.255) </b></label>\
  <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"ip_serve\" name=\"ip_serve\" type=\"text\" value=\""+ip_serve+"\"></p>\
<br>\
<input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
</form>\
<br/>\
<a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
page+=baspage();
page+=FOOTER;

    request->send(200, "text/html", page);
    return;
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
// Page Actionne  Wake On LAN 
//------------------------------------------------------------------------------------------------------------------------------------
void handleActionWOL(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleActionWOL - Serving ActionWOL page");
    char ip_serve[18];
    char adr_mac[20];
    prefs_get_str("wol","adr_mac",adr_mac,20,"");
    prefs_get_str("wol","ip_serve",ip_serve,18,"");
    sendWOL(adr_mac,ip_serve);
    redirect(request,(char*)"/"); // rappel de la fonction pour afficher la page d'acceuil
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Page Horloge  Synchronysation 
//------------------------------------------------------------------------------------------------------------------------------------
void handleClock(AsyncWebServerRequest *request) {
  identification(request);
  write_output_ln("WEBSERVER - handleClock - Serving clock page");

  if (request->method() == HTTP_POST) {

  for (uint8_t i = 0; i < request->args(); i++) {
    String argName = request->argName(i); // nom de l'argument
    String argValue = request->arg(i);   // valeur de l'argument
    Serial.println("Argument " + argName + " : " + argValue);
  }

    if(request->hasArg("ntp_server")){
      String ntp_server = request->arg("ntp_server");
      write_output_ln("WEBSERVER - handleApplication - Storing ntp_server : " + ntp_server);
      prefs_set_str("ntp","ntp_server",ntp_server);
      inittime();
    }

    if(request->hasArg("openweathermap_api")){
      String openweathermap_api_new = request->arg("openweathermap_api");
      write_output_ln("WEBSERVER - handleApplication - Storing openweathermap_api : " + openweathermap_api_new);
      prefs_set_str("openweathermap","api",openweathermap_api_new);
    }

    if(request->hasArg("longitude")){
      String lon_new = request->arg("longitude");
      write_output_ln("WEBSERVER - handleApplication - Storing longitude : " + lon_new);
      prefs_set_str("openweathermap","lon",lon_new);
    }

    if(request->hasArg("latitude")){
      String lat_new = request->arg("latitude");
      write_output_ln("WEBSERVER - handleApplication - Storing latitude : " + lat_new);
      prefs_set_str("openweathermap","lat",lat_new);
    }

    if(request->hasArg("jour")){
      String jour = request->arg("jour");
      write_output_ln("WEBSERVER - handleApplication - Storing jour : " + jour);
      prefs_set_str("heure","jour",jour);
    }

    if(request->hasArg("nuit")){
      String nuit = request->arg("nuit");
      write_output_ln("WEBSERVER - handleApplication - Storing nuit : " + nuit);
      prefs_set_str("heure","nuit",nuit);
    }

    // A voir pour le faire si on le demande 
    String hours = request->arg("hours");
    String minutes = request->arg("minutes");
    if (hours!="" && minutes!="")
      settime(hours.toInt(), minutes.toInt());


    redirect(request, (char*)"/config");
    return;
  }

  if (request->method() == HTTP_GET) {
    uint8_t hours=0;
    uint8_t minutes=0;

    char ntp_server[NTP_SERVER_LENGTH];
    prefs_get_str("ntp","ntp_server",ntp_server,NTP_SERVER_LENGTH,"pool.ntp.org");

    char openweathermap_api[50];
    char lon[10]="";
    char lat[10]="";
    char nuit[4]="";
    char jour[4]="";
    prefs_get_str("openweathermap","api",openweathermap_api,50,"");
    prefs_get_str("openweathermap","lon",lon,9,"2.3333");
    prefs_get_str("openweathermap","lat",lat,9,"48.8666");
    prefs_get_str("heure","jour",jour,4,"30");
    prefs_get_str("heure","nuit",nuit,4,"30");
 
    String page;

    page = HEADER ;
    if (internet_ok==true)
        page= page + STYLE_w3 + HEADER_close;
    else
        page= page + STYLE_w3_light + HEADER_close;

    page=page+"<header class=\"w3-container w3-card w3-theme\">\
            <h1>Réglage de l'heure</h1>\
            </header>\
            <div class=\"w3-container\">\
            <form action=\"clock\" method=\"post\">\
            <label class=\"w3-text-teal w3-xxlarge\" for=\"ntp_server\"><b>NTP serveur (Heure)</b></label><br/>\
            <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" type=\"text\" id=\"ntp_server\" name=\"ntp_server\" value=\""+ntp_server+"\"><br/>\
            <p>\
            <label class=\"w3-text-teal w3-xxlarge\" for=\"openweathermap_api\"><b>Clé API openweathermap (lever et coucher soleil)</b></label><br/>\
            <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" type=\"text\" id=\"openweathermap_api\" name=\"openweathermap_api\" value=\""+openweathermap_api+"\"><br/>\
<div class=\"w3-row\">\
    <div class=\"w3-half\">\
      <label class=\"w3-text-teal w3-xxlarge\" for=\"longitude\">Longitude</label><br/>\
      <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" type=\"text\" id=\"longitude\" name=\"longitude\" value=\""+lon+"\"><br/>\
    </div>\
    <div class=\"w3-half\">\
      <label class=\"w3-text-teal w3-xxlarge\" for=\"latitude\">Latitute</label><br/>\
      <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" type=\"text\" id=\"latitude\" name=\"latitude\" value=\""+lat+"\"><br/>\
    </div>\
  </div>\
  <div class=\"w3-row\">\
    <div class=\"w3-half\">\
      <label class=\"w3-text-teal w3-xlarge\" for=\"jour\">jour : nb de minutes avant lever</label><br/>\
      <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" maxlength=\"4\" type=\"text\" id=\"jour\" name=\"jour\" value=\""+jour+"\"><br/>\
    </div>\
    <div class=\"w3-half\">\
      <label class=\"w3-text-teal w3-xlarge\" for=\"nuit\">nuit : nb de minutes après coucher</label><br/>\
      <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" maxlength=\"4\" type=\"text\" id=\"nuit\" name=\"nuit\" value=\""+nuit+"\"><br/>\
    </div>\
  </div>\
            <input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Enregistrer\">\
            </form>\
            <hr>\
            <p>\
            <form action=\"clock\" method=\"post\">\
            <label class=\"w3-text-teal w3-xxlarge\"><b>Heures</b></label>\
            <input class=\"w3-input w3-border w3-light-grey w3-xxlarge\" id=\"hours\" name=\"hours\" type=\"text\" value=\""+hours+"\"></p>\
            <p>\
            <p>\
            <label class=\"w3-text-teal w3-xxlarge\"><b>Minutes</b></label>\
            <input class=\"w3-input w3-border w3-light-grey w3-xxlarge \" id=\"minutes\" name=\"minutes\" type=\"text\" value=\""+minutes+"\"></p>\
            <p>\
            <input type=\"submit\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\" value=\"Ajuster l'heure\">\
            </form>\
            <br/>\
            <a href=\"config\" class=\"w3-button w3-teal w3-xxlarge w3-round-large w3-block\">Retour</a>";
            page+=baspage();
      page+= FOOTER;

    request->send(200, "text/html", page);
  }
}


//------------------------------------------------------------------------------------------------------------------------------------
// Fonction API si un token n'est pas défini dans la page de configuration l'API n'est pas accéssible
//------------------------------------------------------------------------------------------------------------------------------------
void handleApi(AsyncWebServerRequest *request) {
    identification(request);
    prefs_get_str("token","token",token,16,"");
    write_output_ln("WEBSERVER - handleAPI - Execution commande");

  if (strlen(token)==0) {
    write_output_ln("WEBSERVER - handleAPI - token no define");
    request->send(401);
  }
  else
  if (request->hasArg("roller") && request->hasArg("command") && request->hasArg("token")) {
    String roller_str = request->arg("roller");
    String command_str = request->arg("command");
    String token_str = request->arg("token");
    int command = command_str.toInt();
    int roller = roller_str.toInt();

    if (token_str.compareTo(token) != 0){
      write_output_ln("WEBSERVER - handleAPI - wrong token: " + token_str);
      request->send(401);
    }else{
      if (command == 0) {
        write_output_ln("WEBSERVER - handleAPI - Move down roller : " + String(roller));
        request->send(200, "text/plain", "OK");
        movedown(roller);
        return;
      }
      else if (command == 1) {
        write_output_ln("WEBSERVER - handleAPI - Move up roller : " + String(roller));
        request->send(200, "text/plain", "OK");
        moveup(roller);
        return;
      }
      else if (command == 2) {
        write_output_ln("WEBSERVER - handleAPI - Stop roller : " + String(roller));
        request->send(200, "text/plain", "OK");
        stop(roller);
        return;
      }
    }
  }
}


//------------------------------------------------------------------------------------------------------------------------------------
// Création de l'adresse url, code 
//------------------------------------------------------------------------------------------------------------------------------------
void get_obfuscated_url(char * url, char * key, char * baseurl) {

  if (strlen(key) == 0) {
    strcpy(url, baseurl);
  }
  else {
    sprintf(url, "/%s%s", key, baseurl);
  }
  // Serial.print("WEBSERVER - get_obfuscated_url - url :");
  // Serial.println(url);

}

//------------------------------------------------------------------------------------------------------------------------------------
// redirection page
//------------------------------------------------------------------------------------------------------------------------------------
void redirect(AsyncWebServerRequest *request, char * url) {

  char obfuscated_url[40];

  get_obfuscated_url(obfuscated_url, key, url);


  request->redirect(obfuscated_url);
}

//------------------------------------------------------------------------------------------------------------------------------------
// Nom de la commande Fermer/ Ouvrir
//------------------------------------------------------------------------------------------------------------------------------------
void getcommandname(uint8_t command, char * name) {
  switch (command) {
    case 0 :
      strcpy(name, "Fermer");
      break;
    case 1 :
      strcpy(name, "Ouvrir");
      break;      
    case 2 :
      strcpy(name, "Fermer &#x2606;");
      break;
    case 3 :
      strcpy(name, "Ouvrir &#9728;"); 
      break;
    case 4 :
      strcpy(name, "Fermer &#9728;");
      break;
    case 5 :
      strcpy(name, "Ouvrir &#x2606;");
      break;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------
// Configuration initialisation générale avec les liens des pages => fonctions
//------------------------------------------------------------------------------------------------------------------------------------
void ws_config(int rescue_mode) {

  write_output_ln("WEBSERVER - ws_config - Initialization");


  if (rescue_mode) {
    strcpy(key, "");
  }
  else {
    strcpy(key, "");
    prefs_get_str("application","key",key,KEY_LENGTH,"");
  }
  char url[40];

  get_obfuscated_url(url, key, (char*)"/");
  server.on(url, handleMain);
  get_obfuscated_url(url, key, (char*)"/command");
  server.on(url, handleCommand);
    get_obfuscated_url(url, key, (char*)"/debug");
  server.on(url, handleDebug);
  get_obfuscated_url(url, key, (char*)"/config");
  server.on(url, handleConfig);
    get_obfuscated_url(url, key, (char*)"/wifi");
  server.on(url, handleWifi);
  get_obfuscated_url(url, key, (char*)"/reseau");
  server.on(url, handleReseau);
  get_obfuscated_url(url, key, (char*)"/wol");
  server.on(url, handleWOL);
  get_obfuscated_url(url, key, (char*)"/actionwol");
  server.on(url, handleActionWOL);
  get_obfuscated_url(url, key, (char*)"/securite");
  server.on(url, handleSecurite);
  get_obfuscated_url(url, key, (char*)"/application");  // adresse d'une application externe => c'est ici que l'on mettra les pramètrage lien sur le site
  server.on(url, handleApplication);
  get_obfuscated_url(url, key, (char*)"/prgmlist");
  server.on(url, handlePrgmList);
  get_obfuscated_url(url, key, (char*)"/prgmdelete");
  server.on(url, handlePrgmDelete);
  get_obfuscated_url(url, key, (char*)"/prgmactive");
  server.on(url, handlePrgmActive);
  get_obfuscated_url(url, key, (char*)"/prgmadd");
  server.on(url, handlePrgmAdd);
  get_obfuscated_url(url, key, (char*)"/prgmupdate");
  server.on(url, handlePrgmUpdate);
  get_obfuscated_url(url, key, (char*)"/attach");
  server.on(url, handleAttach);
  get_obfuscated_url(url, key, (char*)"/clock");
  server.on(url, handleClock);
  get_obfuscated_url(url, key, (char*)"/api");
  server.on(url, handleApi);
  get_obfuscated_url(url, key, (char*)"/reboot");
  server.on(url, rebootESP32);

  server.onNotFound(notFound);

  server.begin();

  write_output_ln("WEBSERVER - ws_config - Web server started");
}

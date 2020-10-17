// NOM DU PROGRAMME : SPIFFS_PROTOTYPE
// REALISATION INFORMATIQUE : Mathieu Lepage
// DATE DE CREATION : 2020/10/16
// OBJET : Programme test permettant d'afficher une valeur enregistrée à partir d'une interface web

#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
 
WiFiManager wm;
 
AsyncWebServer server(80);
 
const char* ssid = "ESP32";
const char* password = "devikit1234";
 
const char* PARAM_TEST = "inputTest";
 
// Une page web HTML qui permet de handle un input field (inputTest)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
 
<head>
    <title>Serveur ESP32</title>
    <meta name="viewport" content="width=device-width, initial-scale=1"charset="UTF-8" />
    <script>
      function submitMessage() {
        setTimeout(function(){ document.location.reload(false); }, 500);   
      }
    </script>
</head>
 
<body>
    <div>
        <form action="/get" target="hidden-form" style="border: 3px groove;padding: 0 20px 20px 20px;width: 350px;margin-bottom:20px;">
            <h2>Prototype du SPIFF</h2>
            <br>
              Champ de test : <input type="text" value="%inputTest%" name="inputTest">
            <br><br><br>
            <input type="submit" value="Appliquer" onclick="submitMessage()">
        </form>
    </div>
    <iframe style="display:none" name="hidden-form"></iframe>
</body>
 
</html>)rawliteral";
 
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Fonction qui permet de lire une valeur encapsulée dans un fichier .txt 
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  return fileContent;
}
 // Fonction qui permet d'encapsuler une valeur dans un fichier .txt
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}
 
// Fonction qui permet de remplacer les placeholder entre % dans la page HTML par les valeurs stockées
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputTest"){
    return readFile(SPIFFS, "/inputTest.txt");
  }
  return String();
}
 
void setup() {
  Serial.begin(9600);
  // Initialize le SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
 // Initialisation du WiFi-Manager
  WiFi.mode(WIFI_STA);
  if(!wm.autoConnect(ssid, password))
		Serial.println("Erreur de connexion.");
	else
		Serial.println("Connexion etablie!");
 
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 
  // Envoie la web page à l'utilisateur
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
 
  // Envoie une GET request à <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputTest
    if (request->hasParam(PARAM_TEST)) {
      inputMessage = request->getParam(PARAM_TEST)->value();
      writeFile(SPIFFS, "/inputTest.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });

  server.onNotFound(notFound);
  // Fonction qui démarre le serveur local
  server.begin();
}
 
void loop() {
  String test = readFile(SPIFFS, "/inputTest.txt");
  //Affichage du champ test.
  Serial.println(test);
  delay(5000);
}
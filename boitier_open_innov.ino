#define DELAY_OPEN_TIME 3000
#define BOITIER_ID 11111111
#define WS_HOST "ADR_BACKEND"
#define WS_PORT "PORT_BACKEND"

const byte PIN_LED_R = 4;
const byte PIN_LED_G = 3;
const byte PIN_LED_B = 2;

const String ssid = "NOM_WIFI";
const String password = "MDP_WIFI";

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  
  initESP8266();

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  displayColor(250, 255, 255);
}

void loop() {
  //Attente message du module HC-05
  while(Serial2.available()){
    //Récupération du token
    String hc05_msg = Serial2.readString();
    Serial.println(hc05_msg);
    String rep = openAuthorization(hc05_msg);
    Serial2.println("OPEN: " + rep);
  }
}

String openAuthorization(String token){
  String request = "GET /boitier/"+String(BOITIER_ID)+"/open HTTP/1.1"
                 + "\r\nHost: " + String(WS_HOST) + ":" + String(WS_PORT)
                 + "\r\nAuthorization: Bearer " + token + "\r\n";
                 
  //Ouverture de la connexion
  sendToESP8266("AT+CIPSTART=1,\"TCP\",\""+ String(WS_HOST) +"\"," + String(WS_PORT), 1000);    
  //Envoie de la requete http           
  sendToESP8266("AT+CIPSEND=1," + String(request.length()+2), 1000);
  String ws_msg = sendToESP8266(request, 1000);
  //Fermeture de la connexion
  sendToESP8266("AT+CIPCLOSE=1", 1000);

  //Recupération du code http
  String codeHttp = getHttpCode(ws_msg);

  //Ouverture si l'utilisateur à l'authorisation
  if(codeHttp.compareTo("200") == 0) open();

  return codeHttp;
}

String getHttpCode(String httpRequest){
  int posSendOk = httpRequest.indexOf("SEND OK");
  int posHttp = httpRequest.indexOf("HTTP", posSendOk);
  int posStart = httpRequest.indexOf(" ", posHttp);
  int posEnd = httpRequest.indexOf(" ", posStart+1);
  return httpRequest.substring(posStart+1, posEnd);
}

void initESP8266(){
  Serial.println("*****************************************");
  Serial.println("*** Initialisation du module ESP8266 ***");
  Serial.println("*****************************************");
  sendToESP8266("AT+CWMODE=1", 2000);
  Serial.println("----------------------------------------");
  sendToESP8266("AT+CWJAP=\""+ssid+"\",\""+password+"\"", 8000);
  Serial.println("----------------------------------------");
  sendToESP8266("AT+CIFSR", 2000);
  Serial.println("----------------------------------------");
  sendToESP8266("AT+CIPMUX=1", 2000);
  Serial.println("*****************************************");
}

String sendToESP8266(String command, const int timeout) {
  //Envoie de la commande
  Serial1.println(command);
  
  String response = "";
  long int time = millis();
  //Attente de la réponse du module
  while ((time + timeout) > millis()) {
    while (Serial1.available()) {
      int c = Serial1.read();
      if(c!=-1) {
        Serial.print((char)c);
        response += (char)c;
      }
    }
    if(response.indexOf("\r\nOK\r\n") != -1) break;
  }

  return response;
}

void open() {
  displayColor(255, 250, 255); //Vert
  delay(DELAY_OPEN_TIME);
  displayColor(250, 255, 255); //Rouge
}

void displayColor(byte r, byte g, byte b) {
  analogWrite(PIN_LED_R, ~r);
  analogWrite(PIN_LED_G, ~g);
  analogWrite(PIN_LED_B, ~b);
}

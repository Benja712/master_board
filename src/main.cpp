#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>

//declaracion de Variables conexion wifi
const char ssid[] = "TP-Link_79F6";
const char pass[]= "26201429";

//Variables para la conexion mqtts
const char* mqtt_server = "18.229.85.142";
const int mqtt_port = 1883;
const char *mqtt_user = "Benjamin_IoT_Electronica";
const char *mqtt_pass = "batman1322benjatheflashDigitales7845Pass";
String id_client = "Secador_Client_";
const int keplive = 60;
const bool secion_clean = true;
const int out = 10000;

//pines del nodeMCU
const byte m0 = 0;
const byte m1 = 2;
const byte m2 = 4;
const byte m3 = 5;
const byte m4 = 12;
const byte m5 = 13;
const byte m6 = 14;
const byte m7 = 15;
const byte counter = 16;
const byte wenable = 10;
const byte buffer = 3;

//variable de tiempo y creacion de clientes wifi y mqtts
unsigned long lastMillis = 0;
WiFiClient net;//cliente Wifi
MQTTClient client; //cliente mqtt

/***********************
Declaracion de Funciones
***********************/
//funcion para conectarse a internet y al brocker mqtt
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  id_client += String(random(0xffff), HEX); //define el client_id random por si llegara a desconectarse
  client.setOptions(keplive, secion_clean, out);//da opciones nesesarias para la conexion mqtt

  while (!client.connect(id_client.c_str(), mqtt_user, mqtt_pass)) {//ciclo while que se detiene hasta que
    Serial.print(".");                                              //nuentro dispositivo se conecte a mqtts
    delay(1000);
  }

  Serial.println("\nconnected!");
  client.subscribe("password/send/part1");//subscripcion al topico send
  client.subscribe("password/send/part2");//subscripcion al topico send
  client.subscribe("password/change/part1"); //subscripcion al topico change
  client.subscribe("password/change/part2"); //subscripcion al topico change
}

//funcion writepin
void writePin(byte pin,  bool state){
  switch (pin) {
    case 0:
        digitalWrite(m0, state);
        break;
    case 1:
        digitalWrite(m1, state);
        break;
    case 2:
        digitalWrite(m2, state);
        break;
    case 3:
        digitalWrite(m3, state);
        break;
    case 4:
        digitalWrite(m4, state);
        break;
    case 5:
        digitalWrite(m5, state);
        break;
    case 6:
        digitalWrite(m6, state);
        break;
    case 7:
        digitalWrite(m7, state);
        break;
    default: break;
  }
}

//funcion de escritura de la memoria
void write(char type, byte pin){
  switch (type) {
    case '0':
      writePin(pin, LOW);
      break;
    case '1':
      writePin(pin, HIGH);
      break;
    default: break;
  }
}

//funcion para el control del mensaje separando caracter por caracter
void messageRecognition (String message){
  message.trim();
  byte str = message.length();
  char messageChar[str];
  unsigned int bufsize = str;
  message.toCharArray(messageChar, bufsize);
  for(byte i = 0; i < str; i++){
    write(message[i], i);
  }
  delay(500);
  digitalWrite(wenable, LOW);
  delay(500);
  digitalWrite(wenable, HIGH);
  delay(100);
  digitalWrite(counter, LOW);
  delay(100);
  digitalWrite(counter, HIGH);
}

//funcion para el control del mensaje separando en areglos de caracteres
void processMsg(String msg ){
  msg.trim();
  int index;
  String characters[8];
  for(size_t i = 0; i < 8; i++){
    index = msg.indexOf('_');
    characters[i] = msg.substring(0, index);
    msg = msg.substring(index + 1);
    Serial.print("=> ");
    Serial.println(characters[i]);
    messageRecognition(characters[i]);
  }
}

//funcion que se manda a llamar cuando un mensaje del brocker llega
void messageReceived(String &topic, String &payload) {
  if(topic == "password/change/part1"){
    Serial.println("incoming: " + topic + " - " + payload);
    digitalWrite(buffer, HIGH);
    processMsg(payload);
    digitalWrite(buffer, LOW);
  }
  if(topic == "password/change/part2"){
    Serial.println("incoming: " + topic + " - " + payload);
    digitalWrite(buffer, HIGH);
    processMsg(payload);
    digitalWrite(buffer, LOW);
  }
  if(topic == "password/send/part1"){
    Serial.println("incoming: " + topic + " - " + payload);
  }
  if(topic == "password/send/part2"){
    Serial.println("incoming: " + topic + " - " + payload);
  }
}

/*******************
cliclos setup y loop
*******************/
void setup() {
  Serial.begin(115200);
  pinMode(m0, OUTPUT);
  pinMode(m1, OUTPUT);
  pinMode(m2, OUTPUT);
  pinMode(m3, OUTPUT);
  pinMode(m4, OUTPUT);
  pinMode(m5, OUTPUT);
  pinMode(m6, OUTPUT);
  pinMode(m7, OUTPUT);
  pinMode(counter, OUTPUT);
  pinMode(buffer, OUTPUT);
  pinMode(wenable, OUTPUT);
  digitalWrite(wenable, HIGH);
  digitalWrite(counter, HIGH);
  digitalWrite(buffer, LOW);
  WiFi.begin(ssid, pass);
  client.begin(mqtt_server, mqtt_port, net);//establece los parametros de la ubicacion del servidor mqtt
  client.onMessage(messageReceived);
  connect();
}



void loop() {
  client.loop();
  delay(10);

  if (!client.connected()) {//si nos desconectamos o perdemos la conexion se vuelve a intentar hasta que
    connect();              //nos volvamos a conectar
  }
}

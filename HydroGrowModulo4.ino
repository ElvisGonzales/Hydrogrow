//BLYNK
#define BLYNK_TEMPLATE_ID "TMPL2bP9IesvB"
#define BLYNK_TEMPLATE_NAME "HYDROGROW"
#define BLYNK_AUTH_TOKEN "R4XArBgxq7HViZeI88J9KqUhB86c73pb"
#define BLYNK_PRINT Serial

//FIREBASE ELVIS
#define FIREBASE_HOST "hydrogrowtesis-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ac0YAOhFe2NEsA2FDD6CVcCBhhWQnWTkLAMuPsQR"
//-------------------
// #define WIFI_SSID "QP_COM"
// #define WIFI_PASSWORD "1207454172Eg"

//LIBRERIAS
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include "DHTesp.h"
#include "LiquidCrystal_I2C.h"

// Definir los nuevos pines I2C
// #define SDA_PIN 13  // GPIO0 (D7)
// #define SCL_PIN 15  // GPIO2 (D8)

//VARIABLES
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "QP_COM";
char pass[] = "1207454172Eg";
SimpleTimer timer;

#define LlenadoAgua D3
int SensorArriba = D5;
int SensorAbajo = D6;
int sensorValue1;
int sensorValue2;
int valorAgua;

//-------------------

volatile bool estadoLlenadoAgua = false;

FirebaseData firebaseData;
String ruta = "HydroGrow", path = "/";
String ruta2 = "RealTime";
String path2 = "RealTime";


BLYNK_WRITE(V6) {
  int value = 0;
  value = param.asInt();
  Serial.println(value);
  if (value == 1) {
    digitalWrite(LlenadoAgua, LOW);
    Firebase.pushBool(firebaseData, ruta + "/LLENADO DE AGUA", true);
    Firebase.setBool(firebaseData, ruta2 + "/LLENADO DE AGUA", true);
    Serial.println("LLENADO DE AGUA ENCENDIDO");
  }
  if (value == 0) {
    digitalWrite(LlenadoAgua, HIGH);
    Firebase.pushBool(firebaseData, ruta + "/LLENADO DE AGUA", false);
    Firebase.setBool(firebaseData, ruta2 + "/LLENADO DE AGUA", false);
    Serial.println("LLENADO DE AGUA APAGADO");
  }
}

void setup() {
  Serial.begin(115200);
  DS18B20.begin();
  Wire.begin();
  lightMeter.begin();
  lcdSetup();
  Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(LlenadoAgua, OUTPUT);
  pinMode(SensorArriba, INPUT);
  pinMode(SensorAbajo, INPUT);
  timer.setInterval(1000L, getDataSensores);  // new data will be updated every 1 sec
  delay(1000L);
}

void loop() {
  actualizar_estadoLlenadoAgua();
  PrenderApagarLlenadoAgua();
  Blynk.run();
  timer.run();  // Initiates SimpleTimer
}

void PrenderApagarLlenadoAgua() {
  if (estadoLlenadoAgua) {
    digitalWrite(LlenadoAgua, LOW);
  } else {
    digitalWrite(LlenadoAgua, HIGH);
  }
}

void getDataSensores() {

  sensorValue1 = digitalRead(SensorArriba);
  sensorValue2 = digitalRead(SensorAbajo);
  if (SensorAbajo == 0) {
    valorAgua = 1;
    Serial.println("NIVEL DEL AGUA ALTO");
  }
  if (SensorAbajo == 1) {
    valorAgua = 0;
    Serial.println("NIVEL DEL AGUA BAJO");
  }
  Blynk.virtualWrite(V9, valorAgua);  //
  Firebase.pushInt(firebaseData, ruta + "/SENSOR DE AGUA ARRIBA", sensorValue1);
  Firebase.setInt(firebaseData, ruta2 + "/SENSOR DE AGUA ARRIBA", sensorValue1);
  Firebase.pushInt(firebaseData, ruta + "/SENSOR DE AGUA ABAJO", sensorValue2);
  Firebase.setInt(firebaseData, ruta2 + "/SENSOR DE AGUA ABAJO", sensorValue2);
  if (sensorValue1 == 1 && sensorValue2 == 1) {
    while (sensorValue2 != 0 || sensorValue1 != 0) {
      digitalWrite(LlenadoAgua, LOW);
      sensorValue1 = digitalRead(SensorArriba);
      sensorValue2 = digitalRead(SensorAbajo);
      Firebase.pushInt(firebaseData, ruta + "/SENSOR DE AGUA ARRIBA", sensorValue1);
      Firebase.setInt(firebaseData, ruta2 + "/SENSOR DE AGUA ARRIBA", sensorValue1);
      Firebase.pushInt(firebaseData, ruta + "/SENSOR DE AGUA ABAJO", sensorValue2);
      Firebase.setInt(firebaseData, ruta2 + "/SENSOR DE AGUA ABAJO", sensorValue2);
      Serial.println("BOMBA DE LLENADO DE AGUA ENCENDIDA :-) ");
    }
  } else {
    digitalWrite(LlenadoAgua, HIGH);
    Serial.println("BOMBA DE LLENADO DE AGUA APAGADA :-() ");
  }
  Serial.print("Sensor Arriba: ");
  Serial.println(sensorValue1);
  Serial.print("Sensor Abajo: ");
  Serial.println(sensorValue2);
  delay(1000);
}

void actualizar_estadoLlenadoAgua() {
  if (Firebase.getBool(firebaseData, path2 + "/LLENADO DE AGUA")) {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.print("VALUE: ");
    estadoLlenadoAgua = firebaseData.boolData();
    Serial.println(estadoLlenadoAgua);
    Serial.println("------------------------------------");
    Serial.println();
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
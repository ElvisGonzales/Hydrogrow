//BLYNK MODULO 1
#define BLYNK_TEMPLATE_ID "TMPL2bP9IesvB"
#define BLYNK_TEMPLATE_NAME "HYDROGROW"
#define BLYNK_AUTH_TOKEN "R4XArBgxq7HViZeI88J9KqUhB86c73pb"
#define BLYNK_PRINT Serial

//FIREBASE ELVIS
#define FIREBASE_HOST "hydrogrowtesis-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ac0YAOhFe2NEsA2FDD6CVcCBhhWQnWTkLAMuPsQR"

//LIBRERIAS
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <MQ135.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include "DHTesp.h"
#include "LiquidCrystal_I2C.h"
#include <WiFiClient.h>

//VARIABLES
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "QP_COM";
char pass[] = "1207454172Eg";
SimpleTimer timer;
int mq135 = A0;  // smoke sensor is connected with the analog pin A0
int ppm = 0;
#define DHTpin 0 //D5 of NodeMCU is GPIO14
// #define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board "D4 pin on the ndoemcu Module"
// OneWire oneWire(ONE_WIRE_BUS);
// DallasTemperature DS18B20(&oneWire);

float temp;
float lux;
BH1750 lightMeter;
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHTesp dht;
int ph_pin = A0;  //
int m_4 = 623;
int m_7 = 605;  //agua
float t;
float h;
float Po;
#define LucesLed D5
// #define NutrienteNitrogenoFosforo D1
// #define LlenadoAgua D3
// int SensorArriba = D5;
// int SensorAbajo = D6;
// int sensorValue1;
// int sensorValue2;
// int valorAgua;
//-------------------
namespace pin {
const byte tds_sensor = A0;
const byte one_wire_bus = D4; // Dallas Temperature Sensor
}

namespace device {
float aref = 3.3; // Vref, this is for 3.3v compatible controller boards, for arduino use 5.0v.
}

namespace sensor {
float ec = 0;
unsigned int tds = 0;
float waterTemp = 0;
float ecCalibration = 1;
}
OneWire oneWire(pin::one_wire_bus);
DallasTemperature dallasTemperature(&oneWire);
//-------------------
volatile bool estadoLucesLed = false;
// volatile bool estadoNutrienteNitroFosf = false;
// volatile bool estadoLlenadoAgua = false;

FirebaseData firebaseData;
String ruta = "HydroGrow", path = "/";
String ruta2 = "RealTime";
String path2 = "RealTime";

BLYNK_WRITE(V5) { //LUCES LED
  int value = 0;
  value = param.asInt();
  Serial.println(value);
  if (value == 1) {
    digitalWrite(LucesLed, LOW);
    Firebase.pushBool(firebaseData, ruta + "/LUCES LED", true);
    Firebase.setBool(firebaseData, ruta2 + "/LUCES LED", true);
    Serial.println("LUCES LED ENCENDIDAS");
  }
  if (value == 0) {
    digitalWrite(LucesLed, HIGH);
    Firebase.pushBool(firebaseData, ruta + "/LUCES LED", false);
    Firebase.setBool(firebaseData, ruta2 + "/LUCES LED", false);
    Serial.println("LUCES LED APAGADAS");
  }
}

void setup() {
  Serial.begin(115200);
  // DS18B20.begin();
  dallasTemperature.begin();
  Wire.begin();
  lightMeter.begin();
  lcd.init();
  lcd.backlight();
  Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.setup(DHTpin, DHTesp::DHT11); // for DHT11 Connect DHT sensor to GPIO 17
  lcdSetup();
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(LucesLed, OUTPUT);
  // pinMode(NutrienteNitrogenoFosforo, OUTPUT);
  // pinMode(LlenadoAgua, OUTPUT);
  // pinMode(SensorArriba, INPUT);
  // pinMode(SensorAbajo, INPUT);
  cestorePrint();
  lcd.clear();
  timer.setInterval(1000L, getDataDHT11);  // new data will be updated every 1 sec
  timer.setInterval(1000L, getSendDataBH1750);   // new data will be updated every 1 sec
  // timer.setInterval(1000L, getSendDataPH);    // new data will be updated every 1 sec
  // timer.setInterval(1000L, getDataSensores);  // new data will be updated every 1 sec
  // timer.setInterval(1000L, getSendDataMQ135);    // new data will be updated every 1 sec
  //timer.setInterval(1000L, getSendDataDS18B20);  // new data will be updated every 1 sec
  delay(1000L);
}

void loop() {
  
  PrenderApagarLucesLed();
  actualizar_estadoLucesLed();
  Blynk.run();
  timer.run();  // Initiates SimpleTimer
  readTdsQuick();
  delay(1000);
  // PrenderApagarNutrientePotasio();
  // actualizar_estadoNutrientePotasio();
  // actualizar_estadoLlenadoAgua();
  // PrenderApagarLlenadoAgua();
  // PrenderApagarNutrienteNitrogenoFosforo();
  // actualizar_estadoNutrienteNitrogenoFosforo();
}

void getDataDHT11(){
  t = dht.getTemperature();
  h = dht.getHumidity();
  Serial.print("Temperatura Aire: ");
  Serial.println(t, 1);
  Serial.print("Humedad: ");
  Serial.println(h, 1);
  Serial.print("Luminosidad: ");
  Serial.println(lux, 1);
  Serial.println("-----------------"); // Separador
  Blynk.virtualWrite(V0, t);  //
  Blynk.virtualWrite(V1, h);  //
  Firebase.pushInt(firebaseData, ruta + "/TEMPERATURA AIRE", t);
  Firebase.setInt(firebaseData, ruta2 + "/TEMPERATURA AIRE", t);
  Firebase.pushInt(firebaseData, ruta + "/HUMEDAD AIRE", h);
  Firebase.setInt(firebaseData, ruta2 + "/HUMEDAD AIRE", h);
  delay(5000);
}



void getSendDataBH1750() {
  lux = lightMeter.readLightLevel();
  Blynk.virtualWrite(V3, lux);
  Firebase.pushInt(firebaseData, ruta + "/LUMINOSIDAD", lux);
  Firebase.setInt(firebaseData, ruta2 + "/LUMINOSIDAD", lux);
  Serial.print("LUMINOSIDAD: ");
  Serial.print(lux);
  Serial.println(" lx");
  if (temp > 8000) {
    Firebase.pushInt(firebaseData, ruta + "/LUMINOSIDAD", lux);
    Firebase.setInt(firebaseData, ruta2 + "/LUMINOSIDAD", lux);

    Serial.print("LUMINOSIDAD MUY ALTA: ");
    Serial.print(lux);
    Serial.println(" lx");
  }
}

void readTdsQuick() {
  dallasTemperature.requestTemperatures();
  sensor::waterTemp = dallasTemperature.getTempCByIndex(0);
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (sensor::waterTemp - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  sensor::ec = (rawEc / temperatureCoefficient) * sensor::ecCalibration; // temperature and calibration compensation
  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec) * 0.5; //convert voltage value to tds value
  Serial.print(F("TDS:")); Serial.println(sensor::tds);
  Serial.print(F("Temperature:")); Serial.println(sensor::waterTemp,2);
  Blynk.virtualWrite(V4,(sensor::tds));
  Blynk.virtualWrite(V2,(sensor::waterTemp));
  Firebase.pushInt(firebaseData, ruta + "/TEMPERATURA AGUA", sensor::waterTemp);
  Firebase.setInt(firebaseData, ruta2 + "/TEMPERATURA AGUA", sensor::waterTemp);
  Firebase.pushInt(firebaseData, ruta + "/TDS AGUA", sensor::tds);
  Firebase.setInt(firebaseData, ruta2 + "/TDS AGUA", sensor::tds);
  Serial.print("TEMPERATURA AGUA: ");
  Serial.print(sensor::waterTemp);
  Serial.println(" C");
  Serial.print("TDS AGUA: ");
  Serial.print(sensor::tds);
  Serial.println(" PPM");
}

// void getSendDataDS18B20() {
//   DS18B20.requestTemperatures();
//   temp = DS18B20.getTempCByIndex(0);  // Celcius
//   Blynk.virtualWrite(V2, temp);
//   Firebase.pushInt(firebaseData, ruta + "/TEMPERATURA AGUA", temp);
//   Firebase.setInt(firebaseData, ruta2 + "/TEMPERATURA AGUA", temp);
//   Serial.print("TEMPERATURA AGUA: ");
//   Serial.print(temp);
//   Serial.println(" C");
//   if (temp > 150) {
//     Firebase.pushInt(firebaseData, ruta + "/TEMPERATURA AGUA", temp);
//     Firebase.setInt(firebaseData, ruta2 + "/TEMPERATURA AGUA", temp);
//     Serial.print("TEMPERATURA AGUA MUY ALTA: ");
//     Serial.print(temp);
//     Serial.println(" ");
//   }
// }

// void getSendDataMQ135() {
//   ppm = analogRead(mq135);
//   Blynk.virtualWrite(V4, ppm);  //
//   Firebase.pushInt(firebaseData, ruta + "/CALIDAD DEL AIRE", ppm);
//   Firebase.setInt(firebaseData, ruta2 + "/CALIDAD DEL AIRE", ppm);
//   Serial.print("NIVEL DE CO2: ");
//   Serial.print(ppm);
//   Serial.println(" ");
//   if (ppm > 500) {
//     Firebase.pushInt(firebaseData, ruta + "/CALIDAD DEL AIRE", ppm);
//     Firebase.setInt(firebaseData, ruta2 + "/CALIDAD DEL AIRE", ppm);
//     Serial.print("CALIDAD DEL AIRE MUY ALTA: ");
//     Serial.print(ppm);
//     Serial.println(" ");
//   }
// }

void PrenderApagarLucesLed() {
  if (estadoLucesLed) {
    digitalWrite(LucesLed, LOW);
  } else {
    digitalWrite(LucesLed, HIGH);
  }
}

void actualizar_estadoLucesLed() {
  if (Firebase.getBool(firebaseData, path2 + "/LUCES LED")) {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.print("VALUE: ");
    estadoLucesLed = firebaseData.boolData();
    Serial.println(estadoLucesLed);
    Serial.println("------------------------------------");
    Serial.println();
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void lcdSetup() {
  lcd.init();
  lcd.backlight();
  cestorePrint();
  lcd.setCursor(0, 0);
  lcd.print("Tem:");
  lcd.setCursor(0, 1);
  lcd.print("Hum:");
  lcd.setCursor(0, 2);
  lcd.print("Lx:");
}

void clearData() {
  lcd.setCursor(11, 0);
  lcd.print("          "); // 10 Spaces to clear the humidity data
  lcd.setCursor(11, 1);
  lcd.print("        "); // 10 Spaces to clear the temperature data
  lcd.setCursor(11, 2);
  lcd.print("   "); // 10 Spaces to clear the humidity data
}

void cestorePrint() {
  lcd.setCursor(5, 0);
  lcd.print("Bienvenidos");
  lcd.setCursor(10, 1);
  lcd.print("a");
  lcd.setCursor(6, 2);
  lcd.print("HYDROGROW");
  delay(5000);
  lcd.clear();
}
// BLYNK_WRITE(V6) {
//   int value = 0;
//   value = param.asInt();
//   Serial.println(value);
//   if (value == 1) {
//     digitalWrite(LlenadoAgua, LOW);
//     Firebase.pushBool(firebaseData, ruta + "/LLENADO DE AGUA ", true);
//     Firebase.setBool(firebaseData, ruta2 + "/LLENADO DE AGUA ", true);
//     Serial.println("LLENADO DE AGUA ENCENDIDO");
//   }
//   if (value == 0) {
//     digitalWrite(LlenadoAgua, HIGH);
//     Firebase.pushBool(firebaseData, ruta + "/LLENADO DE AGUA ", false);
//     Firebase.setBool(firebaseData, ruta2 + "/LLENADO DE AGUA ", false);
//     Serial.println("LLENADO DE AGUA APAGADO");
//   }
// }

// BLYNK_WRITE(V8) {
//   int value = 0;
//   value = param.asInt();
//   Serial.println(value);
//   if (value == 1) {
//     digitalWrite(NutrientePotasio, LOW);
//     Firebase.pushBool(firebaseData, ruta + "/NUTRIENTE POTASIO ", true);
//     Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTE POTASIO ", true);
//     Serial.println("NUTRIENTE POTASIO ABIERTO");
//   }
//   if (value == 0) {
//     digitalWrite(NutrientePotasio, HIGH);
//     Firebase.pushBool(firebaseData, ruta + "/NUTRIENTE POTASIO ", false);
//     Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTE POTASIO ", false);
//     Serial.println("NUTRIENTE POTASIO CERRADO");
//   }
// }
// void PrenderApagarLlenadoAgua() {
//   if (estadoLlenadoAgua) {
//     digitalWrite(LlenadoAgua, LOW);
//   } else {
//     digitalWrite(LlenadoAgua, HIGH);
//   }
// }

// void PrenderApagarNutrientePotasio() {
//   if (estadoNutrientePotasio) {
//     digitalWrite(NutrientePotasio, LOW);
//   } else {
//     digitalWrite(NutrientePotasio, HIGH);
//   }
// }

// void getDataSensores() {
//   //Auxiliar para probar el resto
//   SensorArriba=0;
//   SensorAbajo=1;
//   //Eliminar lo de arriba despues de implementar 
//   sensorValue1 = digitalRead(SensorArriba);
//   sensorValue2 = digitalRead(SensorAbajo);
//   if (SensorAbajo == 0) {
//     valorAgua = 1;
//     Serial.println("NIVEL DEL AGUA ALTO");
//   }
//   if (SensorAbajo == 1) {
//     valorAgua = 0;
//     Serial.println("NIVEL DEL AGUA BAJO");
//   }
//   Blynk.virtualWrite(V9, valorAgua);  //
//   Firebase.pushInt(firebaseData, ruta + "/NIVEL DE AGUA ARRIBA ", sensorValue1);
//   Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE AGUA ARRIBA ", sensorValue1);
//   Firebase.pushInt(firebaseData, ruta + "/NIVEL DE AGUA ABAJO ", sensorValue2);
//   Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE AGUA ABAJO ", sensorValue2);
//   if (sensorValue1 == 1 && sensorValue2 == 1) {
//     while (sensorValue2 != 0 || sensorValue1 != 0) {
//       digitalWrite(LlenadoAgua, LOW);
//       sensorValue1 = digitalRead(SensorArriba);
//       sensorValue2 = digitalRead(SensorAbajo);
//       Firebase.pushInt(firebaseData, ruta + "/NIVEL DE AGUA ARRIBA ", sensorValue1);
//       Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE AGUA ARRIBA ", sensorValue1);
//       Firebase.pushInt(firebaseData, ruta + "/NIVEL DE AGUA ABAJO ", sensorValue2);
//       Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE AGUA ABAJO ", sensorValue2);
//       Serial.println("BOMBA DE LLENADO DE AGUA ENCENDIDA :-) ");
//     }
//   } else {
//     digitalWrite(LlenadoAgua, HIGH);
//     Serial.println("BOMBA DE LLENADO DE AGUA APAGADA :-() ");
//   }
//   Serial.print("Sensor Arriba: ");
//   Serial.println(sensorValue1);
//   Serial.print("Sensor Abajo: ");
//   Serial.println(sensorValue2);
//   delay(1000);
// }
// void getSendDataPH() {
//   int measure = 0;
//   int prom = 0;
//   for (int i = 0; i < 20; i++) {
//     measure = analogRead(ph_pin);
//     prom = prom + measure;
//     delay(50);
//   }
//   prom = prom / 20;
//   Serial.print("Medida: ");
//   Serial.print(prom);
//   //calibracion
//   Po = 7 + ((measure - m_7) * 3 / (m_7 - m_4));
//   Serial.print("\tPH: ");
//   Serial.print(Po, 2);
//   Serial.println("");
//   Blynk.virtualWrite(V3, Po);  //
//   Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE PH ", Po);
//   Firebase.pushInt(firebaseData, ruta + "/NIVEL DE PH ", Po);
//   Serial.print("NIVEL DE PH: ");
//   Serial.print(Po);
//   Serial.println(" ");
//   if (Po > 10) {
//     //Firebase.setInt(firebaseData, ruta + "/NIVEL DE PH ", Po);
//     Serial.print("PH DEL AGUA MUY ALTO: ");
//     Serial.print(Po);
//     Serial.println(" ");
//   } else if (Po > 4) {
//     //Firebase.setInt(firebaseData, ruta + "/NIVEL DE PH ", Po);
//     Serial.print("PH DEL AGUA MUY BAJO: ");
//     Serial.print(Po);
//     Serial.println(" ");
//   }
// }

// void actualizar_estadoLlenadoAgua() {
//   if (Firebase.getBool(firebaseData, path2 + "/LLENADO DE AGUA ")) {
//     Serial.println("PASSED");
//     Serial.println("PATH: " + firebaseData.dataPath());
//     Serial.println("TYPE: " + firebaseData.dataType());
//     Serial.println("ETag: " + firebaseData.ETag());
//     Serial.print("VALUE: ");
//     estadoLlenadoAgua = firebaseData.boolData();
//     Serial.println(estadoLlenadoAgua);
//     Serial.println("------------------------------------");
//     Serial.println();
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + firebaseData.errorReason());
//     Serial.println("------------------------------------");
//     Serial.println();
//   }
// }

// void actualizar_estadoNutrienteNitrogenoFosforo() {
//   if (Firebase.getBool(firebaseData, path2 + "/NUTRIENTE NITROGENO FOSFORO ")) {
//     Serial.println("PASSED");
//     Serial.println("PATH: " + firebaseData.dataPath());
//     Serial.println("TYPE: " + firebaseData.dataType());
//     Serial.println("ETag: " + firebaseData.ETag());
//     Serial.print("VALUE: ");
//     estadoNutrienteNitroFosf = firebaseData.boolData();
//     Serial.println(estadoNutrienteNitroFosf);
//     Serial.println("------------------------------------");
//     Serial.println();
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + firebaseData.errorReason());
//     Serial.println("------------------------------------");
//     Serial.println();
//   }
// }



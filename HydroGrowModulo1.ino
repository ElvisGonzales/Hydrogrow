//BLYNK MODULO 1
#define BLYNK_TEMPLATE_ID "TMPL2mn8q-oX2"
#define BLYNK_TEMPLATE_NAME "HYDROGROW MODULO 1"
#define BLYNK_AUTH_TOKEN "KuaEwoBAhtfpy44t6J0oOjStss-tZ_F6"
#define BLYNK_PRINT Serial

//FIREBASE ELVIS
#define FIREBASE_HOST "hydrogrowtesis-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ac0YAOhFe2NEsA2FDD6CVcCBhhWQnWTkLAMuPsQR"
//-------------------
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
#include "RTClib.h"

//VARIABLES
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "QP_COM";
char pass[] = "1207454172Eg";
SimpleTimer timer;
int mq135 = A0;  // smoke sensor is connected with the analog pin A0
int ppm = 0;
#define FlujoAgua D3
#define TRIG_PIN D4
#define ECHO_PIN D5
#define DHTpin 0        //D5 of NodeMCU is GPIO14
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board "D4 pin on the ndoemcu Module"
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float temp;
float lux;
BH1750 lightMeter;
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);
DHTesp dht;
int ph_pin = A0;  //
int m_4 = 623;
int m_7 = 605;  //agua
float t;
float h;
float Po;
// #define LucesLed D5
char daysOfTheWeek[7][12] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
char MonthOfTheYear[12][12] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre" };
int Hora = 0, Minuto = 0, Segundo = 0, anio = 0, mes = 0, dia = 0;
// #define NutrienteNitrogenoFosforo D1
// #define LlenadoAgua D3
// int SensorArriba = D5;
// int SensorAbajo = D6;
// int sensorValue1;
// int sensorValue2;
// int valorAgua;

//-------------------
//volatile bool estadoLucesLed = false;
// volatile bool estadoNutrienteNitroFosf = false;
// volatile bool estadoLlenadoAgua = false;
volatile bool estadoFlujoAgua = false;
const int TANK_HEIGHT = 60;     // Altura del tanque en cm
const int EMPTY_THRESHOLD = 5;  // Valor para considerar el tanque vacío
int timeRange = 0; // in minutes
int interval = 0; // in minutes
// Variables para el control de la bomba
unsigned long previousMillis = 0; // Almacena el último tiempo en que se encendió/apagó la bomba
bool isPumpOn = false; // Estado de la bomba
unsigned long pumpDuration = 0; // Duración de la bomba en milisegundos
unsigned long pumpInterval = 0; // Intervalo de encendido en milisegundos


FirebaseData firebaseData;
String ruta = "HydroGrow", path = "/";
String ruta2 = "RealTime";
String path2 = "RealTime";
String rutaVariables= "Configuracion";
String pathVariables = "Configuracion";

// Firebase Variables
String pathTimeRange = rutaVariables + "/Lapso_Min";
String pathInterval = rutaVariables + "/Intervalo_Hora";

BLYNK_WRITE(V0) {
  int value = 0;
  value = param.asInt();
  Serial.println(value);
  if (value == 1) {
    digitalWrite(FlujoAgua, LOW);
    Firebase.pushBool(firebaseData, ruta + "/FLUJO DE AGUA", true);
    Firebase.setBool(firebaseData, ruta2 + "/FLUJO DE AGUA", true);
    Serial.println("FLUJO DE AGUA ENCENDIDO");
  }
  if (value == 0) {
    digitalWrite(FlujoAgua, HIGH);
    Firebase.pushBool(firebaseData, ruta + "/FLUJO DE AGUA", false);
    Firebase.setBool(firebaseData, ruta2 + "/FLUJO DE AGUA", false);
    Serial.println("FLUJO DE AGUA APAGADO");
  }
}

void setup() {
  Serial.begin(115200);
  DS18B20.begin();
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
  Serial.println("RTC lost power, let's set the time!");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.setup(DHTpin, DHTesp::DHT11);  // for DHT11 Connect DHT sensor to GPIO 17
  // lcdSetup();
  pinMode(A0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(FlujoAgua, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // pinMode(NutrienteNitrogenoFosforo, OUTPUT);
  // pinMode(LlenadoAgua, OUTPUT);
  // pinMode(SensorArriba, INPUT);
  // pinMode(SensorAbajo, INPUT);
  // cestorePrint();
  // lcd.clear();
  // timer.setInterval(1000L, getSendDataPH);    // new data will be updated every 1 sec
  // timer.setInterval(1000L, getDataSensores);  // new data will be updated every 1 sec
  // timer.setInterval(1000L, getDataDHT11);        // new data will be updated every 1 sec
  timer.setInterval(1000L, getSendDataMQ135);  // new data will be updated every 1 sec
  // timer.setInterval(1000L, getSendDataBH1750);   // new data will be updated every 1 sec
  // timer.setInterval(1000L, getSendDataDS18B20);  // new data will be updated every 1 sec
  // timer.setInterval(1000L, getRtc);  // new data will be updated every 1 sec
  delay(1000L);
}

void loop() {

  readFirebaseValues();
  controlarBomba();
  actualizar_estadoFlujoAgua();
  PrenderApagarFlujoAgua();
  SetRtc();
  SetJSN();
  Blynk.run();
  timer.run();  // Initiates SimpleTimer
  // PrenderApagarNutrientePotasio();
  // actualizar_estadoNutrientePotasio();
  // actualizar_estadoLlenadoAgua();
  // PrenderApagarLlenadoAgua();
  // PrenderApagarNutrienteNitrogenoFosforo();
  // actualizar_estadoNutrienteNitrogenoFosforo();
  delay(500);  // Reducido para mejorar la respuesta del sistema
}

void SetJSN() {
  // Envía un pulso de 10us al pin TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Lee el tiempo que toma el pulso en llegar de vuelta
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calcula la distancia en cm
  long distance = duration * 0.034 / 2;

  // Calcula el nivel de agua (altura del tanque - distancia medida)
  long waterLevel = TANK_HEIGHT - distance;

  // Calcula el porcentaje de llenado del tanque
  float percentage = ((float)waterLevel / TANK_HEIGHT) * 100;

  // Imprime los valores en el monitor serial
  Blynk.virtualWrite(V2, percentage);
  Firebase.pushInt(firebaseData, ruta + "/DISTANCIA DEL AGUA", percentage);
  Firebase.setInt(firebaseData, ruta2 + "/DISTANCIA DEL AGUA", percentage);
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.print(" cm, Nivel de Agua: ");
  Serial.print(waterLevel);
  Serial.print(" cm, Porcentaje: ");
  Serial.print(percentage);
  Serial.println(" %");
}

void getRtc() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void getSendDataMQ135() {
  ppm = analogRead(mq135);
  Blynk.virtualWrite(V1, ppm);  //
  Firebase.pushInt(firebaseData, ruta + "/CALIDAD DEL AIRE", ppm);
  Firebase.setInt(firebaseData, ruta2 + "/CALIDAD DEL AIRE", ppm);
  Serial.print("NIVEL DE CO2: ");
  Serial.print(ppm);
  Serial.println(" ");
  if (ppm > 500) {
    Firebase.pushInt(firebaseData, ruta + "/CALIDAD DEL AIRE", ppm);
    Firebase.setInt(firebaseData, ruta2 + "/CALIDAD DEL AIRE", ppm);
    Serial.print("CALIDAD DEL AIRE MUY ALTA: ");
    Serial.print(ppm);
    Serial.println(" ");
  }
}

void SetRtc() {
  DateTime now = rtc.now();
  Hora = now.hour();
  Minuto = now.minute();
  Segundo = now.second();
  anio = now.year();
  mes = now.month();
  dia = now.day();
  Serial.print(dia);
  Serial.print(mes);
  Serial.print(anio);
  Serial.print(' ');
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(' ');
  Serial.print(now.day());
  Serial.print(" de ");
  Serial.print(MonthOfTheYear[now.month()]);
  Serial.print(" del ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void PrenderApagarFlujoAgua() {
  if (estadoFlujoAgua) {
    digitalWrite(FlujoAgua, LOW);
  } else {
    digitalWrite(FlujoAgua, HIGH);
  }
}

void actualizar_estadoFlujoAgua() {
  if (Firebase.getBool(firebaseData, path2 + "/FLUJO DE AGUA")) {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.print("VALUE: ");
    estadoFlujoAgua = firebaseData.boolData();
    Serial.println(estadoFlujoAgua);
    Serial.println("------------------------------------");
    Serial.println();
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

// Function to read values from Firebase
void readFirebaseValues() {
    if (Firebase.getInt(firebaseData, pathTimeRange)) {
        timeRange = firebaseData.intData();
        Serial.print("Time Range: ");
        Serial.println(timeRange);
    }
    if (Firebase.getInt(firebaseData, pathInterval)) {
        interval = firebaseData.intData();
        Serial.print("Interval: ");
        Serial.println(interval);
    }
}

//funcion de controlar el horario de prendido de la bomba
void controlarBomba() {
    // Obtiene la hora actual
    DateTime now = rtc.now();
    int currentHour = now.hour();
    int currentMinute = now.minute();

    // Verifica si es hora de encender la bomba
    if (currentMinute % interval == 0 && !isPumpOn) {
        // Enciende la bomba
        digitalWrite(FlujoAgua, LOW); // Encender la bomba
        isPumpOn = true; // Actualiza el estado de la bomba
        previousMillis = millis(); // Reinicia el temporizador
        pumpDuration = timeRange * 60000; // Convierte minutos a milisegundos
        Firebase.pushBool(firebaseData, ruta + "/FLUJO DE AGUA", true);
        Serial.println("FLUJO DE AGUA ENCENDIDO");
    }

    // Controla la duración de la bomba
    if (isPumpOn) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= pumpDuration) {
            // Apaga la bomba
            digitalWrite(FlujoAgua, HIGH); // Apagar la bomba
            isPumpOn = false; // Actualiza el estado de la bomba
            Firebase.pushBool(firebaseData, ruta + "/FLUJO DE AGUA", false);
            Serial.println("FLUJO DE AGUA APAGADO");
        }
    }
}

// void lcdSetup() {
//   lcd.init();
//   lcd.backlight();
//   cestorePrint();
//   lcd.setCursor(0, 0);
//   lcd.print("Tem:");
//   lcd.setCursor(0, 1);
//   lcd.print("Hum:");
//   lcd.setCursor(0, 2);
//   lcd.print("Lx:");
// }

// void clearData() {
//   lcd.setCursor(11, 0);
//   lcd.print("          ");  // 10 Spaces to clear the humidity data
//   lcd.setCursor(11, 1);
//   lcd.print("        ");  // 10 Spaces to clear the temperature data
//   lcd.setCursor(11, 2);
//   lcd.print("   ");  // 10 Spaces to clear the humidity data
// }

// void cestorePrint() {
//   lcd.setCursor(5, 0);
//   lcd.print("Bienvenidos");
//   lcd.setCursor(10, 1);
//   lcd.print("a");
//   lcd.setCursor(6, 2);
//   lcd.print("HYDROGROW");
//   delay(5000);
//   lcd.clear();
// }

// void getDataDHT11() {
//   t = dht.getTemperature();
//   h = dht.getHumidity();
//   Serial.print("Temperatura Aire: ");
//   Serial.println(t, 1);
//   Serial.print("Humedad: ");
//   Serial.println(h, 1);
//   Serial.print("Luminosidad: ");
//   Serial.println(lux, 1);
//   Serial.println("-----------------");  // Separador
//   Blynk.virtualWrite(V0, t);            //
//   Blynk.virtualWrite(V1, h);            //
//   Firebase.pushInt(firebaseData, ruta + "/TEMPERATURA AIRE ", t);
//   Firebase.setInt(firebaseData, ruta2 + "/TEMPERATURA AIRE ", t);
//   Firebase.pushInt(firebaseData, ruta + "/HUMEDAD AIRE ", h);
//   Firebase.setInt(firebaseData, ruta2 + "/HUMEDAD AIRE ", h);
//   delay(5000);
// }

// void getSendDataBH1750() {
//   lux = lightMeter.readLightLevel();
//   Blynk.virtualWrite(V3, lux);
//   Firebase.pushInt(firebaseData, ruta + "/LUMINOSIDAD ", lux);
//   Firebase.setInt(firebaseData, ruta2 + "/LUMINOSIDAD ", lux);
//   Serial.print("LUMINOSIDAD: ");
//   Serial.print(lux);
//   Serial.println(" lx");
//   if (temp > 8000) {
//     Firebase.pushInt(firebaseData, ruta + "/LUMINOSIDAD ", lux);
//     Firebase.setInt(firebaseData, ruta2 + "/LUMINOSIDAD ", lux);

//     Serial.print("LUMINOSIDAD MUY ALTA: ");
//     Serial.print(lux);
//     Serial.println(" lx");
//   }
// }

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

// void PrenderApagarLucesLed() {
//   if (estadoLucesLed) {
//     digitalWrite(LucesLed, LOW);
//   } else {
//     digitalWrite(LucesLed, HIGH);
//   }
// }

// void actualizar_estadoLucesLed() {
//   if (Firebase.getBool(firebaseData, path2 + "/LUCES LED ")) {
//     Serial.println("PASSED");
//     Serial.println("PATH: " + firebaseData.dataPath());
//     Serial.println("TYPE: " + firebaseData.dataType());
//     Serial.println("ETag: " + firebaseData.ETag());
//     Serial.print("VALUE: ");
//     estadoLucesLed = firebaseData.boolData();
//     Serial.println(estadoLucesLed);
//     Serial.println("------------------------------------");
//     Serial.println();
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + firebaseData.errorReason());
//     Serial.println("------------------------------------");
//     Serial.println();
//   }
// }

// BLYNK_WRITE(V5) {
//   int value = 0;
//   value = param.asInt();
//   Serial.println(value);
//   if (value == 1) {
//     digitalWrite(LucesLed, LOW);
//     Firebase.pushBool(firebaseData, ruta + "/LUCES LED ", true);
//     Firebase.setBool(firebaseData, ruta2 + "/LUCES LED ", true);
//     Serial.println("LUCES LED ENCENDIDAS");
//   }
//   if (value == 0) {
//     digitalWrite(LucesLed, HIGH);
//     Firebase.pushBool(firebaseData, ruta + "/LUCES LED ", false);
//     Firebase.setBool(firebaseData, ruta2 + "/LUCES LED ", false);
//     Serial.println("LUCES LED APAGADAS");
//   }
// }

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

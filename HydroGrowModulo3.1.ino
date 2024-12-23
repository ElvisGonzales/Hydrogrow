#define BLYNK_TEMPLATE_ID "TMPL23FwyqjHu"
#define BLYNK_TEMPLATE_NAME "NewHydrogrow"
#define BLYNK_AUTH_TOKEN "O-tdpvVBq5NhfullueaG5-6nIdSARx6H"
#define BLYNK_PRINT Serial

//FIREBASE ELVIS
#define FIREBASE_HOST "hydrogrowtesis-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ac0YAOhFe2NEsA2FDD6CVcCBhhWQnWTkLAMuPsQR"

//LIBRERIAS
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <WiFiClient.h>

char auth[] = BLYNK_AUTH_TOKEN;  // Your AUTH_TOKEN
char ssid[] = "QP_COM";          // Your WiFi SSID
char pass[] = "1207454172Eg";    // Your WiFi password

#define ENA D7  // PWM pin for motor A
#define ENB D8  // PWM pin for motor B
#define IN1 D1  // Motor A control pin 1
#define IN2 D2  // Motor A control pin 2
#define IN3 D5  // Motor B control pin 1
#define IN4 D6  // Motor B control pin 2
#define buttonA D3
#define buttonB D4

const int defaultSpeed = 255;  // Velocidad predeterminada
volatile bool estadoNutrientesA = false;
volatile bool estadoNutrientesB = false;

SimpleTimer timer;
int ph_pin = A0;  //
int m_4 = 623;
int m_7 = 605;  //agua
float Po;
// Variables to store values
float phMin = 0.0;
float phMax = 0.0;
int timeRange = 0; // in minutes
int interval = 0; // in minutes

FirebaseData firebaseData;
String ruta = "HydroGrow", path = "/";
String ruta2 = "RealTime";
String path2 = "RealTime";
String rutaVariables= "Configuracion";
String pathVariables = "Configuracion";

// Firebase Variables
String pathPHMin = rutaVariables + "/Ph Min";
String pathPHMax = rutaVariables + "/Ph Max";
String pathTimeRange = rutaVariables + "/Lapso_Min";
String pathInterval = rutaVariables + "/Intervalo_Hora";

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // readFirebaseValues();
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(buttonA, INPUT_PULLUP);  // Activar resistencia pull-up interna
  pinMode(buttonB, INPUT_PULLUP);  // Activar resistencia pull-up interna
  // Apagar las bombas inicialmente
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  timer.setInterval(1000L, getSendDataPH);  // new data will be updated every 1 sec
  delay(1000L);
}

void loop() {
  Blynk.run();
  actualizar_estadoNutrientesA();
  actualizar_estadoNutrientesB();
  leerBotonA();
  leerBotonB();
  controlNutrientes();  // Llamar a la función para controlar las bombas de nutrientes
  readFirebaseValues();
  delay(500);  // Reducido para mejorar la respuesta del sistema
}

//Leer el boton Motor A
void leerBotonA() {
  if (digitalRead(buttonA) == LOW) {  // El botón está siendo presionado
    estadoNutrientesA = !estadoNutrientesA;  // Cambiar el esta
    Firebase.setBool(firebaseData, ruta + "/NUTRIENTES A", estadoNutrientesA);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES A", estadoNutrientesA);
    PrenderApagarNutrientesA();
    delay(500); // Debounce delay
  }
}

//Leer el boton Motor B
void leerBotonB() {
  if (digitalRead(buttonB) == LOW) {  // El botón está siendo presionado
    estadoNutrientesB = !estadoNutrientesB;  // Cambiar el estado
    Firebase.setBool(firebaseData, ruta + "/NUTRIENTES B", estadoNutrientesB);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES B", estadoNutrientesB);
    PrenderApagarNutrientesB();
    delay(500);  // Debounce delay
  }
}

// Control Blynk Motor 1
BLYNK_WRITE(V2) {
  int state1 = param.asInt();

  if (state1 == 1) {  // Encender motor 1
    estadoNutrientesA = true;
    analogWrite(ENA, defaultSpeed);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    Firebase.pushBool(firebaseData, ruta + "/NUTRIENTES A", true);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES A", true);
    Serial.println("BOMBA NUTRIENTES A ENCENDIDA");
  } else {  // Apagar motor 1
    estadoNutrientesA = false;
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    Firebase.pushBool(firebaseData, ruta + "/NUTRIENTES A", false);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES A", false);
    Serial.println("BOMBA NUTRIENTES A APAGADA");
  }
}

// Control Blynk Motor 2
BLYNK_WRITE(V3) {
  int state2 = param.asInt();

  if (state2 == 1) {  // Encender motor 2
    estadoNutrientesB = true;
    analogWrite(ENB, defaultSpeed);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Firebase.pushBool(firebaseData, ruta + "/NUTRIENTES B", true);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES B", true);
    Serial.println("BOMBA NUTRIENTES B ENCENDIDA");
  } else {  // Apagar motor 2
    estadoNutrientesB = false;
    analogWrite(ENB, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    Firebase.pushBool(firebaseData, ruta + "/NUTRIENTES B", false);
    Firebase.setBool(firebaseData, ruta2 + "/NUTRIENTES B", false);
    Serial.println("BOMBA NUTRIENTES B APAGADA");
  }
}

void PrenderApagarNutrientesA() {
  if (estadoNutrientesA) {
    analogWrite(ENA, defaultSpeed);  // Velocidad predeterminada
    digitalWrite(IN1, HIGH);         // Encender motor A
    digitalWrite(IN2, LOW);
  } else {
    analogWrite(ENA, 0);  // Apagar motor A
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

void PrenderApagarNutrientesB() {
  if (estadoNutrientesB) {
    analogWrite(ENB, defaultSpeed);  // Velocidad predeterminada
    digitalWrite(IN3, HIGH);         // Encender motor B
    digitalWrite(IN4, LOW);
  } else {
    analogWrite(ENB, 0);  // Apagar motor B
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void actualizar_estadoNutrientesA() {
  if (Firebase.getBool(firebaseData, path2 + "/NUTRIENTES A")) {
    if (firebaseData.dataType() == "boolean") {
      bool nuevoEstado = firebaseData.boolData();
      if (estadoNutrientesA != nuevoEstado) {
        estadoNutrientesA = nuevoEstado;
        PrenderApagarNutrientesA();
        Serial.println("Estado de Nutrientes A actualizado desde Firebase.");
      }
    }
  } else {
    Serial.println("Error al leer NUTRIENTES A desde Firebase: " + firebaseData.errorReason());
  }
}

void actualizar_estadoNutrientesB() {
  if (Firebase.getBool(firebaseData, path2 + "/NUTRIENTES B")) {
    if (firebaseData.dataType() == "boolean") {
      bool nuevoEstado = firebaseData.boolData();
      if (estadoNutrientesB != nuevoEstado) {
        estadoNutrientesB = nuevoEstado;
        PrenderApagarNutrientesB();
        Serial.println("Estado de Nutrientes B actualizado desde Firebase.");
      }
    }
  } else {
    Serial.println("Error al leer NUTRIENTES B desde Firebase: " + firebaseData.errorReason());
  }
}

void getSendDataPH() {
  int measure = 0;
  int prom = 0;
  for (int i = 0; i < 20; i++) {
    measure = analogRead(ph_pin);
    prom = prom + measure;
    delay(50);
  }
  prom = prom / 20;
  Serial.print("Medida: ");
  Serial.print(prom);
  //calibracion
  Po = 7 + ((measure - m_7) * 3 / (m_7 - m_4));
  Serial.print("\tPH: ");
  Serial.print(Po, 2);
  Serial.println("");
  Blynk.virtualWrite(V3, Po);  //
  Firebase.setInt(firebaseData, ruta2 + "/NIVEL DE PH", Po);
  Firebase.pushInt(firebaseData, ruta + "/NIVEL DE PH", Po);
  Serial.print("NIVEL DE PH: ");
  Serial.print(Po);
  Serial.println(" ");
  if (Po > 10) {
    //Firebase.setInt(firebaseData, ruta + "/NIVEL DE PH ", Po);
    Serial.print("PH DEL AGUA MUY ALTO: ");
    Serial.print(Po);
    Serial.println(" ");
  } else if (Po > 4) {
    //Firebase.setInt(firebaseData, ruta + "/NIVEL DE PH ", Po);
    Serial.print("PH DEL AGUA MUY BAJO: ");
    Serial.print(Po);
    Serial.println(" ");
  }
}

void controlNutrientes() {
  if (Po < phMin) {
    estadoNutrientesA = true;  // Encender nutrientes A
    estadoNutrientesB = false;  // Apagar nutrientes B
  } else if (Po > phMax) {
    estadoNutrientesA = false;  // Apagar nutrientes A
    estadoNutrientesB = true;   // Encender nutrientes B
  } else {
    estadoNutrientesA = false;  // Apagar nutrientes A
    estadoNutrientesB = false;  // Apagar nutrientes B
  }

  // Actualizar el estado de las bombas
  PrenderApagarNutrientesA();
  PrenderApagarNutrientesB();
}

void controlNutrientes2() {
  while (true) {  // Bucle infinito hasta que se cumpla una condición de salida
    if (Po < 5.5) {
      estadoNutrientesA = true;  // Encender nutrientes A
      estadoNutrientesB = false;  // Apagar nutrientes B
    } else if (Po > 5.8) {
      estadoNutrientesA = false;  // Apagar nutrientes A
      estadoNutrientesB = true;   // Encender nutrientes B
    } else {
      estadoNutrientesA = false;  // Apagar nutrientes A
      estadoNutrientesB = false;  // Apagar nutrientes B
      break;  // Salir del bucle si el pH está en el rango deseado
    }
    // Actualizar el estado de las bombas
    PrenderApagarNutrientesA();
    PrenderApagarNutrientesB();

    // Esperar un tiempo antes de volver a medir el pH
    delay(1000);  // Esperar 1 segundo antes de volver a comprobar el pH
    getSendDataPH();  // Medir el pH nuevamente
  }
}

// Function to read values from Firebase
void readFirebaseValues() {
    if (Firebase.getFloat(firebaseData, pathPHMin)) {
        phMin = firebaseData.floatData();
        Serial.print("pH Min: ");
        Serial.println(phMin);
    }
    if (Firebase.getFloat(firebaseData, pathPHMax)) {
        phMax = firebaseData.floatData();
        Serial.print("pH Max: ");
        Serial.println(phMax);
    }
    // if (Firebase.getInt(firebaseData, pathTimeRange)) {
    //     timeRange = firebaseData.intData();
    //     Serial.print("Time Range: ");
    //     Serial.println(timeRange);
    // }
    // if (Firebase.getInt(firebaseData, pathInterval)) {
    //     interval = firebaseData.intData();
    //     Serial.print("Interval: ");
    //     Serial.println(interval);
    // }
}
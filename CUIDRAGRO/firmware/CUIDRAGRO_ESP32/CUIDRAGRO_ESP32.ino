/*
  ============================================================================
  Proyecto:     CUIDRAGRO - Monitoreo IoT de variables ambientales
  Ubicación:    Finca Villa Dolly, Manizales, Caldas
  Componente:   Firmware ESP32 (nodo sensor)
  Autor:        Wilber Domínguez Mosquera
  Universidad:  UNAD - Ingeniería de Sistemas - Proyecto de grado 202016907
  Descripción:  Lee temperatura, humedad relativa, humedad del suelo y pH
                cada N minutos y envía los datos en formato JSON al backend
                mediante una petición HTTP POST.
  ============================================================================
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------
// 1. CONFIGURACIÓN DE RED Y SERVIDOR
// ---------------------------------------------------------------------------
const char* WIFI_SSID     = "NOMBRE_DE_TU_RED";
const char* WIFI_PASSWORD = "CONTRASENA_DE_TU_RED";
const char* SERVER_URL    = "http://IP_O_DOMINIO_DEL_SERVIDOR:3000/api/lecturas";
const char* DEVICE_TOKEN  = "cuidragro-esp32-001"; // identificador del nodo

// ---------------------------------------------------------------------------
// 2. CONFIGURACIÓN DE PINES Y SENSORES
// ---------------------------------------------------------------------------
#define DHTPIN        4          // Pin de datos del DHT22
#define DHTTYPE       DHT22
#define PIN_HUMEDAD_SUELO  34    // Pin analógico (ADC1_CH6)
#define PIN_PH             35    // Pin analógico (ADC1_CH7)

DHT dht(DHTPIN, DHTTYPE);

// Calibración del sensor de humedad de suelo (valores ADC de referencia)
const int SUELO_SECO_ADC   = 3000; // lectura ADC con sensor al aire (0% humedad)
const int SUELO_HUMEDO_ADC = 1200; // lectura ADC en agua (100% humedad)

// Calibración del sensor de pH (ajustar según hoja de datos del módulo usado)
const float PH_PENDIENTE   = -0.0169; // pendiente de la recta de calibración
const float PH_OFFSET      = 21.34;   // intercepto de la recta de calibración

// Intervalo entre lecturas (milisegundos)
const unsigned long INTERVALO_LECTURA = 5UL * 60UL * 1000UL; // 5 minutos

// ---------------------------------------------------------------------------
// 3. FUNCIONES AUXILIARES
// ---------------------------------------------------------------------------
void conectarWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 20000) {
    delay(400);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nNo fue posible conectar a la red WiFi.");
  }
}

float leerHumedadSuelo() {
  int lectura = analogRead(PIN_HUMEDAD_SUELO);
  float porcentaje = map(lectura, SUELO_SECO_ADC, SUELO_HUMEDO_ADC, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100);
  return porcentaje;
}

float leerPH() {
  int lectura = analogRead(PIN_PH);
  float voltaje = lectura * (3.3 / 4095.0);
  float ph = PH_PENDIENTE * lectura + PH_OFFSET; // curva de calibración lineal
  return ph;
}

bool enviarDatos(float temperatura, float humedadAire, float humedadSuelo, float ph) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sin conexión WiFi. Se omite el envío.");
    return false;
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["dispositivo"]      = DEVICE_TOKEN;
  doc["temperatura"]      = temperatura;
  doc["humedad_aire"]     = humedadAire;
  doc["humedad_suelo"]    = humedadSuelo;
  doc["ph"]               = ph;

  String payload;
  serializeJson(doc, payload);

  int codigoRespuesta = http.POST(payload);
  Serial.printf("POST -> código HTTP: %d\n", codigoRespuesta);
  http.end();

  return codigoRespuesta == 200 || codigoRespuesta == 201;
}

// ---------------------------------------------------------------------------
// 4. CICLO PRINCIPAL
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  dht.begin();
  analogReadResolution(12); // ESP32: resolución ADC de 12 bits (0-4095)
  conectarWiFi();
}

void loop() {
  float temperatura   = dht.readTemperature();
  float humedadAire   = dht.readHumidity();
  float humedadSuelo  = leerHumedadSuelo();
  float ph            = leerPH();

  if (isnan(temperatura) || isnan(humedadAire)) {
    Serial.println("Error de lectura en el sensor DHT22.");
  } else {
    Serial.printf("Temp: %.1f C | Humedad aire: %.1f%% | Humedad suelo: %.1f%% | pH: %.2f\n",
                  temperatura, humedadAire, humedadSuelo, ph);
    enviarDatos(temperatura, humedadAire, humedadSuelo, ph);
  }

  delay(INTERVALO_LECTURA);
}

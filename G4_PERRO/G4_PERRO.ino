//GRUPO 4 OTERO, FRACCARO, PONCZYK, NISIM


#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "AsyncMqttClient.h"
#include "time.h"
#include "Arduino.h"

const char* ssid = "MECA-IoT";
const char* password = "IoT$2026";

const uint8_t name_device_id = 14;  //NUMERO DE GRUPO

unsigned long now = 0;
unsigned long lastMeasure1 = 0;
unsigned long lastMeasure2 = 0;

unsigned long interval_envio = 30000UL;
const unsigned long interval_lectura = 60000UL;

const char* ntpServer = "south-america.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;
long unsigned int timestamp;


int indice_entra = 0;
int indice_saca = 0;
bool flag_vacio = true;

#define MQTT_HOST IPAddress(192, 168, 5, 123)
#define MQTT_PORT 1884
#define MQTT_USERNAME "esp32"
#define MQTT_PASSWORD "mirko15"
char mqtt_payload[200];
#define MQTT_PUB "/esp32/datos_sensores"
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

typedef struct  //Estructura
{
  long time;
  float T1;
  float H1;
  float luz;
  float G1;
  float G2;
} estructura;

const int valor_max_struct = 1000;
estructura datos_struct[valor_max_struct];
estructura aux2;

void connectToWifi();
void connectToMqtt();
void WiFiEvent(WiFiEvent_t event);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttPublish(uint16_t packetId);
void setupmqtt();
void fun_envio_mqtt();
void fun_saca();
void fun_entra();
void sincronizarHora();
void HoraGMT();

SemaphoreHandle_t xDatosMutex = NULL;

ESP32Time rtc;
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;

#define LDR 34
#define GAS 32
#define MQ9 33
#define LED1 13
#define LED2 12
#define LED3 14
#define BOTON1 27
#define BOTON2 26
#define BOTON3 25
#define BOTON4 35
#define BOTON5 36

#define P1 1
#define ESPERA1 2
#define ESPERA2 3
#define ESPERA3 4
#define ESPERA4 5
#define ESPERA5 6
#define TEMP 10
#define RESTATEMP 11
#define SUMATEMP 12
#define GMT_MQTT 13
#define SUMAGMT 14
#define RESTAGMT 15
#define SUMATIEMPO 16
#define RESTATIEMPO 17
#define EGAS 18
#define SUMAGAS 19
#define RESTAGAS 20
#define LUZ 21
#define SUMALUZ 22
#define RESTALUZ 23
#define SUMAHUM 24
#define RESTAHUM 25
#define SUMAMQ9 26
#define RESTAMQ9 27

int estado = P1;

int millis_actual;
int millis_aht;

int valorLuz;
int LuzMap;

int gasValor;
int gasMap;
int MQ9Valor;
int MQ9Map;

int intervalo;

int uLuz;
int uMQ9;
int uGas;
int uTemp;
int uHum;
int uIntervalo;  // para mostrar en p (s)
int uGMT;

bool mensajeEnviadoTemp = false;
bool mensajeEnviadoHum = false;
bool mensajeEnviadoLuz = false;
bool mensajeEnviadoGas = false;
bool mensajeEnviadoMQ9 = false;

Preferences preferences;

// --------------------------- TAREAS -------------------------------------
TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code(void* pvParameters);
void Task2code(void* pvParameters);

// --------------------------- SETUP MQTT ---------------------------------
void setupmqtt() {
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
  connectToWifi();
}

void fun_envio_mqtt() {
  // Extraigo un elemento de la cola (si hay)
  fun_saca();
  if (!flag_vacio) {
    Serial.print("enviando... ");

    // Protejo aux2 mientras genero el payload (aunque fun_saca ya lo copió)
    xSemaphoreTake(xDatosMutex, portMAX_DELAY);
    memset(mqtt_payload, 0, sizeof(mqtt_payload));
    // Enviamos: id, time, T1, H1, luz, G1, G2
    snprintf(mqtt_payload, sizeof(mqtt_payload),
             "%u&%ld&%.2f&%.2f&%.2f&%.2f&%.2f",
             (unsigned)name_device_id, aux2.time, aux2.T1, aux2.H1, aux2.luz, aux2.G1, aux2.G2);
    // Limpio aux2
    aux2.time = 0;
    aux2.T1 = 0;
    aux2.H1 = 0;
    aux2.luz = 0;
    aux2.G1 = 0;
    aux2.G2 = 0;
    xSemaphoreGive(xDatosMutex);

    Serial.print("Publish message: ");
    Serial.println(mqtt_payload);
    if (mqttClient.connected()) {
      uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB, 1, true, mqtt_payload);
      (void)packetIdPub1;
    } else {
      Serial.println("MQTT no conectado, no se publica");
    }
  } else {
    Serial.println("no hay valores nuevos");
  }
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
    default:
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged. packetId: ");
  Serial.println(packetId);
}

void fun_saca() {
  xSemaphoreTake(xDatosMutex, portMAX_DELAY);
  if (indice_saca != indice_entra) {
    // Copio entrada a aux2
    aux2.time = datos_struct[indice_saca].time;
    aux2.T1 = datos_struct[indice_saca].T1;
    aux2.H1 = datos_struct[indice_saca].H1;
    aux2.luz = datos_struct[indice_saca].luz;
    aux2.G1 = datos_struct[indice_saca].G1;
    aux2.G2 = datos_struct[indice_saca].G2;

    flag_vacio = false;

    Serial.print("indice_saca previo: ");
    Serial.println(indice_saca);

    // incremento circular
    indice_saca++;
    if (indice_saca >= valor_max_struct) {
      indice_saca = 0;
    }

    Serial.print("saco valores de la struct, indice ahora: ");
    Serial.println(indice_saca);
  } else {
    flag_vacio = true;  // no hay datos
  }
  xSemaphoreGive(xDatosMutex);
}

void fun_entra(void) {
  xSemaphoreTake(xDatosMutex, portMAX_DELAY);
  if (indice_entra >= valor_max_struct) {
    indice_entra = 0;
  }

  // timestamp
  timestamp = time(NULL);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    xSemaphoreGive(xDatosMutex);
    return;
  }
  char buftime[64];
  strftime(buftime, sizeof(buftime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.print("> NTP Time: ");
  Serial.println(buftime);

  // Copio mediciones actuales (se asume que Task2 actualiza estas variables)
  datos_struct[indice_entra].time = timestamp;
  datos_struct[indice_entra].T1 = temp.temperature;
  datos_struct[indice_entra].H1 = humidity.relative_humidity;
  datos_struct[indice_entra].luz = LuzMap;
  datos_struct[indice_entra].G1 = gasMap;
  datos_struct[indice_entra].G2 = MQ9Map;

  indice_entra++;
  if (indice_entra >= valor_max_struct) {
    indice_entra = 0;
  }

  Serial.print("ingreso valores a la struct, indice_entra: ");
  Serial.println(indice_entra);

  xSemaphoreGive(xDatosMutex);
}

// --------------------------- HORA / RTC ----------------------------------
void sincronizarHora() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  } else {
    Serial.println("Error al sincronizar hora");
  }
}

void HoraGMT() {
  int offset = uGMT * 3600;
  configTime(offset, 0, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
    Serial.println("Hora sincronizada con GMT actualizado");
  } else {
    Serial.println("Error al sincronizar hora con nuevo GMT");
  }
}

// --------------------------- pS ----------------------------------
void pMenu(void) {
  lcd.setCursor(0, 0);
  lcd.print("LUZ:1 GAS:2 TH:3   ");
  lcd.setCursor(0, 1);
  lcd.print("MQTT-GMT:4 MENU5  ");
}

void pLuz(void) {
  lcd.setCursor(0, 0);
  lcd.print("VALOR DE LUZ      ");
  lcd.setCursor(0, 1);
  lcd.print("Act:");
  lcd.print(LuzMap);
  lcd.print("% ");
  lcd.print("Um:");
  lcd.print(uLuz);
  lcd.print("%  ");
}

void pTemp(void) {
  lcd.setCursor(0, 0);
  lcd.print("Tact");
  lcd.print(temp.temperature);
  lcd.print("C ");
  lcd.print("Tum:");
  lcd.print(uTemp);
  lcd.print("C ");
  lcd.setCursor(0, 1);
  lcd.print("Hact:");
  lcd.print(humidity.relative_humidity);
  lcd.print("% ");
  lcd.print("Hum:");
  lcd.print(uHum);
  lcd.print("% ");
}

void pGas(void) {
  lcd.setCursor(0, 0);
  lcd.print("GA");
  lcd.print(gasMap);
  lcd.print("% ");
  lcd.print("GU:");
  lcd.print(uGas);
  lcd.print("%  ");
  lcd.setCursor(0, 1);
  lcd.print("MA:");
  lcd.print(MQ9Map);
  lcd.print("% ");
  lcd.print("MU:");
  lcd.print(uMQ9);
  lcd.print("%  ");
}

void pGMT_MQTT(void) {
  lcd.setCursor(0, 0);
  lcd.print("tiempo:");
  lcd.print(uIntervalo / 1000);
  lcd.print("s    ");
  lcd.setCursor(0, 1);
  lcd.print("GMT:");
  lcd.print(uGMT);
  lcd.print("     ");
}

// --------------------------- SETUP --------------------------------------
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  // creo mutex
  xDatosMutex = xSemaphoreCreateMutex();
  if (xDatosMutex == NULL) {
    Serial.println("ERROR: no se pudo crear mutex");
    while (1) delay(1000);
  }

  setupmqtt();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (!aht.begin(&Wire)) {
    Serial.println("No se encontró AHT10/AHT20, revisa conexiones!");
    while (1) delay(1000);
  }
  Serial.println("Sensor AHT10 detectado correctamente.");

  pinMode(LDR, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  pinMode(BOTON3, INPUT_PULLUP);
  pinMode(BOTON4, INPUT_PULLUP);
  pinMode(BOTON5, INPUT_PULLUP);

  preferences.begin("config", false);
  uLuz = preferences.getInt("uLuz", 70);
  uMQ9 = preferences.getInt("uMQ9", 10);
  uGas = preferences.getInt("uGas", 10);
  uTemp = preferences.getInt("uTemp", 23);
  uHum = preferences.getInt("uHum", 40);
  interval_envio = (unsigned long)preferences.getInt("intervalo", (int)interval_envio);
  uGMT = preferences.getInt("uGMT", -3);
  preferences.end();

  uIntervalo = interval_envio;

  int offset = uGMT * 3600;
  configTime(offset, 0, ntpServer);
  sincronizarHora();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Hola ESP32!");
  lcd.setCursor(0, 1);
  lcd.print("LCD 16x2 prueba");

  // Creo tareas
  xTaskCreatePinnedToCore(
    Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  delay(500);
  xTaskCreatePinnedToCore(
    Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
  delay(500);
}

// --------------------------- LOOP 1 -----------------
void Task1code(void* pvParameters) {
  for (;;) {
    now = millis();
    if (now - lastMeasure1 > interval_envio) {
      lastMeasure1 = now;
      fun_envio_mqtt();
    }
    if (now - lastMeasure2 > interval_lectura) {
      lastMeasure2 = now;
      fun_entra();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ---------------------------LOOP2 -------------------------
void Task2code(void* pvParameters) {
  intervalo = 30000;
  for (;;) {
    millis_actual = millis();
    if (millis_actual - millis_aht >= intervalo) {
      // leo sensor AHT y analógicos
      aht.getEvent(&humidity, &temp);
      Serial.print("Temperatura: ");
      Serial.print(temp.temperature);
      Serial.println(" °C");
      Serial.print("Humedad: ");
      Serial.print(humidity.relative_humidity);
      Serial.println(" %");

      valorLuz = analogRead(LDR);
      LuzMap = map(valorLuz, 0, 4095, 0, 100);
      Serial.print("luz:");
      Serial.print(LuzMap);
      Serial.println("%");

      gasValor = analogRead(GAS);
      gasMap = map(gasValor, 0, 4095, 0, 100);
      Serial.print("Gas: ");
      Serial.print(gasMap);
      Serial.println("%");

      MQ9Valor = analogRead(MQ9);
      MQ9Map = map(MQ9Valor, 0, 4095, 0, 100);
      Serial.print("MQ9: ");
      Serial.print(MQ9Map);
      Serial.println("%");

      millis_aht = millis_actual;
    }

    bool peligro = false;
    bool alerta = false;

    if (temp.temperature >= uTemp) alerta = true;
    if (temp.temperature >= uTemp * 1.3) peligro = true;
    if (humidity.relative_humidity >= uHum) alerta = true;
    if (humidity.relative_humidity >= uHum * 1.3) peligro = true;
    if (gasMap >= uGas) alerta = true;
    if (gasMap >= uGas * 1.3) peligro = true;
    if (MQ9Map >= uMQ9) alerta = true;
    if (MQ9Map >= uMQ9 * 1.3) peligro = true;
    if (LuzMap >= uLuz) alerta = true;
    if (LuzMap >= uLuz * 1.3) peligro = true;

    if (peligro) {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, HIGH);
    } else if (alerta) {
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, LOW);
    } else {
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
    }
    switch (estado) {
      case P1:
        pMenu();
        if (digitalRead(BOTON1) == LOW) estado = ESPERA2;
        if (digitalRead(BOTON2) == LOW) estado = ESPERA3;
        if (digitalRead(BOTON3) == LOW) estado = ESPERA4;
        if (digitalRead(BOTON4) == LOW) estado = ESPERA5;
        break;

      case ESPERA1:
        lcd.clear();
        if (digitalRead(BOTON5) == HIGH) estado = P1;
        break;

      case ESPERA2:
        lcd.clear();
        if (digitalRead(BOTON1) == HIGH) estado = LUZ;
        break;

      case ESPERA3:
        lcd.clear();
        if (digitalRead(BOTON2) == HIGH) estado = EGAS;
        break;

      case ESPERA4:
        lcd.clear();
        if (digitalRead(BOTON3) == HIGH) estado = TEMP;
        break;

      case ESPERA5:
        lcd.clear();
        if (digitalRead(BOTON4) == HIGH) estado = GMT_MQTT;
        break;

      case LUZ:
        pLUZ();
        if (digitalRead(BOTON1) == LOW) estado = SUMALUZ;
        if (digitalRead(BOTON2) == LOW) estado = RESTALUZ;
        if (digitalRead(BOTON5) == LOW) estado = ESPERA1;
        break;

      case EGAS:
        pGas();
        if (digitalRead(BOTON1) == LOW) estado = SUMAGAS;
        if (digitalRead(BOTON2) == LOW) estado = RESTAGAS;
        if (digitalRead(BOTON3) == LOW) estado = SUMAMQ9;
        if (digitalRead(BOTON4) == LOW) estado = RESTAMQ9;
        if (digitalRead(BOTON5) == LOW) estado = ESPERA1;
        break;

      case TEMP:
        pTemp();
        if (digitalRead(BOTON1) == LOW) estado = SUMATEMP;
        if (digitalRead(BOTON2) == LOW) estado = RESTATEMP;
        if (digitalRead(BOTON3) == LOW) estado = SUMAHUM;
        if (digitalRead(BOTON4) == LOW) estado = RESTAHUM;
        if (digitalRead(BOTON5) == LOW) estado = ESPERA1;
        break;

      case GMT_MQTT:
        pGMT_MQTT();
        if (digitalRead(BOTON1) == LOW) estado = SUMAGMT;
        if (digitalRead(BOTON2) == LOW) estado = RESTAGMT;
        if (digitalRead(BOTON3) == LOW) estado = SUMATIEMPO;
        if (digitalRead(BOTON4) == LOW) estado = RESTATIEMPO;
        if (digitalRead(BOTON5) == LOW) estado = ESPERA1;
        break;



      case SUMALUZ:
        pLuz();
        if (digitalRead(BOTON1) == HIGH) {
          uLuz++;
          if (uLuz > 100) uLuz = 100;
          preferences.begin("config", false);
          preferences.putInt("uLuz", uLuz);
          preferences.end();
          estado = LUZ;
        }
        break;

      case RESTALUZ:
        pLuz();
        if (digitalRead(BOTON2) == HIGH) {
          uLuz--;
          if (uLuz < 0) uLuz = 0;
          preferences.begin("config", false);
          preferences.putInt("uLuz", uLuz);
          preferences.end();
          estado = LUZ;
        }
        break;

      case SUMAGAS:
        pGas();
        if (digitalRead(BOTON1) == HIGH) {
          uGas++;
          if (uGas > 100) uGas = 100;
          preferences.begin("config", false);
          preferences.putInt("uGas", uGas);
          preferences.end();
          estado = EGAS;
        }
        break;

      case RESTAGAS:
        pGas();
        if (digitalRead(BOTON2) == HIGH) {
          uGas--;
          if (uGas < 0) uGas = 0;
          preferences.begin("config", false);
          preferences.putInt("uGas", uGas);
          preferences.end();
          estado = EGAS;
        }
        break;

      case SUMAMQ9:
        pGas();
        if (digitalRead(BOTON3) == HIGH) {
          uMQ9++;
          if (uMQ9 > 100) uMQ9 = 100;
          preferences.begin("config", false);
          preferences.putInt("uMQ9", uMQ9);
          preferences.end();
          estado = EGAS;
        }
        break;

      case RESTAMQ9:
        pGas();
        if (digitalRead(BOTON4) == HIGH) {
          uMQ9--;
          if (uMQ9 < 0) uMQ9 = 0;
          preferences.begin("config", false);
          preferences.putInt("uMQ9", uMQ9);
          preferences.end();
          estado = EGAS;
        }
        break;

      case SUMATEMP:
        pTemp();
        if (digitalRead(BOTON1) == HIGH) {
          uTemp++;
          preferences.begin("config", false);
          preferences.putInt("uTemp", uTemp);
          preferences.end();
          estado = TEMP;
        }
        break;

      case RESTATEMP:
        pTemp();
        if (digitalRead(BOTON2) == HIGH) {
          uTemp--;
          preferences.begin("config", false);
          preferences.putInt("uTemp", uTemp);
          preferences.end();
          estado = TEMP;
        }
        break;

      case SUMAHUM:
        pTemp();
        if (digitalRead(BOTON3) == HIGH) {
          uHum++;
          if (uHum > 100) uHum = 100;
          preferences.begin("config", false);
          preferences.putInt("uHum", uHum);
          preferences.end();
          estado = TEMP;
        }
        break;

      case RESTAHUM:
        pTemp();
        if (digitalRead(BOTON4) == HIGH) {
          uHum--;
          if (uHum < 0) uHum = 0;
          preferences.begin("config", false);
          preferences.putInt("uHum", uHum);
          preferences.end();
          estado = TEMP;
        }
        break;

      case SUMAGMT:
        pGMT_MQTT();
        if (digitalRead(BOTON1) == HIGH) {
          uGMT++;
          if (uGMT > 12) uGMT = 12;
          preferences.begin("config", false);
          preferences.putInt("uGMT", uGMT);
          preferences.end();
          HoraGMT();
          estado = GMT_MQTT;
        }
        break;

      case RESTAGMT:
        pGMT_MQTT();
        if (digitalRead(BOTON2) == HIGH) {
          uGMT--;
          if (uGMT < -12) uGMT = -12;
          preferences.begin("config", false);
          preferences.putInt("uGMT", uGMT);
          preferences.end();
          HoraGMT();
          estado = GMT_MQTT;
        }
        break;

      case SUMATIEMPO:
        pGMT_MQTT();
        if (digitalRead(BOTON3) == HIGH) {
          interval_envio += 10000;
          if (interval_envio < 10000) interval_envio = 10000;
          preferences.begin("config", false);
          preferences.putInt("intervalo", (int)interval_envio);
          preferences.end();
          uIntervalo = interval_envio;
          estado = GMT_MQTT;
        }
        break;

      case RESTATIEMPO:
        pGMT_MQTT();
        if (digitalRead(BOTON4) == HIGH) {
          if (interval_envio > 10000) interval_envio -= 10000;
          if (interval_envio < 10000) interval_envio = 10000;
          preferences.begin("config", false);
          preferences.putInt("intervalo", (int)interval_envio);
          preferences.end();
          uIntervalo = interval_envio;
          estado = GMT_MQTT;
        }
        break;

      default:
        estado = P1;
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void loop() {
  delay(1000);
}
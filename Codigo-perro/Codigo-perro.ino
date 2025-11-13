// Grupo 4 Otero - Fraccaro - Ponczyk - Nisim
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ---------------- TELEGRAM ----------------
#define BOTtoken "8596626013:AAGZKLDOlDMJaJa_Z3Db5A5g1mkmqeQCxJk"
#define CHAT_ID "5941222238"

const char* ssid = "MECA-IoT";
const char* password = "IoT$2026";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
unsigned long lastTelegramCheck = 0;
bool alertaTemp = false, alertaHum = false, alertaLuz = false, alertaGas = false;

// ---------------- LCD / Sensores ----------------
Preferences preferences;
#define LCD_ADDR 0x27
#define SDA_PIN 21
#define SCL_PIN 22
//#define MQ4_PIN 32
//#define MQ9_PIN 33
//#define LDR_PIN 34
#define BOTON1 27
#define BOTON2 26
#define BOTON3 25
#define BOTON4 32
#define BOTON5 33
#define LED1 13
#define LED2 12
#define LED3 14

Adafruit_AHTX0 aht;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void menu(void);
void ptemp(void);
void pluz(void);
void pgas(void);
void pgmt(void);
void penvio(void);
void leerSensores(void);
void handleNewMessages(int numNewMessages);

int estado = 0;
int ultimoEstado = -1;
int lecturaMQ4, valorMQ4;
int lecturaMQ9, valorMQ9;
int uTemp, uHum, uLuz, uMq4, uMq9, uGmt, uEnvio;
unsigned long millis_valor;
float t = 0, h = 0;
int valorLDR = 0;


#define RST 0
#define MENU 2
#define TEMP 3
#define LUZ 4
#define GAS 5
#define GMT 6
#define ENVIO 7
#define ESPERA1 10
#define ESPERA2 14
#define ESPERA3 15
#define ESPERA4 28
#define ESPERA5 23
#define ESPERA6 24
#define SUMATEMP 89
#define SUMAHUM 88
#define SUMALUZ 87
#define SUMAMQ4 86
#define SUMAMQ9 85
#define SUMAGMT 84
#define SUMAENVIO 83
#define RESTATEMP 79
#define RESTAHUM 78
#define RESTALUZ 77
#define RESTAMQ4 76
#define RESTAMQ9 75
#define RESTAGMT 74
#define RESTAENVIO 73

TaskHandle_t Tarea1;
TaskHandle_t Tarea2;

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BOTON1, INPUT_PULLUP);
  pinMode(BOTON2, INPUT_PULLUP);
  pinMode(BOTON3, INPUT_PULLUP);
  pinMode(BOTON4, INPUT_PULLUP);
  pinMode(BOTON5, INPUT_PULLUP);

  preferences.begin("umbrales", false);
  uTemp = preferences.getInt("uTemp", 23);
  uHum = preferences.getInt("uHum", 50);
  uLuz = preferences.getInt("uLuz", 50);
  uMq4 = preferences.getInt("uMq4", 50);
  uMq9 = preferences.getInt("uMq9", 50);
  uGmt = preferences.getInt("uGmt", 0);
  uEnvio = preferences.getInt("uEnvio", 30);

  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");

  // Conexión WiFi
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Conectado!");
  Serial.println("\nWiFi conectado");

  if (!aht.begin(&Wire)) {
    Serial.println("❌ Error: No se detectó el AHT10.");
    //while (1)
    ;
  }
  xTaskCreatePinnedToCore(
    CodigoTarea1,  // loop sensores/LCD
    "Tarea1",
    15000,
    NULL,
    1,
    &Tarea1,
    0  // core 0
  );

  xTaskCreatePinnedToCore(
    CodigoTarea2,  // loop telegram
    "Tarea2",
    15000,
    NULL,
    1,
    &Tarea2,
    1  // core 1
  );
}

void CodigoTarea1(void* pvParameters) {
  for (;;) {
    unsigned long now = millis();
    //if (estado != ultimoEstado) {
    //  lcd.clear();
    //  ultimoEstado = estado;
    //}

    // Leer sensores cada uEnvio segundos
    /*
    if (millis() - millis_valor > uEnvio * 1000UL) {
    //  leerSensores();
      millis_valor = millis();
    }
  */
    switch (estado) {
      case RST:
        millis_valor = millis();
        estado = MENU;
        break;

      case MENU:
        if (ultimoEstado != MENU) {  // solo dibujar cuando se entra al menú
          lcd.clear();
          menu();
          Serial.println("MENU");
        }
        ultimoEstado = MENU;

        if (digitalRead(BOTON1) == LOW) estado = ESPERA1;
        if (digitalRead(BOTON2) == LOW) estado = ESPERA2;
        if (digitalRead(BOTON3) == LOW) estado = ESPERA3;
        if (digitalRead(BOTON4) == LOW) estado = ESPERA4;
        if (digitalRead(BOTON5) == LOW) estado = ESPERA5;
        break;


      case ESPERA1:
        if (digitalRead(BOTON1) == HIGH) estado = TEMP;
        break;

      case ESPERA2:
        if (digitalRead(BOTON2) == HIGH) estado = LUZ;
        break;

      case ESPERA3:
        if (digitalRead(BOTON3) == HIGH) estado = GAS;
        break;

      case ESPERA4:
        if (digitalRead(BOTON4) == HIGH) estado = GMT;
        break;

      case ESPERA5:
        if (digitalRead(BOTON5) == HIGH) estado = ENVIO;
        break;

      case ESPERA6:
        if (digitalRead(BOTON5) == HIGH) estado = MENU;
        break;

      case TEMP:
        ptemp();
        Serial.println("TEMPERATURA - HUMEDAD");
        if (digitalRead(BOTON1) == LOW) estado = SUMATEMP;
        if (digitalRead(BOTON2) == LOW) estado = RESTATEMP;
        if (digitalRead(BOTON3) == LOW) estado = SUMAHUM;
        if (digitalRead(BOTON4) == LOW) estado = RESTAHUM;
        break;

      case LUZ:
        Serial.println("LUZ");
        pluz();
        if (digitalRead(BOTON1) == LOW) estado = SUMALUZ;
        if (digitalRead(BOTON2) == LOW) estado = RESTALUZ;
        break;

      case GAS:
        Serial.println("GAS");
        pgas();
        if (digitalRead(BOTON1) == LOW) estado = SUMAMQ4;
        if (digitalRead(BOTON2) == LOW) estado = RESTAMQ4;
        if (digitalRead(BOTON3) == LOW) estado = SUMAMQ9;
        if (digitalRead(BOTON4) == LOW) estado = RESTAMQ9;
        break;

      case GMT:
        Serial.println("GMT");
        pgmt();
        if (digitalRead(BOTON1) == LOW) estado = SUMAGMT;
        if (digitalRead(BOTON2) == LOW) estado = RESTAGMT;
        break;

      case ENVIO:
        Serial.println("ENVIO");
        penvio();
        if (digitalRead(BOTON1) == LOW) estado = SUMAENVIO;
        if (digitalRead(BOTON2) == LOW) estado = RESTAENVIO;
        break;

      // Sumas y restas (igual que antes)
      case SUMATEMP:
        if (digitalRead(BOTON1) == HIGH) {
          uTemp++;
          preferences.putInt("uTemp", uTemp);
          estado = TEMP;
        }
        break;
      case RESTATEMP:
        if (digitalRead(BOTON2) == HIGH) {
          uTemp--;
          if (uTemp < 0) uTemp = 0;
          preferences.putInt("uTemp", uTemp);
          estado = TEMP;
        }
        break;
      case SUMAHUM:
        if (digitalRead(BOTON3) == HIGH) {
          uHum++;
          if (uHum > 100) uHum = 0;
          preferences.putInt("uHum", uHum);
          estado = TEMP;
        }
        break;
      case RESTAHUM:
        if (digitalRead(BOTON4) == HIGH) {
          uHum--;
          if (uHum < 0) uHum = 100;
          preferences.putInt("uHum", uHum);
          estado = TEMP;
        }
        break;
      case SUMALUZ:
        if (digitalRead(BOTON1) == HIGH) {
          uLuz++;
          if (uLuz > 100) uLuz = 0;
          preferences.putInt("uLuz", uLuz);
          estado = LUZ;
        }
        break;
      case RESTALUZ:
        if (digitalRead(BOTON2) == HIGH) {
          uLuz--;
          if (uLuz < 0) uLuz = 100;
          preferences.putInt("uLuz", uLuz);
          estado = LUZ;
        }
        break;
      case SUMAMQ4:
        if (digitalRead(BOTON1) == HIGH) {
          uMq4++;
          if (uMq4 > 100) uMq4 = 0;
          preferences.putInt("uMq4", uMq4);
          estado = GAS;
        }
        break;
      case RESTAMQ4:
        if (digitalRead(BOTON2) == HIGH) {
          uMq4--;
          if (uMq4 < 0) uMq4 = 100;
          preferences.putInt("uMq4", uMq4);
          estado = GAS;
        }
        break;
      case SUMAMQ9:
        if (digitalRead(BOTON3) == HIGH) {
          uMq9++;
          if (uMq9 > 100) uMq9 = 0;
          preferences.putInt("uMq9", uMq9);
          estado = GAS;
        }
        break;
      case RESTAMQ9:
        if (digitalRead(BOTON4) == HIGH) {
          uMq9--;
          if (uMq9 < 0) uMq9 = 100;
          preferences.putInt("uMq9", uMq9);
          estado = GAS;
        }
        break;
      case SUMAGMT:
        if (digitalRead(BOTON1) == HIGH) {
          uGmt++;
          if (uGmt > 12) uGmt = -12;
          preferences.putInt("uGmt", uGmt);
          estado = GMT;
        }
        break;
      case RESTAGMT:
        if (digitalRead(BOTON2) == HIGH) {
          uGmt--;
          if (uGmt < -12) uGmt = 12;
          preferences.putInt("uGmt", uGmt);
          estado = GMT;
        }
        break;
      case SUMAENVIO:
        if (digitalRead(BOTON1) == HIGH) {
          uEnvio += 30;
          preferences.putInt("uEnvio", uEnvio);
          estado = ENVIO;
        }
        break;
      case RESTAENVIO:
        if (digitalRead(BOTON2) == HIGH) {
          uEnvio -= 30;
          if (uEnvio <= 30) uEnvio = 30;
          preferences.putInt("uEnvio", uEnvio);
          estado = ENVIO;
        }
        break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}



void CodigoTarea2(void* pvParameters) {

  unsigned long lastCheckTelegram = 0;

  for (;;) {
    unsigned long now = millis();
    // --- ALERTAS TELEGRAM ---
    if (t > uTemp && !alertaTemp) {
      bot.sendMessage(CHAT_ID, "⚠️ Temperatura alta: " + String(t) + " °C (umbral " + String(uTemp) + ")", "");
      alertaTemp = true;
    } else if (t <= uTemp) alertaTemp = false;

    if (h > uHum && !alertaHum) {
      bot.sendMessage(CHAT_ID, "⚠️ Humedad alta: " + String(h) + "% (umbral " + String(uHum) + ")", "");
      alertaHum = true;
    } else if (h <= uHum) alertaHum = false;

    if (valorLDR < uLuz && !alertaLuz) {
      bot.sendMessage(CHAT_ID, "⚠️ Poca luz detectada (" + String(valorLDR) + "%, umbral " + String(uLuz) + ")", "");
      alertaLuz = true;
    } else if (valorLDR >= uLuz) alertaLuz = false;

    if ((valorMQ4 > uMq4 || valorMQ9 > uMq9) && !alertaGas) {
      bot.sendMessage(CHAT_ID, "⚠️ Nivel de gas elevado: MQ4 " + String(valorMQ4) + "% MQ9 " + String(valorMQ9) + "%", "");
      alertaGas = true;
    } else if (valorMQ4 <= uMq4 && valorMQ9 <= uMq9) alertaGas = false;

    // Revisar mensajes de Telegram cada 2 segundos
    if (now - lastCheckTelegram > 2000) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages) {
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastCheckTelegram = now;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}



void loop() {
}

//========FUNCIONES==========

void leerSensores() {
  //lecturaMQ4 = analogRead(MQ4_PIN);
  //valorMQ4 = map(lecturaMQ4, 0, 4095, 0, 100);
  //lecturaMQ9 = analogRead(MQ9_PIN);
  //valorMQ9 = map(lecturaMQ9, 0, 4095, 0, 100);
  // valorLDR = map(analogRead(LDR_PIN), 0, 4095, 0, 100);

  //sensors_event_t humidity, temp;
  //aht.getEvent(&humidity, &temp);

  //t = temp.temperature;
  // h = humidity.relative_humidity;

  // Serial.printf("Temp: %.1f°C  Hum: %.1f%%  MQ4:%d%% MQ9:%d%% LDR:%d%%\n",
  //  t, h, valorMQ4, valorMQ9, valorLDR);
}


void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "🚫 Usuario no autorizado", "");
      continue;
    }

    if (text == "estado" || text == "Estado" || text == "/estado") {
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);
      String mensaje = "📊 Estado actual:\n";
      mensaje += "🌡 Temp: " + String(temp.temperature, 1) + " °C (umbral " + String(uTemp) + ")\n";
      mensaje += "💧 Hum: " + String(humidity.relative_humidity, 1) + "% (umbral " + String(uHum) + ")\n";
      mensaje += "💨 MQ4: " + String(valorMQ4) + "%  MQ9: " + String(valorMQ9) + "%\n";
      bot.sendMessage(CHAT_ID, mensaje, "");
    } else {
      bot.sendMessage(CHAT_ID, "Comando no reconocido. Escribí 'estado' para ver valores actuales.", "");
    }
  }
}

//========================== PANTALLAS =======================================

void menu(void) {
  lcd.setCursor(0, 0);
  lcd.print("MENU:1:TEM 2:LUZ");
  lcd.setCursor(0, 1);
  lcd.print("3:GAS 4:GMT 5:EN");
}

void ptemp(void) {
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(uTemp);
  lcd.print(" C   ");
  lcd.setCursor(0, 1);
  lcd.print("HUM : ");
  lcd.print(uHum);
  lcd.print(" %   ");
}

void pluz(void) {
  lcd.setCursor(0, 0);
  lcd.print(" UMBRAL DE LUZ ");
  lcd.setCursor(0, 1);
  lcd.print(uLuz);
  lcd.print(" %   ↑↓ Cambiar");
}

void pgas(void) {
  lcd.setCursor(0, 0);
  lcd.print("MQ4: ");
  lcd.print(uMq4);
  lcd.print(" %     ");
  lcd.setCursor(0, 1);
  lcd.print("MQ9: ");
  lcd.print(uMq9);
  lcd.print(" %     ");
}

void pgmt(void) {
  lcd.setCursor(0, 0);
  lcd.print(" AJUSTE DE GMT ");
  lcd.setCursor(0, 1);
  lcd.print("UTC ");
  if (uGmt >= 0) lcd.print("+");
  lcd.print(uGmt);
  lcd.print("   ↑↓ Cambiar");
}

void penvio(void) {
  lcd.setCursor(0, 0);
  lcd.print(" INTERVALO ENVIO ");
  lcd.setCursor(0, 1);
  lcd.print(uEnvio);
  lcd.print(" s   ↑↓ Cambiar");
}

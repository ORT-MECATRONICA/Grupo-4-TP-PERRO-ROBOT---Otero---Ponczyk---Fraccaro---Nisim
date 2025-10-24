//Grupo 4 Otero - Fraccaro - Ponczyk - Nisim

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x27

#define SDA_PIN 21  // Pin SDA I2C1
#define SCL_PIN 22  // Pin SCL I2C1
#define MQ4_PIN 32
#define MQ9_PIN 33
#define LDR_PIN 34
#define BOTON1 18
#define BOTON2 19
#define BOTON3 21
#define BOTON4 22
#define BOTON5 23
#define LED1 25
#define LED2 26
#define LED3 27

void printBMP_OLED(void);
void printBMP_OLED2(void);
void printBMP_OLED3(void);
void printBMP_OLED4(void);
void printBMP_OLED5(void);
void printBMP_OLED6(void);


Adafruit_AHTX0 aht;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);





#define RST 0
#define P1 0
#define TEMP 1
#define LUZ 2
#define GAS 3
#define GMT 4
#define ENVIO 5
#define ESPERA1 10
#define ESPERA2 14
#define ESPERA3 15
#define ESPERA4 28
#define ESPERA5 23
#define ESPERA6 96
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

int estado = 0;
int lecturaMQ4;
int valorMQ4;
int lecturaMQ9;
int valorMQ9;

int uTemp = 23;
int uHum = 50;
int uLuz = 50;
int uMq4 = 50;
int uMq9 = 50;
int uGmt = 0;
int uEnvio = 30;


void setup() {
  Serial.begin(115200);
  pinMode(BOTON1, INPUT);
  pinMode(BOTON2, INPUT);
  pinMode(BOTON3, INPUT);
  pinMode(BOTON4, INPUT);
  pinMode(BOTON5, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);


  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  if (!aht.begin(&Wire)) {
    Serial.println("❌ Error: No se detectó el AHT10. Revisá las conexiones.");
    while (1)
      ;
  }


  //EN EL LOOP
  //GAS
  lecturaMQ4 = analogRead(MQ4_PIN);  // Lee valor analógico
  valorMQ4 = map(lecturaMQ4, 0, 4095, 0, 100);
  lecturaMQ9 = analogRead(MQ9_PIN);
  valorMQ9 = map(lecturaMQ9, 0, 4095, 0, 100);
  Serial.print("Valor MQ-4: ");
  Serial.print(valorMQ4);
  Serial.println("%");
  Serial.print("Valor MQ-9: ");
  Serial.print(valorMQ9);
  Serial.println("%");

  //LDR
  int lecturaLDR = analogRead(LDR_PIN);  // Lee valor analógico
  int valorLDR = map(lecturaLDR, 0, 4095, 0, 100);
  Serial.print("Valor LDR: ");
  Serial.print(valorLDR);
  Serial.println("%");

  //TEMPERATURA
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("🌡️  Temperatura: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");
  Serial.print("💧 Humedad: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  //DISPLAY
  lcd.setCursor(0, 1);
  lcd.print("Contador: ");
}

void loop() {

  switch (estado) {
    case RST:

      millis_valor = millis();
      estado = P1;
      break;

    case P1:
      Serial.println("p1");

      printBMP_OLED();
      if (digitalRead(BOTON1) == LOW) {
        estado = ESPERA1;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = ESPERA2;
      }
      if (digitalRead(BOTON3) == LOW) {
        estado = ESPERA3;
      }
      if (digitalRead(BOTON4) == LOW) {
        estado = ESPERA4;
      }
      if (digitalRead(BOTON5) == LOW) {
        estado = ESPERA5;
      }

      break;
    case ESPERA1:
      if (digitalRead(BOTON1) == HIGH) {
        Serial.println("boton 1  soltado");
        estado = TEMP;
      }
      break;

    case ESPERA2:
      if (digitalRead(BOTON2) == HIGH) {
        Serial.println("boton 2  soltado");
        estado = LUZ;
      }
      break;

    case ESPERA3:
      if (digitalRead(BOTON3) == HIGH) {
        Serial.println("boton 3  soltado");
        estado = GAS;
      }
      break;

    case ESPERA4:
      if (digitalRead(BOTON4) == HIGH) {
        Serial.println("boton 4  soltado");
        estado = GMT;
      }
      break;

    case ESPERA5:
      if (digitalRead(BOTON5) == HIGH) {
        Serial.println("boton 5  soltado");
        estado = ENVIO;
      }
      break;

    case ESPERA6:
      if (digitalRead(BOTON5) == HIGH) {
        Serial.println("boton 5  soltado");
        estado = P1;
      }
      break;

    case TEMP:
      if (digitalRead(BOTON1) == LOW) {
        estado = SUMATEMP;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = RESTATEMP;
      }
      if (digitalRead(BOTON3) == LOW) {
        estado = SUMAHUM;
      }
      if (digitalRead(BOTON4) == LOW) {
        estado = RESTAHUM;
      }
      break;

    case LUZ:
      if (digitalRead(BOTON1) == LOW) {
        estado = SUMALUZ;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = RESTALUZ;
      }

      break;

    case GAS:
      if (digitalRead(BOTON1) == LOW) {
        estado = SUMAMQ4;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = RESTAMQ4;
      }
      if (digitalRead(BOTON3) == LOW) {
        estado = SUMAMQ9;
      }
      if (digitalRead(BOTON4) == LOW) {
        estado = RESTAMQ9;
      }
      break;

    case GMT:
      if (digitalRead(BOTON1) == LOW) {
        estado = SUMAGMT;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = RESTAGMT;
      }

      break;

    case ENVIO:
      if (digitalRead(BOTON1) == LOW) {
        estado = SUMAENVIO;
      }
      if (digitalRead(BOTON2) == LOW) {
        estado = RESTAENVIO;
      }

      break;

    case SUMATEMP:
      if (digitalRead(BOTON1) == HIGH) {
        uTemp = uTemp + 1;
        estado = TEMP;
      }

      break;

    case RESTATEMP:
      if (digitalRead(BOTON2) == HIGH) {
        uTemp = uTemp - 1;
        if (uTemp < 0) {
          uTemp = 0;
        }
        estado = TEMP;
      }
      break;

    case SUMAHUM:
      if (digitalRead(BOTON3) == HIGH) {
        uHum = uHum + 1;
        if (uHum > 100) {
          uHum = 0;
        }
        estado = TEMP;
      }
      break;

    case RESTAHUM:
      if (digitalRead(BOTON4) == HIGH) {
        uHum = uHum - 1;
        if (uHum < 0) {
          uHum = 100;
        }
        estado = TEMP;
      }
      break;

    case SUMALUZ:
      if (digitalRead(BOTON1) == HIGH) {
        uLuz = uLuz + 1;
        if (uLuz > 100) {
          uLuz = 0;
        }
        estado = LUZ;
      }
      break;

    case RESTALUZ:
      if (digitalRead(BOTON2) == HIGH) {
        uLuz = uLuz - 1;
        if (uLuz < 0) {
          uLuz = 100;
        }
        estado = LUZ;
      }
      break;

    case SUMAMQ4:
      if (digitalRead(BOTON1) == HIGH) {
        uMq4 = uMq4 + 1;
        if (uMq4 > 100) {
          uMq4 = 0;
        }
        estado = GAS;
      }
      break;

    case RESTAMQ4:
      if (digitalRead(BOTON2) == HIGH) {
        uMq4 = uMq4 - 1;
        if (uMq4 < 0) {
          uMq4 = 100;
        }
        estado = GAS;
      }
      break;

    case SUMAMQ9:
      if (digitalRead(BOTON3) == HIGH) {
        uMq9 = uMq9 + 1;
        if (uMq9 > 100) {
          uMq9 = 0;
        }
        estado = GAS;
      }
      break;

    case RESTAMQ9:
      if (digitalRead(BOTON4) == HIGH) {
        uMq9 = uMq9 - 1;
        if (uMq9 < 0) {
          uMq9 = 100;
        }
        estado = GAS;
      }
      break;

    case SUMAGMT:
      if (digitalRead(BOTON1) == HIGH) {
        uGmt++;
        if (uGmt > 12) {
          uGmt = -12;
        }
        estado = GMT;
      }
      break;

    case RESTAGMT:
      if (digitalRead(BOTON2) == HIGH) {
        uGmt--;
        if (uGmt < 12) {
          uGmt = 12;
        }
        estado = GMT;
      }
      break;

    case SUMAENVIO:
      if (digitalRead(BOTON1) == HIGH) {
        uEnvio = uEnvio + 30;
        estado = ENVIO;
      }
      break;

    case RESTAENVIO:
      if (digitalRead(BOTON2) == HIGH) {
        uEnvio = uEnvio - 30;
        if (uEnvio < 30) {
          uEnvio = 30;
        }
        estado = ENVIO;
      }
      break;
  }
}

void menu(void) {
  char stringU[5];
  char stringtemp[5];
  u8g2.clearBuffer();                 // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  sprintf(stringtemp, "%.2f", temp);  ///convierto el valor float a string
  sprintf(stringU, "%d", valorU);     ///convierto el valor float a string
  u8g2.drawStr(0, 35, "T. Actual:");
  u8g2.drawStr(60, 35, stringtemp);
  u8g2.drawStr(90, 35, " C");
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}

void ptemp(void) {
  char stringU[5];
  u8g2.clearBuffer();  // clear the internal memory
  sprintf(stringU, "%d", valorU);
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}

void pluz(void) {
  char stringU[5];
  char stringtemp[5];
  u8g2.clearBuffer();                 // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  sprintf(stringtemp, "%.2f", temp);  ///convierto el valor float a string
  sprintf(stringU, "%d", valorU);     ///convierto el valor float a string
  u8g2.drawStr(0, 35, "T. Actual:");
  u8g2.drawStr(60, 35, stringtemp);
  u8g2.drawStr(90, 35, " C");
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}

void pgas(void) {
  char stringU[5];
  u8g2.clearBuffer();  // clear the internal memory
  sprintf(stringU, "%d", valorU);
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}
void pgmt(void) {
  char stringU[5];
  char stringtemp[5];
  u8g2.clearBuffer();                 // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  sprintf(stringtemp, "%.2f", temp);  ///convierto el valor float a string
  sprintf(stringU, "%d", valorU);     ///convierto el valor float a string
  u8g2.drawStr(0, 35, "T. Actual:");
  u8g2.drawStr(60, 35, stringtemp);
  u8g2.drawStr(90, 35, " C");
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}

void penvio(void) {
  char stringU[5];
  u8g2.clearBuffer();  // clear the internal memory
  sprintf(stringU, "%d", valorU);
  u8g2.setFont(u8g2_font_t0_11b_tr);  // choose a suitable font
  u8g2.drawStr(0, 50, "V. Umbral:");
  u8g2.drawStr(60, 50, stringU);
  u8g2.drawStr(75, 50, " C");
  u8g2.sendBuffer();  // transfer internal memory to the display
}
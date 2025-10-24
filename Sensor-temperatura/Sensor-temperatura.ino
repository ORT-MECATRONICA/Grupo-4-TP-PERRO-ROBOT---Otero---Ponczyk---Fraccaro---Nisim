//Grupo 4 Otero - Fraccaro - Ponczyk - Nisim
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// Definimos pines para I2C1 (podés cambiarlos si usás otros)
#define SDA_PIN 21   // Pin SDA I2C1
#define SCL_PIN 22   // Pin SCL I2C1

// Creamos objeto del sensor AHT10
Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  
  // Iniciar I2C1 en los pines definidos
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Iniciando sensor AHT10...");

  // Inicializamos el sensor
  if (!aht.begin(&Wire)) {
    Serial.println("❌ Error: No se detectó el AHT10. Revisá las conexiones.");
    while (1);
  }

  Serial.println("✅ Sensor AHT10 inicializado correctamente.");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("🌡️  Temperatura: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("💧 Humedad: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.println("------------------------");
  delay(1000);
}

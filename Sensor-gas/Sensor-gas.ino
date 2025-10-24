//Grupo 4 Otero - Fraccaro - Ponczyk - Nisim

#define MQ4_PIN 32
#define MQ9_PIN 33

int lecturaMQ4;
int valorMQ4; 
int lecturaMQ9;
int valorMQ9;

void setup() {
  Serial.begin(115200);
  Serial.println("Lectura de Sensor de Gas");
}

void loop() {
  lecturaMQ4 = analogRead(MQ4_PIN);  // Lee valor analógico
  valorMQ4 = map(lecturaMQ4, 0, 4095, 0, 100);
  lecturaMQ9 = analogRead(MQ9_PIN);
  valorMQ9 = map(lecturaMQ9,0,4095,0,100);

  Serial.print("Valor MQ-4: ");
  Serial.print(valorMQ4);
  Serial.println("%");
  Serial.print("Valor MQ-9: ");
  Serial.print(valorMQ9);
  Serial.println("%");

  delay(500);  // Medio segundo entre lecturas
}

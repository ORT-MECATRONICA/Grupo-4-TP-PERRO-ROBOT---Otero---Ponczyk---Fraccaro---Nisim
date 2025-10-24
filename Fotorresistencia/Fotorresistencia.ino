//Grupo 4 Otero - Fraccaro - Ponczyk - Nisim

#define LDR_PIN 34

void setup() {
  Serial.begin(115200);
  Serial.println("Lectura de Fotorresistencia (LDR)");
}

void loop() {
  int lecturaLDR = analogRead(LDR_PIN);  
  int valorLDR = map(lecturaLDR, 0, 4095, 0, 100);
  Serial.print("Valor LDR: ");
  Serial.print(valorLDR);
  Serial.println("%");

  delay(500);  
}

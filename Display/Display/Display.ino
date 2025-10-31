#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define LCD_ADDR 0x27  

// Creamos objeto LCD: (dirección, columnas, filas)
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// Pines I2C1
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  
  lcd.init();         // Inicializamos la pantalla LCD
  lcd.backlight();  // Encender luz de fondo

  lcd.setCursor(0, 0);
  lcd.print("Hola, Roman!");


  Serial.println("Pantalla LCD inicializada correctamente.");
}

void loop() {
  static int contador = 0;

  lcd.setCursor(0, 1);
  lcd.print("Contador: ");
  lcd.print(contador);
  lcd.print("   ");  // Borra posibles restos

  contador++;
  if (contador > 999) contador = 0;

  delay(1000);
}

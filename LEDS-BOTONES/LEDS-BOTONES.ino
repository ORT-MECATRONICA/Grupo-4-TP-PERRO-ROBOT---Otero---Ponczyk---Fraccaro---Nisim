//Grupo 4 Otero - Fraccaro - Ponczyk - Nisim

#define BOTON1 18
#define BOTON2 19
#define BOTON3 21
#define BOTON4 22
#define BOTON5 23
#define LED1 25
#define LED2 26
#define LED3 27

void setup() {
  Serial.begin(115200);
  pinMode(BOTON1,INPUT_PULLUP);
  pinMode(BOTON2,INPUT_PULLUP);
  pinMode(BOTON3,INPUT_PULLUP);
  pinMode(BOTON4,INPUT_PULLUP);
  pinMode(BOTON5,INPUT_PULLUP);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
}

void loop() {
  if(digitalRead(BOTON1)==LOW){
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
  }
  if(digitalRead(BOTON2)==LOW){
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,LOW);
  }
  if(digitalRead(BOTON3)==LOW){
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,HIGH);
  }
  if(digitalRead(BOTON4)==LOW){
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,LOW);
  }
  if(digitalRead(BOTON5)==LOW){
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,HIGH);
  }
}

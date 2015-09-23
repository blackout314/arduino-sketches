#include <Time.h>

int MOIST_PIN = A0;
int LDR_PIN = A1;  //define a pin for Photo resistor
int MOIST_VAL = 0;

int WHITE = 11;
int RED = 13;
int BLUE = 12;

void setup(){
  Serial.begin(9600);
  setTime(0);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT);
}

void loop(){
  MOIST_VAL = analogRead(MOIST_PIN);
  
  if(MOIST_VAL < 700){
    digitalWrite(BLUE, HIGH);
    digitalWrite(RED, LOW);
  }else{
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, LOW);
  }

  
  if(second() > 10 && (analogRead(LDR_PIN) < 500)){
    //Serial.println("Not Enough Light");
    Blink();
    setTime(0);
  }

  delay(100);
}

void Blink(){
  for(int i = 0; i < 20; i++){
    digitalWrite(WHITE, HIGH);
    delay(250);
    digitalWrite(WHITE, LOW);
    delay(250);
  }
}


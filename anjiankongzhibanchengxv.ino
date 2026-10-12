#include<SoftwareSerial.h>
SoftwareSerial buttonSerial(6,7);
const int button1=2;
const int button2=3;
const int button3=4;
const int button4=5;
int lastState1=HIGH;
int lastState2=HIGH;
int lastState3=HIGH;
int lastState4=HIGH;
void setup() {
  // put your setup code here, to run once:
pinMode(button1,INPUT_PULLUP);
pinMode(button2,INPUT_PULLUP);
pinMode(button3,INPUT_PULLUP);
pinMode(button4,INPUT_PULLUP);
buttonSerial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  checkButton(button1,lastState1,'1');
  checkButton(button2,lastState2,'2');
  checkButton(button3,lastState3,'3');
  checkButton(button4,lastState4,'4');
}
void checkButton(int pin,int &lastState,char command){
  int currentState=digitalRead(pin);
  if(currentState!=lastState){
    delay(20);
    currentState=digitalRead(pin);
    if(currentState==LOW&&lastState==HIGH){
      buttonSerial.write(command);
    }
    lastState=currentState;
  }
}

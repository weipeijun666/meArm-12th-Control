#include<SoftwareSerial.h>
SoftwareSerial buttonSerial(6,7);//r，t
const int button1=2;
const int button2=3;
const int button3=4;
const int button4=5;
const int button5=8;
const int button6=9;
const int button7=10;
const int button8=11;

int lastState1=HIGH;
int lastState2=HIGH;
int lastState3=HIGH;
int lastState4=HIGH;
int lastState5=HIGH;
int lastState6=HIGH;
int lastState7=HIGH;
int lastState8=HIGH;
void setup() {
  // put your setup code here, to run once:
pinMode(button1,INPUT_PULLUP);
pinMode(button2,INPUT_PULLUP);
pinMode(button3,INPUT_PULLUP);
pinMode(button4,INPUT_PULLUP);
pinMode(button5,INPUT_PULLUP);
pinMode(button6,INPUT_PULLUP);
pinMode(button7,INPUT_PULLUP);
pinMode(button8,INPUT_PULLUP);
buttonSerial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  checkButton(button1,lastState1,'1');
  checkButton(button2,lastState2,'2');
  checkButton(button3,lastState3,'3');
  checkButton(button4,lastState4,'4');
  checkButton(button5,lastState5,'5');
  checkButton(button6,lastState6,'6');
  checkButton(button7,lastState7,'7');
  checkButton(button8,lastState8,'8');
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

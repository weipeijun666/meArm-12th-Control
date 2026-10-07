#include<Wire.h>
#include<LU9685.h>

LU9685 board;

int joy1x=A0;
int joy1y=A1;
int joy2x=A2;
int joy2y=A3;

const int baseMin=0;
const int baseMax=180;
const int rArmMin=0;
const int rArmMax=180;
const int fArmMin=0;
const int fArmMax=180;
const int clawMin=0;
const int clawMax=180;

int baseAngle=90;
int rArmAngle=90;
int fArmAngle=90;
int clawAngle=90;

int DSD=15;
int mode=0;//默认0：摇杆模式，1：键盘摇杆模式，2：键盘指令模式
int moveStep=3;


void setup() {
  Wire.begin();
  board.beginI2C(0x80,50);

  board.setPos(0,90);
  delay(10);
  board.setPos(1,90);
  delay(10);
  board.setPos(2,90);
  delay(10);
  board.setPos(3,90);
  delay(10);

  Serial.begin(9600);
  Serial.println("Welcome to My Robot Arm product");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(mode==0){servoJoyCmd();}
  if(Serial.available()>0){
    char serialCmd=Serial.read();
    switch(serialCmd){
      case 'O':
      Serial.println("+Command : Claw Open!");
      servoCmd('c',25,DSD);
      break;

      case'S':
      Serial.println("+Command : Claw Shut!");
      servoCmd('c',100,DSD);
      break;
      
      case'H':
      if(DSD-3>=0){
        Serial.println("+Command : Speed high");
        DSD-=3;
      }else{
        Serial.println("+Warning : Speed limit reached!");
      }
      break;
      
      case'L':
      Serial.println("+Command : Speed low");
        DSD+=3;
        break;
        
      default:
    
    if(mode==2){
      armDataCmd(serialCmd);
    }
    
    else if(mode==1){
      armKeyboardJoyCmd(serialCmd);
    }
    }
  }
}
void servoJoyCmd(){
  
    int baseJoyPos;
    int rArmJoyPos;
    int fArmJoyPos;
    int clawJoyPos;
    int t;
    int m1x=analogRead(joy1x);
    int m1y=analogRead(joy1y);
    int m2x=analogRead(joy2x);
    int m2y=analogRead(joy2y);
    
    
   if(m1x<256){
    Serial.println("Received Command: Base Turn Left");
    baseJoyPos=baseAngle-moveStep;
    servoCmd('b',baseJoyPos,DSD);
   }

    if(m1x>767){
    Serial.println("Received Command: Base Turn Right");
    baseJoyPos=baseAngle+moveStep;
    servoCmd('b',baseJoyPos,DSD);
    }

    if(m1y>767){
    Serial.println("Received Command: Rear Arm Down");
    rArmJoyPos=rArmAngle+moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m1y<256){
    Serial.println("Received Command: Rear Arm Up");
    rArmJoyPos=rArmAngle-moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m2y<256){
    Serial.println("Received Command: Front Arm Up");
    fArmJoyPos=fArmAngle+moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2y>767){
    Serial.println("Received Command: Front Arm Down");
    fArmJoyPos=fArmAngle-moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2x<256){
    Serial.println("Received Command: Claw Close Down");
    clawJoyPos=clawAngle+moveStep;
    servoCmd('c',clawJoyPos,DSD);
    }

    if(m2x>767){
    Serial.println("Received Command: Claw Open Up");
    clawJoyPos=clawAngle-moveStep;
    servoCmd('c',clawJoyPos,DSD);
    }

    if(Serial.available()>0){
      char r=Serial.read();

      if(r=='b'||r=='c'||r=='f'||r=='r'||r=='w'||r=='s'||r=='a'||r=='d'||r=='5'||r=='8'||r=='4'||r=='6'){
      Serial.println("+Warning:Robot in Joy-Stick Mode");
      delay(100);
      while(Serial.available()>0) char wrongCommand=Serial.read();
      return;
      }
    
      switch(r){
      case 'O':
      Serial.println("+Command : Claw Open!");
      servoCmd('c',25,DSD);
      break;

      case'S':
      Serial.println("+Command : Claw Shut!");
      servoCmd('c',100,DSD);
      break;
      
      case'H':
      if(DSD-3>=0){
        Serial.println("+Command : Speed high");
        DSD-=3;
      }else{
        Serial.println("+Warning : Speed limit reached!");
      }
      break;
      
      case'L':
      Serial.println("+Command : Speed low");
        DSD+=3;
        break;
        
      case 'm':
      mode=1;
      Serial.println("Command: Switch to Keyboard-Joy-Stick Mode.");
      break;
  
      case'o':
      reportStatus();
      break;
  
      case'i':
      armIniPos();
      break;
  
      default:
      Serial.println("Unknown Command.");
      return;
      }
    }
}

void armKeyboardJoyCmd(char serialCmd){
  if(serialCmd=='b'||serialCmd=='c'||serialCmd=='f'||serialCmd=='r'){
    Serial.println("+Warning:Robot in Keyboard-Joy-Stick Mode");
    delay(100);
    while(Serial.available()>0) char wrongCommand=Serial.read();
    return;
    
  }
  int baseJoyPos;
  int rArmJoyPos;
  int fArmJoyPos;
  int clawJoyPos;
  switch(serialCmd){
    case'a':
    Serial.println("Received Command: Base Turn Left");
    baseJoyPos=baseAngle-moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case'd':
    Serial.println("Received Command: Base Turn Right");
    baseJoyPos=baseAngle+moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case's':
    Serial.println("Received Command: Rear Arm Down");
    rArmJoyPos=rArmAngle+moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    break;

    case'w':
    Serial.println("Received Command: Rear Arm Up");
    rArmJoyPos=rArmAngle-moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    break;

    case'8':
    Serial.println("Received Command: Front Arm Up");
    fArmJoyPos=fArmAngle+moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    break;

    case'5':
    Serial.println("Received Command: Front Arm Down");
    fArmJoyPos=fArmAngle-moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    break;

    case'4':
    Serial.println("Received Command: Claw Close Down");
    clawJoyPos=clawAngle+moveStep;
    servoCmd('c',clawJoyPos,DSD);
    break;

    case'6':
    Serial.println("Received Command: Claw Open Up");
    clawJoyPos=clawAngle-moveStep;
    servoCmd('c',clawJoyPos,DSD);
    break;

    case 'm':
    mode=2;
    Serial.println("Command: Switch to Instruction Mode.");
    break;

    case'o':
    reportStatus();
    break;

    case'i':
    armIniPos();
    break;

    default:
    Serial.println("Unknown Command.");
    return;
  }
}

void armDataCmd(char serialCmd){
  if(serialCmd=='w'||serialCmd=='s'||serialCmd=='a'||serialCmd=='d'||serialCmd=='5'||serialCmd=='8'||serialCmd=='4'||serialCmd=='6'){
    Serial.println("+Warning:Robot in Instruction Mode");
    delay(100);
    while(Serial.available()>0) char wrongCommand=Serial.read();
    return;
  }

  if(serialCmd=='b'||serialCmd=='c'||serialCmd=='f'||serialCmd=='r'){
    int servoData=Serial.parseInt();
    servoCmd(serialCmd,servoData,DSD);
  }else{
    switch(serialCmd){
      case 'm':
      mode=0;
      Serial.println("Cpmmand:Switch to Joy-Stick Mode.");
      break;

      case'0':
      reportStatus();
      break;

      case 'i':
      armIniPos();
      break;

      default:
      Serial.println("Unknown Command.");
      
    }
  }
}


void servoCmd(char servoName,int toPos,int servoDelay){

  Serial.println("");
  Serial.print("+Command:Servo ");
  Serial.print(servoName);
  Serial.print(" to ");
  Serial.print(toPos);
  Serial.print(" at servoDelay value ");
  Serial.print(servoDelay);
  Serial.println(".");
  Serial.println("");

  int fromPos;
  int channel;

  switch(servoName){
    case 'b':
    if(toPos>=baseMin&&toPos<=baseMax){
      fromPos=baseAngle;
      channel=0;
      break;
    }else{
      Serial.println("+Warning:Base Servo Value Out Of Limit!");
      return;
    }

    case 'c':
    if(toPos>=clawMin&&toPos<=clawMax){
      fromPos=clawAngle;
      channel=3;
      break;
    }else{
      Serial.println("+Warning:Claw Servo Value Out Of Limit!");
      return;
    }

    case 'f':
    if(toPos>=fArmMin&&toPos<=fArmMax){
      fromPos=fArmAngle;
      channel=2;
      break;
    }else{
      Serial.println("+Warning:fArm Servo Value Out Of Limit!");
      return;
    }

    case 'r':
    if(toPos>=rArmMin&&toPos<=rArmMax){
      fromPos=rArmAngle;
      channel=1;
      break;
    }else{
      Serial.println("+Warning:fArm Servo Value Out Of Limit!");
      return;
    }
  }


  if(fromPos<=toPos){
    for(int i=fromPos;i<=toPos;i++){
      board.setPos(channel,i);
      delay(servoDelay);
    }
  }else{
    for(int i=fromPos;i>=toPos;i--){
      board.setPos(channel,i);
      delay(servoDelay);
    }
  }

  if(servoName=='b'){
    baseAngle=toPos;
  }
  if(servoName=='c'){
    clawAngle=toPos;
  }
  if(servoName=='f'){
    fArmAngle=toPos;
  }
  if(servoName=='r'){
    rArmAngle=toPos;
  }
}



void reportStatus(){
  Serial.println("");
  Serial.println("");
  Serial.println("+Robot-Arm Status Report +");
  Serial.print("Claw Position:");Serial.println(clawAngle);
  Serial.print("Base Position:");Serial.println(baseAngle);
  Serial.print("Rear Arm Position:");Serial.println(rArmAngle);
  Serial.print("Front Arm Position:");Serial.println(fArmAngle);
  Serial.println("+++++++++++++++++++++++++");
  Serial.println("");
  
}


void armIniPos(){
  Serial.println("+Command: Restore Initial Position.");
  int robotIniPosArray[4][3]={
    {'b',90,DSD},
    {'r',90,DSD},
    {'f',90,DSD},
    {'c',90,DSD}
  };

  for(int i=0;i<4;i++){
    servoCmd(robotIniPosArray[i][0],robotIniPosArray[i][1],robotIniPosArray[i][2]);
  }
}

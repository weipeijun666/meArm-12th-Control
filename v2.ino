#include <Servo.h>
Servo base,fArm,rArm,claw;

const int baseMin=0;
const int baseMax=180;
const int rArmMin=45;
const int rArmMax=180;
const int fArmMin=35;
const int baseMax=120;
const int clawMin=25;
const int clawMax=100;

int DSD=15;
bool mode;
int moveStep=3;


void setup() {
  // put your setup code here, to run once:
  base.attach(11);
  delay(200);
  rArm.attach(10);
  delay(200);
  fArm.attach(9);
  delay(200)
  claw.attach(6);
  delay(200);

  base.write(90);
  delay(10);
  fArm.write(90);
  delay(10);
  rArm.write(90);
  delay(10);
  claw.write(90);
  delay(10);

  Serial.begin(9600);
  Serial.println("Welcome to My Robot Arm product");
}

void loop() {
  // put your main code here, to run repeatedly:
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
    
    if(mode==1){
      armDataCmd(serialCmd);
    }else{
      armJoyCmd(serialCmd);
    }
    }
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

void armJoyCmd(char serialCmd){
  if(serialCmd=='b'||serialCmd=='c'||serialCmd=='f'||serialCmd=='r'){
    Serial.println("+Warning:Robot in Joy-Stick Mode");
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
    baseJoyPos=base.read()-moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case'd':
    Serial.println("Received Command: Base Turn Right");
    baseJoyPos=base.read()+moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case's':
    Serial.println("Received Command: Rear Arm Down");
    rArmJoyPos=base.read()+moveStep;
    servoCmd('r',baseJoyPos,DSD);
    break;

    case'w':
    Serial.println("Received Command: Rear Arm Up");
    rArmJoyPos=base.read()-moveStep;
    servoCmd('r',baseJoyPos,DSD);
    break;

    case'8':
    Serial.println("Received Command: Front Arm Up");
    fArmJoyPos=base.read()+moveStep;
    servoCmd('f',baseJoyPos,DSD);
    break;

    case'5':
    Serial.println("Received Command: Front Arm Down");
    fArmJoyPos=base.read()-moveStep;
    servoCmd('f',baseJoyPos,DSD);
    break;

    case'4':
    Serial.println("Received Command: Claw Close Down");
    clawJoyPos=base.read()+moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case'6':
    Serial.println("Received Command: Claw Open Up");
    baseJoyPos=base.read()-moveStep;
    clawCmd('b',baseJoyPos,DSD);
    break;

    case 'm':
    mode=1;
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
void servoCmd(char servoName,int toPos,int servoDelay){
  Servo servo2go;

  Serial.println("");
  Serial.print("+Command:Servo ");
  Serial.print(servoName);
  Serial.print(" to ");
  Serial.print(toPos);
  Serial.print(" at servoDelay value ");
  Serial.print(servoDelay);
  Serial.println(".");
  Serial,println("");

  int fromPos;

  switch(servoName){
    case 'b':
    if(toPos>=baseMin&&toPos<=baseMax){
      servo2go=base;
      fromPos=base.read();
      break;
    }else{
      Serial.println("+Warning:Base Servo Value Out Of Limit!");
      return;
    }

    case 'c':
    if(toPos>=clawMin&&toPos<=clawMax){
      servo2go=claw;
      fromPos=claw.read();
      break;
    }else{
      Serial.println("+Warning:Claw Servo Value Out Of Limit!");
      return;
    }

    case 'f':
    if(toPos>=fArmMin&&toPos<=fArmMax){
      servo2go=fArm;
      fromPos=fArm.read();
      break;
    }else{
      Serial.println("+Warning:fArm Servo Value Out Of Limit!");
      return;
    }

    case 'r':
    if(toPos>=rArmMin&&toPos<=rArmMax){
      servo2go=rArm;
      fromPos=rArm.read();
      break;
    }else{
      Serial.println("+Warning:fArm Servo Value Out Of Limit!");
      return;
    }
  }


  if(fromPos<=toPos){
    for(int i=fromPos;i<=toPos;i++){
      servo2go.write(i);
      delay(servoDelay);
    }
  }else{
    for(int i=fromPos;i>=toPos;i--){
      servo2go.write(i);
      delay(servoDelay);
    }
  }
}

void reportStatus(){
  Serial.println("");
  Serial.println("");
  Serial.println("+Robot-Arm Status Report +");
  Serial.print("Claw Position:");Serial.println(claw.read());
  Serial.print("Base Position:");Serial.println(base.read());
  Serial.print("Rear Arm Position:");Serial.println(rArm.read());
  Serial.print("Front Arm Position:");Serial.println(fArm.read());
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

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
const int fArmMin=20;
const int fArmMax=135;
const int clawMin=0;
const int clawMax=180;
//动作流
//-------------------------------------------------------
//A动作一：回到初始位置，打开夹爪，移动到物体A上方，下降并移动到能直接夹取的位置,夹取。
int A_Action1[][2]={
  {'b',90},{'r',90},{'f',90},{'c',25},{'r',125},{'f',40},{'r',137},{'c',110}
};
//A动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int A_Action2[][2]={
  {'f',90},{'r',125},{'b',76},{'r',95},{'f',105},{'r',122},{'f',26},{'c',25}
};
//A动作三：大臂小臂依次抬起，并回到初始位置，夹爪关闭。
int A_Action3[][2]={
  {'r',95},{'f',105},{'b',90},{'r',90},{'f',90},{'c',110}
};
//B动作一：回到初始位置，打开夹爪，移动到物体A上方，下降并移动到能直接夹取的位置,夹取。
int B_Action1[][2]={
  {'b',90},{'r',90},{'f',90},{'c',25},{'r',125},{'f',40},{'r',137},{'c',110}
};
//B动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int B_Action2[][2]={
  {'f',90},{'r',125},{'b',76},{'r',95},{'f',105},{'r',122},{'f',26},{'c',25}
};
//B动作三：大臂小臂依次抬起，并回到初始位置，夹爪关闭。
int B_Action3[][2]={
  {'r',95},{'f',105},{'b',90},{'r',90},{'f',90},{'c',110}
};
//C动作一：回到初始位置，打开夹爪，移动到物体A上方，下降并移动到能直接夹取的位置,夹取。
int C_Action1[][2]={
  {'b',90},{'r',90},{'f',90},{'c',25},{'r',125},{'f',40},{'r',137},{'c',110}
};
//C动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int C_Action2[][2]={
  {'f',90},{'r',125},{'b',76},{'r',95},{'f',105},{'r',122},{'f',26},{'c',25}
};
//C动作三：大臂小臂依次抬起，并回到初始位置，夹爪关闭。
int C_Action3[][2]={
  {'r',95},{'f',105},{'b',90},{'r',90},{'f',90},{'c',110}
};
//-------------------------------------------------------
//姿态储存数组（每一个数组包含一个底盘，后臂和前臂的角度）
//-------------------------------
/*int homePosition[3]={90,90,90};
int AAbovePosition[3]={90,125,90};
int APickPosition[3]={90,137,40};
int APlaceAbovePosition[3]={76,95,105};
int APlacePosition[3]={76,122,26};
int BAbovePosition[3]={110,65,115};
int BPickPosition[3]={110,75,105};
int BPlaceAbovePosition[3]={140,60,120};
int BPlacePosition[3]={140,70,110};
int CAbovePosition[3]={110,65,115};
int CPickPosition[3]={110,75,105};
int CPlaceAbovePosition[3]={140,60,120};
int CPlacePosition[3]={140,70,110};*/
//-------------------------------
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
  Serial.println(F("Welcome to My Robot Arm product"));
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(mode==0){servoJoyCmd();}
  if(Serial.available()>0){
    char serialCmd=Serial.read();
    if(serialCmd=='\n'||
    serialCmd=='\r'){
      return;
    }
    switch(serialCmd){
      case 'O':
      Serial.println(F("+Command : Claw Open!"));
      servoCmd('c',25,DSD);
      break;

      case'S':
      Serial.println(F("+Command : Claw Shut!"));
      servoCmd('c',100,DSD);
      break;
      
      case'H':
      if(DSD-3>=0){
        Serial.println(F("+Command : Speed high"));
        DSD-=3;
      }else{
        Serial.println(F("+Warning : Speed limit reached!"));
      }
      break;
      
      case'L':
      Serial.println(F("+Command : Speed low"));
        DSD+=3;
        break;

      case'x':
      multiServoCmd();
      break;

      case'A':
      grabAndPlaceA();
      break;

      case'B':
      grabAndPlaceB();
      break;

      case'C':
      grabAndPlaceC();
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
    int m1x=analogRead(joy1x);
    int m1y=analogRead(joy1y);
    int m2x=analogRead(joy2x);
    int m2y=analogRead(joy2y);
    
    
   if(m1x<256){
    Serial.println(F("Received Command: Base Turn Left"));
    baseJoyPos=baseAngle+moveStep;
    servoCmd('b',baseJoyPos,DSD);
   }

    if(m1x>767){
    Serial.println(F("Received Command: Base Turn Right"));
    baseJoyPos=baseAngle-moveStep;
    servoCmd('b',baseJoyPos,DSD);
    }

    if(m1y>767){
    Serial.println(F("Received Command: Rear Arm Down"));
    rArmJoyPos=rArmAngle-moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m1y<256){
    Serial.println(F("Received Command: Rear Arm Up"));
    rArmJoyPos=rArmAngle+moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m2y<256){
    Serial.println(F("Received Command: Front Arm Up"));
    fArmJoyPos=fArmAngle-moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2y>767){
    Serial.println(F("Received Command: Front Arm Down"));
    fArmJoyPos=fArmAngle+moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2x<256){
    Serial.println(F("Received Command: Claw Close Down"));
    clawJoyPos=clawAngle+moveStep;
    servoCmd('c',clawJoyPos,DSD);
    }

    if(m2x>767){
    Serial.println(F("Received Command: Claw Open Up"));
    clawJoyPos=clawAngle-moveStep;
    servoCmd('c',clawJoyPos,DSD);
    }

    if(Serial.available()>0){
      char r=Serial.read();
      if(r=='\n'||r=='\r'){
        return;
      }

      if(r=='b'||r=='c'||r=='f'||r=='r'||r=='w'||r=='s'||r=='a'||r=='d'||r=='5'||r=='8'||r=='4'||r=='6'){
      Serial.println(F("+Warning:Robot in Joy-Stick Mode"));
      delay(100);
      while(Serial.available()>0) char wrongCommand=Serial.read();
      return;
      }
    
      switch(r){
      case 'O':
      Serial.println(F("+Command : Claw Open!"));
      servoCmd('c',25,DSD);
      break;

      case'S':
      Serial.println(F("+Command : Claw Shut!"));
      servoCmd('c',100,DSD);
      break;
      
      case'H':
      if(DSD-3>=0){
        Serial.println(F("+Command : Speed high"));
        DSD-=3;
      }else{
        Serial.println(F("+Warning : Speed limit reached!"));
      }
      break;
      
      case'L':
      Serial.println("+Command : Speed low");
        DSD+=3;
        break;
        
      case 'm':
      mode=1;
      Serial.println(F("Command: Switch to Keyboard-Joy-Stick Mode."));
      break;
  
      case'o':
      reportStatus();
      break;
  
      case'i':
      armIniPos();
      break;

      case'x':
      multiServoCmd();
      break;

      case'A':
      grabAndPlaceA();
      break;

      case'B':
      grabAndPlaceB();
      break;

      case'C':
      grabAndPlaceC();
      break;
      
      default:
      Serial.println(F("Unknown Command."));
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
    Serial.println(F("Received Command: Base Turn Left"));
    baseJoyPos=baseAngle-moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case'd':
    Serial.println(F("Received Command: Base Turn Right"));
    baseJoyPos=baseAngle+moveStep;
    servoCmd('b',baseJoyPos,DSD);
    break;

    case's':
    Serial.println(F("Received Command: Rear Arm Down"));
    rArmJoyPos=rArmAngle+moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    break;

    case'w':
    Serial.println(F("Received Command: Rear Arm Up"));
    rArmJoyPos=rArmAngle-moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    break;

    case'8':
    Serial.println(F("Received Command: Front Arm Up"));
    fArmJoyPos=fArmAngle+moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    break;

    case'5':
    Serial.println(F("Received Command: Front Arm Down"));
    fArmJoyPos=fArmAngle-moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    break;

    case'4':
    Serial.println(F("Received Command: Claw Close Down"));
    clawJoyPos=clawAngle+moveStep;
    servoCmd('c',clawJoyPos,DSD);
    break;

    case'6':
    Serial.println(F("Received Command: Claw Open Up"));
    clawJoyPos=clawAngle-moveStep;
    servoCmd('c',clawJoyPos,DSD);
    break;

    case 'm':
    mode=2;
    Serial.println(F("Command: Switch to Instruction Mode."));
    break;

    case'o':
    reportStatus();
    break;

    case'i':
    armIniPos();
    break;

    default:
    Serial.println(F("Unknown Command."));
    return;
  }
}

void armDataCmd(char serialCmd){
  if(serialCmd=='w'||serialCmd=='s'||serialCmd=='a'||serialCmd=='d'||serialCmd=='5'||serialCmd=='8'||serialCmd=='4'||serialCmd=='6'){
    Serial.println(F("+Warning:Robot in Instruction Mode"));
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
      Serial.println(F("Cpmmand:Switch to Joy-Stick Mode."));
      break;

      case'0':
      reportStatus();
      break;

      case 'i':
      armIniPos();
      break;

      default:
      Serial.println(F("Unknown Command."));
      
    }
  }
}

void multiServoCmd(){
  int xTarget;
  int yTarget;
  int zTarget;

  xTarget=Serial.parseInt();
  Serial.read();
  Serial.read();
  yTarget=Serial.parseInt();
  Serial.read();
  Serial.read();
  zTarget=Serial.parseInt();

  if(xTarget<baseMin||xTarget>baseMax||
     yTarget<rArmMin||yTarget>rArmMax||
     zTarget<fArmMin||zTarget>fArmMax){
      Serial.println(F("+Warning:Servo Value Out Of Limit!"));
      return;
     }

  Serial.println("+Command:");
  Serial.print("X=");
  Serial.println(xTarget);
  Serial.print("Y=");
  Serial.println(yTarget);
  Serial.print("Z=");
  Serial.println(zTarget);

  while(baseAngle!=xTarget||
        rArmAngle!=yTarget||
        fArmAngle!=zTarget){
          if(baseAngle<xTarget){
            baseAngle++;
          }else if(baseAngle>xTarget){
            baseAngle--;
          }
          
          if(rArmAngle<yTarget){
            rArmAngle++;
          }else if(rArmAngle>yTarget){
            rArmAngle--;
          }
          
          if(fArmAngle<zTarget){
            fArmAngle++;
          }else if(fArmAngle>zTarget){
            fArmAngle--;
          }

          board.setPos(0,baseAngle);
          board.setPos(1,rArmAngle);
          board.setPos(2,fArmAngle);

          delay(DSD);
          
        }
   Serial.println(F("Multi-Servo Finished"));
}

void servoCmd(char servoName,int toPos,int servoDelay){

  Serial.println("");
  Serial.print(F("+Command:Servo "));
  Serial.print(servoName);
  Serial.print(F(" to "));
  Serial.print(toPos);
  Serial.print(F(" at servoDelay value "));
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
      Serial.println(F("+Warning:Base Servo Value Out Of Limit!"));
      return;
    }

    case 'c':
    if(toPos>=clawMin&&toPos<=clawMax){
      fromPos=clawAngle;
      channel=3;
      break;
    }else{
      Serial.println(F("+Warning:Claw Servo Value Out Of Limit!"));
      return;
    }

    case 'f':
    if(toPos>=fArmMin&&toPos<=fArmMax){
      fromPos=fArmAngle;
      channel=2;
      break;
    }else{
      Serial.println(F("+Warning:fArm Servo Value Out Of Limit!"));
      return;
    }

    case 'r':
    if(toPos>=rArmMin&&toPos<=rArmMax){
      fromPos=rArmAngle;
      channel=1;
      break;
    }else{
      Serial.println(F("+Warning:rArm Servo Value Out Of Limit!"));
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
  Serial.println(F("+Robot-Arm Status Report +"));
  Serial.print(F("Claw Position:"));Serial.println(clawAngle);
  Serial.print(F("Base Position:"));Serial.println(baseAngle);
  Serial.print(F("Rear Arm Position:"));Serial.println(rArmAngle);
  Serial.print(F("Front Arm Position:"));Serial.println(fArmAngle);
  Serial.println(F("+++++++++++++++++++++++++"));
  Serial.println("");
  
}


void armIniPos(){
  Serial.println(F("+Command: Restore Initial Position."));
  int robotIniPosArray[4][3]={
    {'b',90,DSD},
    {'r',90,DSD},
    {'f',90,DSD},
    {'c',90,DSD}
  };

  for(int i=0;i<4;i++){
    servoCmd(robotIniPosArray[i][0],robotIniPosArray[i][1],robotIniPosArray[i][2]);
    delay(300);
  }
}
/*void moveToPosition(int position[3]){
  servoCmd('b',position[0],DSD);
  servoCmd('f',position[2],DSD);
  servoCmd('r',position[1],DSD);
}

void grabAndPlaceA(){
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Start Grab And Place A");
  Serial.println("=============================");
  moveToPosition(homePosition);
  delay(500);
  servoCmd('c',25,DSD);
  delay(500);
  moveToPosition(AAbovePosition);
  delay(500);
  moveToPosition(APickPosition);
  delay(500);
  servoCmd('c',100,DSD);
  delay(3000);
  moveToPosition(AAbovePosition);
  delay(500);
  moveToPosition(APlaceAbovePosition);
  delay(500);
  moveToPosition(APlacePosition);
  servoCmd('c',25,DSD);
  delay(1000);
  moveToPosition(APlaceAbovePosition);
  delay(500);
  moveToPosition(homePosition);
  delay(500);
  servoCmd('c',100,DSD);
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Task A Finished!");
  Serial.println("=============================");
}
void grabAndPlaceB(){
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Start Grab And Place B");
  Serial.println("=============================");
  moveToPosition(homePosition);
  delay(500);
  servoCmd('c',25,DSD);
  delay(500);
  moveToPosition(BAbovePosition);
  delay(500);
  moveToPosition(BPickPosition);
  delay(500);
  servoCmd('c',100,DSD);
  delay(1000);
  moveToPosition(BAbovePosition);
  delay(500);
  moveToPosition(BPlaceAbovePosition);
  delay(500);
  moveToPosition(BPlacePosition);
  servoCmd('c',25,DSD);
  delay(1000);
  moveToPosition(BPlaceAbovePosition);
  delay(500);
  moveToPosition(homePosition);
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Task B Finished!");
  Serial.println("=============================");
}
void grabAndPlaceC(){
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Start Grab And Place C");
  Serial.println("=============================");
  moveToPosition(homePosition);
  delay(500);
  servoCmd('c',25,DSD);
  delay(500);
  moveToPosition(CAbovePosition);
  delay(500);
  moveToPosition(CPickPosition);
  delay(500);
  servoCmd('c',100,DSD);
  delay(1000);
  moveToPosition(CAbovePosition);
  delay(500);
  moveToPosition(CPlaceAbovePosition);
  delay(500);
  moveToPosition(CPlacePosition);
  servoCmd('c',25,DSD);
  delay(1000);
  moveToPosition(CPlaceAbovePosition);
  delay(500);
  moveToPosition(homePosition);
  Serial.println("");
  Serial.println("=============================");
  Serial.println("Task C Finished!");
  Serial.println("=============================");
}*/
void runAction(int action[][2],int actionSize){
  for(int i=0;i<actionSize;i++){
    servoCmd(action[i][0],action[i][1],DSD);
    delay(300);
  }
}

void grabAndPlaceA(){
  Serial.println(F("+Command:Grab and place A"));
  runAction(A_Action1,sizeof(A_Action1)/sizeof(A_Action1[0]));
  delay(3000);
  runAction(A_Action2,sizeof(A_Action2)/sizeof(A_Action2[0]));
  delay(3000);
  runAction(A_Action3,sizeof(A_Action3)/sizeof(A_Action3[0]));
  Serial.println(F("Tsak A finished!"));
}
void grabAndPlaceB(){
  Serial.println(F("+Command:Grab and place B"));
  runAction(B_Action1,sizeof(B_Action1)/sizeof(B_Action1[0]));
  delay(3000);
  runAction(B_Action2,sizeof(B_Action2)/sizeof(B_Action2[0]));
  delay(3000);
  runAction(B_Action3,sizeof(B_Action3)/sizeof(B_Action3[0]));
  Serial.println(F("Tsak B finished!"));
}
void grabAndPlaceC(){
  Serial.println(F("+Command:Grab and place C"));
  runAction(C_Action1,sizeof(C_Action1)/sizeof(C_Action1[0]));
  delay(3000);
  runAction(C_Action2,sizeof(C_Action2)/sizeof(C_Action2[0]));
  delay(3000);
  runAction(C_Action3,sizeof(C_Action3)/sizeof(C_Action3[0]));
  Serial.println(F("Tsak C finished!"));
}

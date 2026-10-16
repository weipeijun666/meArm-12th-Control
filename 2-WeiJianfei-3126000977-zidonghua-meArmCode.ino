#include<Wire.h>
#include<LU9685.h>
#include<SoftwareSerial.h>
void drawTeachingPolyline();
bool drawTeachingSmoothCurve();
LU9685 board;

SoftwareSerial buttonSerial(6,7);//参数循序：RX，TX
int nextObject=0;
int joy1x=A0;
int joy1y=A1;
int joy2x=A2;
int joy2y=A3;

const int baseMin=0;
const int baseMax=180;
const int rArmMin=60;
const int rArmMax=150;
const int fArmMin=15;
const int fArmMax=105;
const int clawMin=50;
const int clawMax=150;

const float L1=85.0;
const float L2=85.0;
const float L3=25.0;
const float L4=36.0;
const float L5=58.0;
const float H=71.0;
const int line_steps=20;
const unsigned long record_interval=90;
const int max_records=140;

byte recordBase[max_records];
byte recordRear[max_records];
byte recordFront[max_records];
byte recordClaw[max_records];

int recordCount=0;
bool isRecording=false;
bool hasRecording=false;
bool isPlaying=false;
unsigned long lastRecordTime=0;

bool isDrawing=false;
bool isPaused=false;
const int teach_point_count=5;
float teachX[teach_point_count];
float teachY[teach_point_count];
float teachZ[teach_point_count];
int teachPointCount=0;

//动作流
//-------------------------------------------------------
//B动作一：回到初始位置，打开夹爪，移动到物体B上方，下降并移动到能直接夹取的位置,夹取。
int B_Action1[][2]={
  {'c',50},{'b',90},{'r',90},{'f',90},{'f',57},{'r',120},{'c',130}
};
//B动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int B_Action2[][2]={
  {'f',90},{'b',70},{'f',50},{'c',50}
};
//B动作三：大臂小臂依次抬起，并回到初始位置。
int B_Action3[][2]={
  {'r',78},{'b',90},{'r',90},{'f',90}
};
//A动作一：回到初始位置，打开夹爪，移动到物体A上方，下降并移动到能直接夹取的位置,夹取。
int A_Action1[][2]={
  {'c',50},{'b',90},{'r',90},{'f',90},{'b',78},{'f',57},{'r',150},{'c',150}
};
//A动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int A_Action2[][2]={
  {'r',90},{'b',60},{'r',147},{'c',50}
};
//A动作三：大臂小臂依次抬起，并回到初始位置。
int A_Action3[][2]={
  {'r',78},{'b',90},{'r',90},{'f',90}
};
//C动作一：回到初始位置，打开夹爪，移动到物体A上方，下降并移动到能直接夹取的位置,夹取。
int C_Action1[][2]={
  {'c',50},{'b',90},{'r',90},{'f',45},{'r',147},{'c',140}
};
//C动作二：向上抬起，移动到放置位置上方，下降并移动到能直接放置的位置，放置。
int C_Action2[][2]={
  {'r',90},{'b',50},{'r',50},{'r',147},{'c',50}
};
//C动作三：大臂小臂依次抬起，并回到初始位置。
int C_Action3[][2]={
  {'r',78},{'b',90},{'r',90},{'f',90}
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
int clawAngle=50;

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
  board.setPos(3,50);
  delay(10);

  Serial.begin(9600);
  buttonSerial.begin(9600);
  Serial.println(F("Welcome to My Robot Arm product"));
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(mode==0&&!isPlaying){servoJoyCmd();}
  recordAction();
  if(!isDrawing&&buttonSerial.available()>0){
    char buttonCmd=buttonSerial.read();
    Serial.print(F("Received button command:"));
    Serial.println(buttonCmd);
    switch(buttonCmd){
      case'1':
      cyclePick();
      break;
      case'2':
      toggleRecording();
      break;
      case'3':
      playRecording();
      break;
      case'4':
      armIniPos();
      break;
      case'8':
      recordTeachingPoint();
      break;
      
    }
  }
  if(Serial.available()>0){
    char serialCmd=Serial.read();
    if(serialCmd=='\n'||
    serialCmd=='\r'){
      return;
    }
    switch(serialCmd){
      case 'O':
      Serial.println(F("+Command : Claw Open!"));
      servoCmd('c',50,DSD);
      break;

      case'S':
      Serial.println(F("+Command : Claw Shut!"));
      servoCmd('c',114,DSD);
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

      case'T':
      moveToXYZ(0,150,110);
      break;

      case'l':
      isDrawing=true;
      isPaused=false;
      drawLineXYZ(0,150,0,60,150,0);
      isDrawing=false;
      isPaused=false;
      break;

      case'G':
      isDrawing=true;
      isPaused=false;
      drawRightTriangleXYZ(0,150,0,30,150,0,0,180,110);
      isDrawing=false;
      isPaused=false;
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
    Serial.println(F("Received Command: Rear Arm Up"));
    rArmJoyPos=rArmAngle-moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m1y<256){
    Serial.println(F("Received Command: Rear Arm Down"));
    rArmJoyPos=rArmAngle+moveStep;
    servoCmd('r',rArmJoyPos,DSD);
    }

    if(m2y<256){
    Serial.println(F("Received Command: Front Arm Down"));
    fArmJoyPos=fArmAngle-moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2y>767){
    Serial.println(F("Received Command: Front Arm Down"));
    fArmJoyPos=fArmAngle+moveStep;
    servoCmd('f',fArmJoyPos,DSD);
    }

    if(m2x<256){
    Serial.println(F("Received Command: Claw Close Up"));
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
      servoCmd('c',50,DSD);
      break;

      case'S':
      Serial.println(F("+Command : Claw Shut!"));
      servoCmd('c',114,DSD);
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
  
      case'0':
      reportStatus();
      break;
  
      case'i':
      armIniPos();
      break;

      case'x':
      multiServoCmd();
      break;

      case'T':
      moveToXYZ(0,150,110);
      break;

      case'l':
      isDrawing=true;
      isPaused=false;
      drawLineXYZ(0,210,20,0,150,20);
      isDrawing=false;
      isPaused=false;
      break;

      case'G':
      isDrawing=true;
      isPaused=false;
      drawRightTriangleXYZ(0,150,20,60,150,20,0,210,20);
      isDrawing=false;
      isPaused=false;
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

    case'0':
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
    {'c',50,DSD}
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
  delay(5000);
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
void cyclePick(){
  if(nextObject==0){
    grabAndPlaceA();
  }
  else if(nextObject==1){
    grabAndPlaceB();
  }
  else if(nextObject==2){
    grabAndPlaceC();
  }
  nextObject=(nextObject+1)%3;
}
void recordAction(){
  if(!isRecording){
    return;
  }
  unsigned long currentTime=millis();
  if (currentTime-lastRecordTime>=record_interval){
    if(recordCount<max_records){
      recordBase[recordCount]=baseAngle;
      recordRear[recordCount]=rArmAngle;
      recordFront[recordCount]=fArmAngle;
      recordClaw[recordCount]=clawAngle;
      recordCount++;
      lastRecordTime=currentTime;
    }
    else{
      isRecording=false;
      hasRecording=(recordCount>0);
      Serial.println(F("Recording full!"));
      Serial.print(F("Total records:"));
      Serial.println(recordCount);
    }
  }
}

void toggleRecording(){
  if(isPlaying){
    Serial.println(F("Playing,cannot record!"));
    return;
    }
  if(!isRecording){
    recordCount=0;
    lastRecordTime=millis();
    isRecording=true;
    hasRecording=false;
    Serial.println(F("Recording started!"));
  }
  else{
    isRecording=false;
    hasRecording=(recordCount>0);
    Serial.println(F("Recording stopped!"));
    Serial.print(F("Total records:"));
    Serial.println(recordCount);
  }
}

void playRecording(){
  if(isRecording||isPlaying){
    Serial.println(F("Busy,cannot play!"));
    return;
  }
  if(!hasRecording){
    Serial.println("No recording available!");
    return;
  }
  isPlaying=true;
  Serial.println(F("Playback started!"));
  for(int i=0;i<recordCount;i++){
    unsigned long frameStartTime=millis();
    moveToRecordedPosition(
      recordBase[i],
      recordRear[i],
      recordFront[i],
      recordClaw[i]
      );
    unsigned long elapsedTime=millis()-frameStartTime;
    if(elapsedTime<record_interval){
      delay(record_interval-elapsedTime);
    }
  }
  isPlaying=false;
  Serial.println(F("Playback finished!"));
}

bool moveToRecordedPosition(int targetBase,int targetRear,int targetFront,int targetClaw){
  if(targetBase<baseMin||targetBase>baseMax||targetRear>rArmMax||targetRear<rArmMin||targetFront>fArmMax||targetFront<fArmMin||targetClaw<clawMin||targetClaw>clawMax){
    Serial.println(F("Error:Target angle out of range!"));
    return false;
  }
  while(baseAngle!=targetBase||rArmAngle!=targetRear||fArmAngle!=targetFront||clawAngle!=targetClaw){
    if(isDrawing&&!checkDrawingControl()){return false;}
    if(baseAngle<targetBase){
      baseAngle++;
    }
    else if(baseAngle>targetBase){
      baseAngle--;
    }
    if(rArmAngle<targetRear){
      rArmAngle++;
    }
    else if(rArmAngle>targetRear){
      rArmAngle--;
    }
    if(fArmAngle<targetFront){
      fArmAngle++;
    }
    else if(fArmAngle>targetFront){
      fArmAngle--;
    }
    if(clawAngle<targetClaw){
      clawAngle++;
    }
    else if(clawAngle>targetClaw){
      clawAngle--;
    }
    board.setPos(0,baseAngle);
    board.setPos(1,rArmAngle);
    board.setPos(2,fArmAngle);
    board.setPos(3,clawAngle);
    delay(DSD);
  }
  return true;
}
bool inverseKinematics(float x,float y,float z,float &theta0,float &theta1,float &theta2){
  float r=sqrt(x*x+y*y);
  float t0=atan2(x,y);
  float X=r-L5-L3;
  float Y=z+L4-H;
  float EA=sqrt(X*X+Y*Y);
  if(EA>L1+L2||EA<fabs(L1-L2)){
    return false;
  }
  float cosGamma=(L2*L2+EA*EA-L1*L1)/(2*L2*EA);
  float cosAlpha=(L1*L1+EA*EA-L2*L2)/(2*L1*EA);
  if(cosGamma>1)cosGamma=1;
  if(cosGamma<-1)cosGamma=-1;
  if(cosAlpha>1)cosAlpha=1;
  if(cosAlpha<-1)cosAlpha=-1;
  float gamma=acos(cosGamma);
  float alpha=acos(cosAlpha);
  float beta=atan2(Y,X);
  float t1=gamma-beta;
  float t2=alpha+beta;
  theta0=t0*180/PI;
  theta1=t1*180/PI;
  theta2=t2*180/PI;
  return true;
}

void geometryToServo(float theta0,float theta1,float theta2,int &servoBase,int &servoRear,int &servoFront){
  servoBase=round(90-theta0);
  servoRear=round(172-theta2);
  servoFront=round(110-theta1);
}

bool moveToXYZ(float x,float y,float z){
  float theta0;
  float theta1;
  float theta2;
  bool success=inverseKinematics(x,y,z,theta0,theta1,theta2);
  if(!success){
    Serial.println(F("Error:Target unreachable!"));
    return false;
  }
  int servoBase;
  int servoRear;
  int servoFront;
  geometryToServo(theta0,theta1,theta2,servoBase,servoRear,servoFront);
  if(servoBase<baseMin||servoBase>baseMax||servoRear>rArmMax||servoRear<rArmMin||servoFront>fArmMax||servoFront<fArmMin){
    Serial.println(F("Error:Servo angle out of range!"));
    return false;
  }
  Serial.println(F("IK calculation successful"));
  Serial.print(F("Base="));
  Serial.println(servoBase);
  Serial.print(F("Rear="));
  Serial.println(servoRear);
  Serial.print(F("Front="));
  Serial.println(servoFront);
  if(!moveToRecordedPosition(servoBase,servoRear,servoFront,clawAngle)){
    return false;
  }
  return true;
}
void forwardKinematics(float &x,float &y,float &z){
  float theta0=(90-baseAngle)*PI/180;
  float theta1=(110-fArmAngle)*PI/180;
  float theta2=(172-rArmAngle)*PI/180;
  float r=L3+L1*cos(theta2)+L2*cos(theta1)+L5;
  x=r*sin(theta0);
  y=r*cos(theta0);
  z=H+L1*sin(theta2)-L2*sin(theta1)-L4;
}
void recordTeachingPoint(){
  if(teachPointCount>=teach_point_count){
    Serial.println(F("Five points already recorder!"));
    return;
  }
  float x,y,z;
  forwardKinematics(x,y,z);
  teachX[teachPointCount]=x;
  teachY[teachPointCount]=y;
  teachZ[teachPointCount]=z;
  Serial.print(F("Point "));
  Serial.print(teachPointCount+1);
  Serial.print(F(":X="));
  Serial.print(x);
  Serial.print(F(",Y="));
  Serial.print(y);
  Serial.print(F(",Z="));
  Serial.print(z);
  teachPointCount++;
  Serial.print(F("Recorded points: "));
  Serial.print(teachPointCount);
  Serial.println(F("/5"));
  if(teachPointCount==teach_point_count){
    Serial.println(F("Five teaching points recorded!"));
    Serial.println(F("Enter P for straight lines."));
    Serial.println(F("Enter S for smooth curve."));
    while(true){
      if(Serial.available()>0){
        char cmd=Serial.read();
        if(cmd=='\r'||cmd=='\n'){
          continue;
        }
        if(cmd=='P'){
          drawTeachingPolyline();
          break;
        }
        else if(cmd=='S'){
          drawTeachingSmoothCurve();
          break;
        }
      }
    }
    
  }
}
bool drawLineXYZ(float x0,float y0,float z0,float x1,float y1,float z1){
  Serial.println(F("Start drawing line"));
  for(int i=0;i<=line_steps;i++){
    if(!isDrawing){
      Serial.println(F("Line drawing stopped"));
      return false;
    }
    if(!checkDrawingControl()){
      Serial.println(F("Line drawing stopped"));
      return false;
    }
    float t=(float)i/line_steps;
    float x=x0+(x1-x0)*t;
    float y=y0+(y1-y0)*t;
    float z=z0+(z1-z0)*t;
    bool success=moveToXYZ(x,y,z);
    if(!success){
      Serial.println(F("Error:Line drawing stopped"));
      return false;
    }
  }
  delay(500);
  Serial.println(F("Line drawing finished"));
  return true;
}

bool drawRightTriangleXYZ(float xA,float yA,float zA,
                          float xB,float yB,float zB,
                          float xC,float yC,float zC)
    {
      Serial.println(F("Start drawing right triangle"));
      if(!drawLineXYZ(xA,yA,zA,xB,yB,zB)){
        Serial.println(F("Error:AB failed"));
        return false;
      }
      if(!drawLineXYZ(xB,yB,zB,xC,yC,zC)){
        Serial.println(F("Error:BC failed"));
        return false;
      }
      if(!drawLineXYZ(xC,yC,zC,xA,yA,zA)){
        Serial.println(F("Error:CA failed"));
        return false;
      }
      Serial.println(F("Right triangle finished"));
      return true;
    }
void drawTeachingPolyline()
{
  if(teachPointCount!=teach_point_count){
    Serial.println(F("Error:Need five teaching points!"));
    return;
  }
  Serial.println(F("start teaching point drawing"));
  isDrawing=true;
  isPaused=false;
  bool success=true;
  success=moveToRecordedPosition(90,90,90,clawAngle);
  if(success&&isDrawing){
    success=moveToXYZ(teachX[0],teachY[0],teachZ[0]);
  }
  for(int i=0;i<teach_point_count-1;i++){
    if(!success||!isDrawing){
      success=false;
      break;
    }
    Serial.print(F("Drawing Point"));
    Serial.print(i+1);
    Serial.print(F("->Point"));
    Serial.println(i+2);
    success=drawLineXYZ(teachX[i],teachY[i],teachZ[i],teachX[i+1],teachY[i+1],teachZ[i+1]);
  }
  isDrawing=false;
  isPaused=false;
  if(success){
    Serial.println(F("Teaching point drawing finished!"));
  }else{
    Serial.println(F("Teaching point drawing stopped!"));
  }
}
  bool checkDrawingControl(){

      if(!isDrawing){
        return false;
      }
      while(buttonSerial.available()>0){
        char cmd=buttonSerial.read();
        if(cmd=='5'){
          isPaused=true;
          Serial.println(F("Drawing paused"));
        }
        else if(cmd=='6'){
          isPaused=false;
          Serial.println(F("Drawing resumed"));
        }
        else if(cmd=='7'){
          isDrawing=false;
          isPaused=false;
          Serial.println(F("Drawing cancelled"));
          return false;
        }
      }
      while(isDrawing&&isPaused){
        while(buttonSerial.available()>0){
          char cmd=buttonSerial.read();
          if(cmd=='6'){
            isPaused=false;
            Serial.println(F("Drawing resumed"));
          }
          else if(cmd=='7'){
            isDrawing=false;
            isPaused=false;
            Serial.println(F("Drawing cancelled"));
            return false;
          }
        }
        delay(10);
      }
      return isDrawing;
    }
float catmullRom(float p0,float p1,float p2,float p3,float t){
  float t2=t*t;
  float t3=t2*t;
  return 0.5*(2*p1+(-p0+p2)*t+(2*p0-5*p1+4*p2-p3)*t2+(-p0+3*p1-3*p2+p3)*t3);
}
bool drawTeachingSmoothCurve(){
  if(teachPointCount!=teach_point_count){
    Serial.println(F("Error:Need five teaching points!"));
    return false;
  }
  Serial.println(F("Start smooth curve drawing"));
  isDrawing=true;
  isPaused=false;
  bool success=true;
  success=moveToRecordedPosition(90,90,90,clawAngle);
  if(!success||!isDrawing){
    success=false;
  }
  if(success){
    Serial.println(F("Move to point 1"));
    success=moveToXYZ(teachX[0],teachY[0],teachZ[0]);
  }
  const int curve_steps=20;
  for(int i=0;i<teach_point_count-1;i++){
    if(!success||!isDrawing){
      success=false;
      break;
    }
    int i0=(i==0)?0:i-1;
    int i1=i;
    int i2=i+1;
    int i3=(i+2>=teach_point_count)?teach_point_count-1:i+2;
    for(int j=1;j<=curve_steps;j++){
      if(!isDrawing){
        success=false;
        break;
      }
      if(!checkDrawingControl()){
        success=false;
        break;
      }
      float t=(float)j/curve_steps;
      float x=catmullRom(teachX[i0],teachX[i1],teachX[i2],teachX[i3],t);
      float y=catmullRom(teachY[i0],teachY[i1],teachY[i2],teachY[i3],t);
      float z=catmullRom(teachZ[i0],teachZ[i1],teachZ[i2],teachZ[i3],t);
      success=moveToXYZ(x,y,z);
      if(!success){
        Serial.println(F("Error:Cannot reach curve point"));
        break;
      }
    }
    if(!success){break;}
  }
  bool cancelled=!isDrawing;
  isDrawing=false;
  isPaused=false;
  if(success){
    Serial.println(F("Smooth curve drawing finished!"));
  }else{
    Serial.println(F("Smooth curve drawing stopped!"));
  }
  if(cancelled){
    Serial.println(F("Returning to standby position"));
    moveToRecordedPosition(90,90,90,clawAngle);
  }
    return success;
}

#include <TimerOne.h>

// DrFocuser v0.2 Arduino code
// Work with stepper Nema motors and polulu driver
// Work with a serial console, type H for help in the console
// Made to be integrated in ASCOM driver but can work manually with putty 
// or other serial client
// Guillaume Seigneuret 2017

#define motEnable 6
#define motStep 5
#define motDir 4
#define MS1 7
#define MS2 8
#define MS3 9

#define INITFREQ (90)
#define STEP_SHIFT (20)

const int freq = 10369;
int POS = 0;
int DIR = 0; // 0 = Anticlockwise, 1 = clockwise
int MOT = 0;
int firstMSG = 0;

void setup(void)
{
  
  pinMode(motEnable,OUTPUT); // Enable
  pinMode(motStep,OUTPUT); // Step
  pinMode(motDir,OUTPUT); // Dir
  
  //MOTOR On
  digitalWrite(motEnable,LOW);
  
  // Set Dir AntiClockwise (move focuser goes up) becasue we assume position 0 with a focuser as low as possible
  setAntiClockwise();
  
  pinMode(MS1,OUTPUT); // MS1
  pinMode(MS2,OUTPUT); // MS2
  pinMode(MS3,OUTPUT); // MS3
  
  // Set microstep mode
  //setMicroStep();
  //fastSpeed();
  
  //Timer1.attachInterrupt(sideralSpeed);
  Serial.begin(115200);
  
}

void pg_version(void)
{
  Serial.println("DrFocuser v0.1");
  Serial.println("Starting now !");
}

void setClockwise(void)
{
  digitalWrite(motDir,HIGH);
  DIR = 1;
}

void setAntiClockwise(void)
{
  digitalWrite(motDir,LOW);
  DIR = 0;
}

void setMotorOff()
{
  digitalWrite(motEnable,HIGH);
  MOT = 0;
}

void setMotorOn()
{
  digitalWrite(motEnable,LOW);
  MOT = 1;
}

void setMicroStep(void)
{
  // Set microstep mode
  digitalWrite(MS1,HIGH); 
  digitalWrite(MS2,HIGH);
  digitalWrite(MS3,HIGH);
}

void setFullStep(void)
{
  // Set fullstep mode
  digitalWrite(MS1,LOW); 
  digitalWrite(MS2,LOW);
  digitalWrite(MS3,LOW);
}

int moveNstep(int steps)
{
  if(POS == 0 and DIR == 1 ){
    // We already are at the min position
    return 0;
  }
  if(POS-steps < 0 and DIR == 1) {
    steps = POS;
  }
  
  Timer1.detachInterrupt();
  //enable motor
  setMotorOn();
  setFullStep();
  int stepPerTour = 400;
  int iter = 0;
  //digitalWrite(motStep,HIGH); // Impulse start   
  //digitalWrite(motStep,LOW);
  while(iter < steps){
    digitalWrite(motStep,HIGH); 
    digitalWrite(motStep,LOW);
    iter += 1;
    delay(1); 
  }
  if (DIR == 0){
    POS += steps;
  }
  else {
    POS -= steps;
  }
}

void loop(void)
{
  int incomingByte;
  int asked_pos = 0;
  String command = "";
  
  if (Serial.available() > 0) {
                // read the incoming byte:
                //incomingByte = Serial.read();
                //long int number_rec = Serial.parseInt();
                //incomingByte = Serial.read();
                command = Serial.readString();
                /*if (asked_pos = command.toInt()){
                  Serial.print("command conveted to number : ");
                  Serial.println(command);
                }
                else {
                  Serial.print("I received: ");
                  //Serial.println(number_rec);
                  //Serial.print("Also: ");
                  Serial.println(command);
                }*/
                
                if(firstMSG == 0) {
                  pg_version();
                  firstMSG = 1;
                }
                 // say what you got:
                // 48 => 0; 48 => 1; 49 => 2; 50 => 3; 
                
                
                if(command == "L"){
                  Serial.print("Motor On\n");
                  setMotorOn();
                }
                if(command == "C"){
                  //clockwise
                  Serial.print("Clockwise\n");
                  setClockwise();
                }
                if(command == "A"){
                  //anti clockwise
                  Serial.print("Anti-Clockwise\n");
                  setAntiClockwise();
                }
                if(command == "U"){
                  //stop motor
                  Serial.print("Motor Off\n");
                  setMotorOff();
                }
                if(command == "P"){
                  //Serial.print("Current position : ");
                  Serial.println(POS);
                }
                if(asked_pos = command.toInt()){
                  //Serial.print("Move 1 tour\n");
                  moveNstep(asked_pos);
                  //Serial.print("Position : ");
                  Serial.println(POS);
                }

                if(command == "Z"){
                  //Serial.print("Goto position 0\n");
                  setClockwise();
                  moveNstep(POS);
                  setAntiClockwise();
                  //Serial.print("Position : ");
                  //Serial.println(POS);
                }
                if(command == "H"){
                  Serial.print("L: Lock mode\n");
                  Serial.print("C: Change direction to clockwise\n");
                  Serial.print("A: Change direction to anti-clockwise\n");
                  Serial.print("U: Disagage motor\n");
                  Serial.print("P: Get the current position\n");
                  Serial.print("H: This Help\n");
                  Serial.print("Z: Go back to position 0\n");
                  Serial.print("number: Move N steps\n");
                  Serial.print("Clockwise: decrease position, Anti-clockwise: Increase position\n");
                }
                delay(1);
                
  }

  delay(1);
}

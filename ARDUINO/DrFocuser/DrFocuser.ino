#include <EEPROM.h>
#include <TimerOne.h>

/*DrFocuser v0.8 Arduino code
   Work with stepper Nema motors and polulu driver
   Work with a serial console, type H for help in the console
   Made to be integrated in ASCOM driver but can work manually with putty 
   or other serial client
   Guillaume Seigneuret 2017*/
/*    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#define motEnable 6
#define motStep 5
#define motDir 4
#define MS1 7
#define MS2 8
#define MS3 9
#define ClockWise 0
#define AntiClockWise 1

int POS = 0; // Position of the focuser
int DIR = 0; // 0 = Anticlockwise, 1 = clockwise
int MOT = 0; // 0 = Off, 1 = On
int TARGET = 0; 
int firstMSG = 0;
int motorFreq = 500;

void setup(void)
{
  //Timer1.initialize(INITFREQ);
  pinMode(motEnable,OUTPUT); // Enable
  pinMode(motStep,OUTPUT); // Step
  pinMode(motDir,OUTPUT); // Dir
  
  //MOTOR Off
  setMotorOff();
  //digitalWrite(motEnable,LOW);
  
  // Set Dir AntiClockwise (move focuser goes up) becasue we assume position 0 with a focuser as low as possible
  setAntiClockwise();
  
  pinMode(MS1,OUTPUT); // MS1
  pinMode(MS2,OUTPUT); // MS2
  pinMode(MS3,OUTPUT); // MS3
  
  // Set microstep mode
  //setMicroStep();
  
  Serial.begin(115200);
  
}

void pg_version(void)
{
  Serial.println("DrFocuser v0.92");
  //Serial.println("Starting now !");
}

void setClockwise(void)
{
  digitalWrite(motDir,HIGH);
  DIR = 0;
}

void setAntiClockwise(void)
{
  digitalWrite(motDir,LOW);
  DIR = 1;
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

void impulse()
{
  digitalWrite(motStep,HIGH); 
  digitalWrite(motStep,LOW);
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
  /*digitalWrite(MS1,LOW); 
  digitalWrite(MS2,LOW);
  digitalWrite(MS3,LOW);
  */
  //Half step
  digitalWrite(MS1,HIGH); 
  digitalWrite(MS2,LOW);
  digitalWrite(MS3,LOW);
}

// Function with timer
void justMove(){
   
  impulse();
  if (DIR == 1){
    POS += 1;
  }
  else {
    POS -= 1;
  }
  if(TARGET == POS or POS == 0)
  {
    Timer1.detachInterrupt();
    Serial.println(String(POS)+"#");
    setMotorOff();
  }
  
}

int moveNstepNC(int steps){
  //enable motor if its off and wait 1ms for it to be fully ready
  if (MOT == 0){
    setMotorOn();
    setFullStep();
    delay(1);
  }
  
  int iter = 0;
  
  while(iter < steps){
    impulse();
    iter += 1;
    delayMicroseconds(motorFreq); 
  }
  setMotorOff();
}

// Function without timer
int moveNstep(int steps)
{
  if(POS == 0 and DIR == 0 ){
    // We already are at the min position
    return 0;
  }
  if(POS-steps < 0 and DIR == 0) {
    // The asked position is below 0 position, so move to 0
    steps = POS;
  }
  
  //enable motor if its off and wait 1ms for it to be fully ready
  if (MOT == 0){
    setMotorOn();
    setFullStep();
    delay(1);
  }
  
  int iter = 0;
  
  while(iter < steps){
    impulse();
    iter += 1;
    delayMicroseconds(motorFreq); 
  }
  if (DIR == 1){
    POS += steps;
  }
  else {
    POS -= steps;
  }
  //Serial.println(String(POS)+"#");
  setMotorOff();
}

void Zero()
{
  POS = 0;
}

void loop(void)
{
  int asked_pos = 0;
  String command = "";
  int steps = 0;
  
  if (Serial.available() > 0) {
   
                // Read incoming message and convert it to string
                command = Serial.readStringUntil('#');
                
                // First time we receive a command, we send the welcome message
                /*if(firstMSG == 0) {
                  pg_version();
                  firstMSG = 1;
                };*/
                
                // L command received, set motor on
                if(command == "L"){
                  //Serial.println("Motor On#");
                  setMotorOn();
                }
                // C command, Set clockwise rotation
                if(command == "C"){
                  //clockwise
                  //Serial.println("Clockwise#");
                  setClockwise();
                  setMotorOff();
                }
                // A command, set Anticlockwise rotation
                if(command == "A"){
                  //anti clockwise
                  //Serial.println("Anti-Clockwise#");
                  setAntiClockwise();
                  setMotorOff();
                }
                // U command, set motor off, usefull for manual focus
                if(command == "U"){
                  //stop motor
                  //Serial.println("Motor Off#");
                  setMotorOff();
                }
                // S command for an emergency stop
                if(command == "S")
                {
                  Timer1.detachInterrupt();
                  setMotorOff();
                  //Serial.println("Emergency Stop#");
                }
                //P command, return the current position
                if(command == "P"){
                  //Serial.print("Current position : ");
                  Serial.println(String(POS)+"#");
                }
                // If the command is a number, send a rotation of N steps
                if(asked_pos = command.toInt()){
                  //moveNstep(asked_pos);
                  TARGET = asked_pos;
                  if (POS > TARGET and TARGET > 0){
                    setClockwise();
                    setMotorOff();
                  }
                  if (POS < TARGET){
                    setAntiClockwise();
                    setMotorOff();
                  }
                  steps = abs(POS - TARGET);
                  if (POS != TARGET and TARGET >= 0)
                  {
                    // If few steps, use synchrone move
                    // It's much more stable for small moves
                    if (steps <= 100) {
                      moveNstep(steps);
                      Serial.println(String(POS)+"#");
                      setMotorOff();
                      return;
                    }
                    // Else use asynchrone move which allows emergency stops
                    else {
                      //enable motor if its off and wait 1ms for it to be fully ready
                      if (MOT == 0){
                        setMotorOn();
                        setFullStep();
                        delay(1);
                      }
                      Timer1.initialize(motorFreq);
                      Timer1.attachInterrupt(justMove);
                      return;
                    }
                  }
                  if (TARGET < 0) {
                      setClockwise();
                      moveNstepNC(abs(TARGET));
                      Zero();
                      Serial.println(String(POS)+"#");
                      return;
                  }
                  if (TARGET == POS) {
                    Serial.println("not moving");
                    return;
                  }
                }

                // Z command, go back to position zero
                // Dunno why if 0 is entered, it's not recognizes as a number oO
                if(command == "Z" or command == "0"){
                  //Serial.print("Goto position 0\n");
                  setClockwise();
                  moveNstep(POS);
                  setClockwise();
                  Serial.println(String(POS)+"#");
                  setMotorOff();
                }
                if(command == "T"){
                  //Serial.println("OK#");
                }
                if(command == "R"){
                  Zero();
                }
                if(command == "M"){
                  Serial.println("Entering Manual mode#");
                  int foffset = 0;
                  foffset = Serial.read();
                  Serial.println(String(foffset)+"#");
                }
                if(command == "H"){
                  Serial.println("L: Lock mode");
                  Serial.println("C: Change direction to clockwise");
                  Serial.println("A: Change direction to anti-clockwise");
                  Serial.println("U: Disengage motor");
                  Serial.println("S: Emergency stop");
                  Serial.println("P: Get the current position");
                  Serial.println("H: This Help");
                  Serial.println("Z: Go back to position 0");
                  Serial.println("R: Reset offset to 0, no moves");
                  Serial.println("positive number: Move to offset N");
                  Serial.println("negative number: Move backwards of N steps, reset offset to 0");
                  Serial.println("Clockwise: decrease position, Anti-clockwise: Increase position#");
                }
                if(command == "V") {
                  pg_version();
                };
                
  }
  //delay(10);
}

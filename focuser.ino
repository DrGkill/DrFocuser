#include <TimerOne.h>

// 20 bits after the floating point, has to be ajusted regarding the speed
// Eg: For a freq of 10kHz, 24 is a good value,
// For a freq of 1.7 kHz, 20 seams accurate

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
  //Timer1.initialize(INITFREQ); // Call interupts each 90µs
  
  // 1 full tr in 86164 seconds witch is demultiplied by 6^7
  // it requires the motor to do 3.248874 tr/s
  // 3.24887423982 tr/s x 200 steps x 16 microsteps = 10369.3975674 impulses/s 
  // Impules @ 10.369 kHz, so wait 90µs
  
  // 1 full tr in 86164 seconds witch is demultiplied by 6^6
  // it requires the motor to do 0.54147903997 tr/s
  // Impules @ 1732.7329279 Hz, so wait 577.1236550497256 µs
  
  // Demultiplication with 6^4 plus 10 time demult
  // 1/((((6^4)*10)/86164.10)*200*16)*1000000
  // 2077.6451581790129239 µs
  
  pinMode(motEnable,OUTPUT); // Enable
  pinMode(motStep,OUTPUT); // Step
  pinMode(motDir,OUTPUT); // Dir
  
  //digitalWrite(motEnable,LOW); // Set Enable low
  //MOTOR Off
  digitalWrite(motEnable,HIGH);
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

// Keep a track of the accumulated delay
//uint32_t StepCons=(577.1236550497257)*pow(2,STEP_SHIFT);
uint32_t StepCons=(2077.64515817901292394)*pow(2,STEP_SHIFT);
uint32_t lastStepErr = 0;
uint32_t StepCurr = StepCons;

void sideralSpeed(void)
{ 
  digitalWrite(motStep,HIGH); // Impulse start   
  digitalWrite(motStep,LOW); // Impulse stop. 
  
  StepCurr = StepCons + lastStepErr;
  Timer1.setPeriod(StepCurr>>STEP_SHIFT);
  //Serial.println(StepCurr>>STEP_SHIFT);
  lastStepErr = StepCurr & (((uint32_t)~0)>>(32 - STEP_SHIFT) ); // On prend juste le poid faible
}

  //0 = Clockwise , 1 = Anti-clockwise
void fastSpeed(int speedo)
{
  digitalWrite(motEnable,LOW);
  digitalWrite(motStep,HIGH); // Impulse start   
  digitalWrite(motStep,LOW); // Impulse stop.
  
  // Set the initial speed @600 RPM
  if (StepCurr > speedo) {
    StepCurr = speedo;
  }
  // Speed = number_of_step / time_between_steps so function like 1/x
  // to increase speed linearly: multiply by (constant x time_between_steps²)
  
  // Linear acceleration to 2340 RPM
  if (StepCurr > speedo) {
    //10.000.000 give an acceleration to cruise speed in 0.13s
    StepCurr -= 1/10000000*pow(StepCurr,2);
    Timer1.setPeriod(StepCurr);
  }
}

int moveNstep(int steps)
{
  if(POS == 0 and DIR == 1 ){
    // We already are at the min position
    return 0;
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
    delay(3); 
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
  int incomingByte = 0;
  
  if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();
                if(firstMSG == 0) {
                  pg_version();
                  firstMSG = 1;
                }
                 // say what you got:
                // 48 => 0; 48 => 1; 49 => 2; 50 => 3; 
                //Serial.print("I received: ");
                //Serial.println(incomingByte, DEC);
                if(incomingByte == 48){
                  //Serial.print("Lock On\n");
                  setMotorOn();
                }
                if(incomingByte == 49){
                  //clockwise
                  //Serial.print("Clockwise direction\n");
                  setClockwise();
                }
                if(incomingByte == 50){
                  //anti clockwise
                  //Serial.print("Anti-Clockwise direction\n");
                  setAntiClockwise();
                }
                if(incomingByte == 51){
                  //stop motor
                  //Serial.print("Motor Stopped\n");
                  setMotorOff();
                }
                if(incomingByte == 52){
                  Serial.print("Current position : ");
                  Serial.println(POS);
                }
                if(incomingByte == 53){
                  //Serial.print("Move 1 tour\n");
                  moveNstep(400);
                  //Serial.print("Position : ");
                  //Serial.println(POS);
                }
                if(incomingByte == 54){
                  //Serial.print("Move 1 step\n");
                  moveNstep(1);
                  //Serial.print("Position : ");
                  //Serial.println(POS);
                }
                if(incomingByte == 55){
                  //Serial.print("Goto position 0\n");
                  setClockwise();
                  moveNstep(POS);
                  setAntiClockwise();
                  //Serial.print("Position : ");
                  //Serial.println(POS);
                }
                if(incomingByte == 57){
                  Serial.print("0: Lock mode\n");
                  Serial.print("1: Change direction to clockwise\n");
                  Serial.print("2: Change direction to anti-clockwise\n");
                  Serial.print("3: Disagage motor\n");
                  Serial.print("4: Get the current position\n");
                  Serial.print("5: Move one tour (400 step)\n");
                  Serial.print("6: Move one step\n");
                  Serial.print("7: Go back to position 0\n");
                  Serial.print("9: This Help\n");
                  Serial.print("Clockwise: decrease position, Anti-clockwise: Increase position\n");
                }
                
  }
  // Each n time, reset the timer to correct periodic error
  //Timer1.setPeriod(microseconds);
  
  /* Début de mode acceleré
  if(){
    setFullStep();
    Timer1.attachInterrupt(fastSpeed);
  }
  
  if() {
    setMicroStep();
    Timer1.attachInterrupt(sideralSpeed);
  }
  */
  
  //noInterrupts();
  //interrupts();
  //Serial.print("Last step error : ");
  //Serial.println(lastStepErr);
  //Serial.print("Current Step : ");
  //Serial.println(StepCurr>>STEP_SHIFT);
  delay(1);
}

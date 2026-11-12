#include <Arduino.h>

#define VERSION           "0.1"

//
//  Board: Ardunio Uno
//  DEFINE ALL I/O PIN CONNECTIONS
//    *** DO NOT CHANGE ***
//
#define PIN_MTR1_ENCA          2
#define PIN_MTR2_ENCA          3
#define PIN_PB_START           4
#define PIN_MTR1_DIR_FWD       5
#define PIN_MTR1_DIR_REV       6
#define PIN_MTR2_DIR_FWD       7
#define PIN_MTR2_DIR_REV       8
#define PIN_MTR1_PWM           9
#define PIN_MTR2_PWM          10
#define PIN_LED               13


//************************* ADJUST THE FOLLOWING TO MATCH YOUR ROBOT ****************************

#define ENCODER_COUNTS_PER_REV  540   // Set to the number of encoder pulses per wheel revolution
#define MM_PER_REV              184    // Set to the number of mm per wheel revolution (Hence : Diameter * Pi)
#define ENCODER_COUNTS_90_DEG   425    // Set to the number of encoder pulses to make a 90 degree turn
#define SPEED_MIN               120    // Minimum speed (pulses/second) use at the end of individual moves
#define SYNC_GAIN                 2    // PWM correction per encoder count of left/right mismatch
#define SYNC_MAX                 40    // Largest PWM correction the sync loop may apply



unsigned long usLast;
long usecElapsed;
long usScanLong;
int  usLongResetCount;
long usScanAvg;
long timerusScan;
int scanCount;

long timerRunTime;
int speedFwd;
int speedTurn;
int flagTimeRun;
int flagLastMoveFwd;

int flagLED;


int printStep;
int printLastCmd;
unsigned long msTimerPrint;

unsigned long msTimerMPU;

unsigned long timerPBStartOn;
unsigned long timerPBStartOff;

unsigned long timerDelay;

#define MAX_COMMANDS    60        // Maximum number of motion commands allowed

//======================================================================================
// Command object is used to hold a list of commands to be executed
//======================================================================================
class Command
{
  private:
    int start;
    int end;
    int last_cmd;
    int list[MAX_COMMANDS];
    int p1[MAX_COMMANDS];
    int flagFirstScan;

  public:
    inline int   current()  { return list[start]; };
    inline int   getParameter1() { return p1[start]; };
    inline int   empty()    { if (start==end) return 127; else return 0; };
    inline int   last()     { return last_cmd; };
    inline int   firstScan() { int flag = flagFirstScan; flagFirstScan = 0; return flag; } 
  
    Command() {
      clear();
    }
  
    void clear() {
      start = 0;
      end = 0;
      last_cmd = 0;
      flagFirstScan = 0;
      int n = 0;
      for (n=0;n<MAX_COMMANDS;n++) { list[n] = 0;  p1[n] = 0; }
    }

    void add(int cmd) {
      add(cmd,0);
    }

    void add(int cmd,int par1) {
      list[end] = cmd;
      p1[end] = par1;
      end++;
      if (end >= MAX_COMMANDS) end = 0;
    }
  
    int next() {
      flagFirstScan = 127;
      if (empty()) return 0;
      last_cmd = list[start];   // save last command
      start++;
      if (start >= MAX_COMMANDS) start = 0;
      return list[start];
    }

};

Command cmdQueue;

//
//  List of possible vehicle motion commands
//   -- Additional motion commands can be added which will require code to execute
//
#define VEHICLE_START_WAIT      1       // Wait for the start button to be pressed
#define VEHICLE_START           2       // First motion command after button press
#define VEHICLE_FORWARD         10
#define VEHICLE_TURN_RIGHT      40
#define VEHICLE_TURN_LEFT       50
#define VEHICLE_SET_MOVE_SPEED  101
#define VEHICLE_SET_TURN_SPEED  102
#define VEHICLE_SET_ACCEL       105
#define VEHICLE_FINISHED        900     // Must be at the end of the command list
#define VEHICLE_STOP            1000
#define VEHICLE_ABORT           2000    // Used to abort the current movement list and stop the robot

//======================================================================================
//======================================================================================
// Loads the command queue with the robots commands to be executed during a run
//======================================================================================
//======================================================================================
void loadCommandQueue() {

  cmdQueue.clear();
  cmdQueue.add(VEHICLE_START_WAIT);     // do not change this line - waits for start pushbutton
  cmdQueue.add(VEHICLE_START);          // do not change this line

  // Define robot movement speeds
  // Speed is encoder pulses per second.
  // There is a maximum speed.  Testing will be required to learn this speed.
  //    SETTING THE SPEEDS ABOVE THE MOTOR'S MAXIMUM SPEED WILL CAUSE STRANGE RESULTS
      cmdQueue.add(VEHICLE_SET_MOVE_SPEED,500);     // Speed used for forward movements  
      cmdQueue.add(VEHICLE_SET_TURN_SPEED,300);     // Speed used for left or right turns
      cmdQueue.add(VEHICLE_SET_ACCEL,400);         // smaller is softer   larger is quicker and less accurate moves

        // Example list of robot movements
        // This block is modified for each tournament
      cmdQueue.add(VEHICLE_FORWARD,250);
      cmdQueue.add(VEHICLE_TURN_LEFT);
      cmdQueue.add(VEHICLE_FORWARD,250);
      cmdQueue.add(VEHICLE_TURN_LEFT);
      cmdQueue.add(VEHICLE_FORWARD,250);
      cmdQueue.add(VEHICLE_TURN_LEFT);
      cmdQueue.add(VEHICLE_FORWARD,250);
      cmdQueue.add(VEHICLE_TURN_LEFT);
        

      // cmdQueue.add(VEHICLE_FORWARD,830); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_RIGHT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,1000); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,1000); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,1500); 
      // cmdQueue.add(VEHICLE_TURN_RIGHT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_RIGHT);
      // cmdQueue.add(VEHICLE_TURN_RIGHT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_LEFT);
      // cmdQueue.add(VEHICLE_FORWARD,500); 
      // cmdQueue.add(VEHICLE_TURN_RIGHT);
      // cmdQueue.add(VEHICLE_FORWARD,420); 


          // This MUST be the last command.  
      cmdQueue.add(VEHICLE_FINISHED);

}

//======================================================================================
// Motion object (like a library) that calculates the acceleration used for motor speed
// control.
// 
// Distance is encoder pulses
// Speed is encoder pulses per second
// Accel is encoder pulses per second^2
//======================================================================================
class MotionLogic
{
  private:
    long timeAccel;
    long timeAtSpeed;
    long timeDecel;

    long timeRunning;
    long timeRunningLast;
    long timerUpdate;
    
    int pinPWM;
    int outputPWM;
    int outputFwd;
    int outputRev;

    int running;
    int flagStopped;
    int counterStopped;

    int pwmLoopI;
    int pwmLoopP;
    int pwmTrim;       // correction from the wheel-sync loop (keeps both wheels matched)

    int debugPrint;

  public:
    long accelRate;
    long decelRate;
    long position;
    long posProfile;

    int speedTarget;
    int speedProfile;
    int speedActual;
    int speedMinimum;

    int speedAtDecel;

    volatile long countEncoder;   // incremented in the encoder interrupt
    long countEncoderLast;

    inline int getOutputPWM() { return outputPWM; };
    inline int getOutputFwd() { return outputFwd; };
    inline int getOutputRev() { return outputRev; };

    inline void incrEncoder() { countEncoder++; };

    // Read the 32-bit count atomically. The interrupt can fire mid-read on an 8-bit AVR,
    // which would otherwise return a corrupted value.
    long getCount() {
      noInterrupts();
      long c = countEncoder;
      interrupts();
      return c;
    }

    inline void setTrim(int trim) { pwmTrim = trim; };

    inline void debugOn() { debugPrint = 1; };   // used this function to turn on debug print statements
                                                 // recommended to only turn on debug for left or right motor.  NOT both.
    inline int debugState() { return debugPrint; };

    MotionLogic() {
      outputPWM = 0;
      outputFwd = 0;
      outputRev = 0;

      countEncoder = 0;
      countEncoderLast = 0;
      pwmLoopI = 0;
      pwmLoopP = 0;
      pwmTrim = 0;

      debugPrint = 0;

      accelRate = 200;
      decelRate = 200;

      posProfile = 0;

      speedActual = 0;
      speedTarget = 0;
      speedAtDecel = -10000;
      timeRunning = 0;
      timeRunningLast = 0;
      flagStopped = 0;
      counterStopped = 0;
      running = 0;
    }

    int isStopped() {
      if (flagStopped) return 1;
      return 0;
    }

    void setParams(long accel,int spdMin,int pPWM) {
      accelRate = accel;
      decelRate = accel;
      speedMinimum = spdMin;
      pinPWM = pPWM;
    }

    // This object uses the same value for accelerate and decelerate rate
    void setAccel(long accel) {
      accelRate = accel;
      decelRate = accel;
    }

    // This function must be called to start a motion
    void startMove(int pos, int spd) {

      if (spd > 0) {
        outputFwd = 1;
        outputRev = 0;
      } else {
        outputFwd = 0;
        outputRev = 1;
      }

      speedTarget = abs(spd);
      position = abs(pos);
      if (debugPrint) { Serial.print(F("Motion - speed       = ")); Serial.println(speedTarget); }
      if (debugPrint) { Serial.print(F("Motion - position    = ")); Serial.println(position); }

      float tAccel = (float) speedTarget / (float) accelRate;
      float tDecel = (float) speedTarget / (float) decelRate;
      float distAccel = (float) speedTarget / 2.0 * tAccel;
      float distDecel = (float) speedTarget / 2.0 * tDecel;
      float distAtSpeed = (float) position - distAccel - distDecel;
      if (distAtSpeed < 0.0) {  // current written as accel and decel same
        // Serial.println("Motion - Short move logic");
        distAccel = (float) position / 2.0;
        distDecel = (float) position / 2.0;
        distAtSpeed = 0.0;

        tAccel = sqrt(distAccel * 2.0 / (float) accelRate);
        tDecel = sqrt(distDecel * 2.0 / (float) decelRate);
      }
      float tAtSpeed = distAtSpeed / (float) speedTarget;

        // times are in microseconds
      timeAccel = 0;
      timeAtSpeed = tAccel * 1000000;
      timeDecel = timeAtSpeed + tAtSpeed * 1000000;


      pwmLoopI = 0;      // integrator starts empty for every move
      pwmLoopP = 0;
      pwmTrim = 0;
      noInterrupts();
      countEncoder = 0;
      interrupts();
      countEncoderLast = 0;
      posProfile = 0;
      speedAtDecel = -10000;
      timeRunning = 0;
      timeRunningLast = 0;
      timerUpdate = 0;
      flagStopped = 0;
      counterStopped = 0;
      running = 1;
    }

    void stop() {
      outputPWM = 0;
      outputFwd = 0;
      outputRev = 0;
      analogWrite(pinPWM,outputPWM);
      running = 0;
      posProfile = 0;
      speedProfile = 0;
      speedTarget = 0;
    }

    void updateMotion(long usecElapsed) {
      int flagUpdate = 0;

      long count = getCount();

      timerUpdate += usecElapsed;
      if (timerUpdate >= 30000) {
        long delta = count - countEncoderLast;
        speedActual = delta * 1000000L / timerUpdate;
        countEncoderLast = count;
        timerUpdate = 0;
        flagUpdate = 1;
        if (running == 0 && speedActual < 4) {
          if (!flagStopped) counterStopped++;
          if (counterStopped > 2) flagStopped = 1;
        }
      }

      if (running == 0) {
        outputPWM = 0;
        outputFwd = 0;
        outputRev = 0;
        pwmLoopP = 0;
        pwmLoopI = 0;
        return;
      }

      if (count >= (position - 2)) {
        if (debugPrint) { 
          Serial.print("STOPPED,");
          Serial.print("timeRunning:");
          Serial.print(timeRunning);
          Serial.print(",encoder:");
          Serial.print(count);
          Serial.print(",speedActual:");
          Serial.print(speedActual);
          Serial.print(",pwm:");
          Serial.println(outputPWM);
        }

        stop();
        return;
      }

      timeRunning += usecElapsed;
      if (flagUpdate == 0) return;

      float speed;
      if (timeRunning < timeAtSpeed) {
        speed = (float) timeRunning / 1000000.0 * (float) accelRate;
        speedProfile = (int) speed;
        if (debugPrint) { Serial.print(" - Accel speedProfile = "); Serial.println(speedProfile); }
      } else if (timeRunning < timeDecel) {
        speedProfile = speedTarget;
        if (debugPrint) { Serial.print(" - At Speed speedProfile = "); Serial.println(speedProfile); }
      } else {
        if (speedAtDecel <= -10000) speedAtDecel = speedProfile;
        speed = (float) (timeRunning - timeDecel) / 1000000.0 * (float) decelRate;
        speedProfile = speedAtDecel - (int) speed;
        if (speedProfile < speedMinimum) speedProfile = speedMinimum;
        if (debugPrint) { Serial.print(" - Decel speedProfile = "); Serial.println(speedProfile); }
      }

      posProfile += (speedProfile * (timeRunning - timeRunningLast)) / 1000000;
      timeRunningLast = timeRunning;

      long perror = posProfile - count;
      int serror = speedProfile - speedActual;
    
          // This program uses a PI loop to control speed
          // This loop is NOT tuned.  Meaning testing is required to tune the loop
          // which will provide the best preformance
      pwmLoopP = serror / 2;                  // Kp = 1/2
      int iCandidate = pwmLoopI + serror / 4; // Ki = 1/4
      int outCandidate = pwmLoopP + iCandidate + pwmTrim;

      // Anti-windup: stop integrating while the output is pinned at a limit and the
      // error would push it further past that limit. Otherwise the integrator keeps
      // growing during saturation and the wheel overshoots once it catches up.
      bool pushingHigh = (outCandidate > 254 && serror > 0);
      bool pushingLow  = (outCandidate < 0   && serror < 0);
      if (!pushingHigh && !pushingLow) pwmLoopI = iCandidate;
      pwmLoopI = constrain(pwmLoopI, -254, 254);

      outputPWM = pwmLoopP + pwmLoopI + pwmTrim;
      if (outputPWM < 0) outputPWM = 0;
      if (outputPWM > 254) outputPWM = 254;

      if (debugPrint) { 
        Serial.print("timeRunning:");
        Serial.print(timeRunning);
        Serial.print(",encoder:");
        Serial.print(count);
        Serial.print(",speedProfile:");
        Serial.print(speedProfile);
        Serial.print(",speedActual:");
        Serial.print(speedActual);
        Serial.print(",pos_error:");
        Serial.print(perror);
        Serial.print(",speed_error:");
        Serial.print(serror);
        Serial.print(",loopI:");
        Serial.print(pwmLoopI);
        Serial.print(",loopP:");
        Serial.print(pwmLoopP);
        Serial.print(",pwm:");
        Serial.println(outputPWM);
      }

      analogWrite(pinPWM,outputPWM);
    }

};

MotionLogic mtrLeft;
MotionLogic mtrRight;

//---------------------------------------------------------------------------------------
// Interupt function for counting left motor encoder pulses
void encoderIntLeft()  { 
  mtrLeft.incrEncoder();
}

//---------------------------------------------------------------------------------------
// Interupt function for counting right motor encoder pulses
void encoderIntRight()  { 
  mtrRight.incrEncoder();
}


//---------------------------------------------------------------------------------------
// Wheel sync: during a move both wheels should have turned the same number of counts.
// If one wheel gets ahead, slow it down and speed the other up in proportion to the gap.
// This keeps straight runs straight and turns symmetric, instead of each wheel only
// tracking its own speed.
void syncWheels() {
  long diff = mtrLeft.getCount() - mtrRight.getCount();   // + means left is ahead
  int trim = constrain(diff * SYNC_GAIN, -SYNC_MAX, SYNC_MAX);
  mtrLeft.setTrim(-trim);
  mtrRight.setTrim(trim);
}

//---------------------------------------------------------------------------------------
void setMotorOutputs() {
  if (mtrLeft.getOutputFwd()) digitalWrite(PIN_MTR1_DIR_FWD, HIGH);   
  else                        digitalWrite(PIN_MTR1_DIR_FWD, LOW); 
  if (mtrLeft.getOutputRev()) digitalWrite(PIN_MTR1_DIR_REV, HIGH);   
  else                        digitalWrite(PIN_MTR1_DIR_REV, LOW); 
  if (mtrRight.getOutputFwd()) digitalWrite(PIN_MTR2_DIR_FWD, HIGH);   
  else                         digitalWrite(PIN_MTR2_DIR_FWD, LOW); 
  if (mtrRight.getOutputRev()) digitalWrite(PIN_MTR2_DIR_REV, HIGH);   
  else                         digitalWrite(PIN_MTR2_DIR_REV, LOW); 
}

//======================================================================================
// The setup() is called once at the power up of the Arduino
//======================================================================================
void setup() {

  pinMode(PIN_LED, OUTPUT);
  flagLED = false;

  // Only uncomment one motor at a time to use the Serial Plotter function to tune the PID loop
  //mtrLeft.debugOn();
  //mtrRight.debugOn();

  if (mtrLeft.debugState() || mtrRight.debugState()) {
    Serial.begin(115200);
    Serial.println(F("Setup()..."));
  }

  pinMode(PIN_PB_START, INPUT_PULLUP);

  pinMode(PIN_MTR1_PWM,OUTPUT);
  pinMode(PIN_MTR2_PWM,OUTPUT);

  pinMode(PIN_MTR1_ENCA, INPUT_PULLUP);
  pinMode(PIN_MTR2_ENCA, INPUT_PULLUP);


  timerDelay = 0;

  attachInterrupt(digitalPinToInterrupt(PIN_MTR1_ENCA), encoderIntLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_MTR2_ENCA), encoderIntRight, RISING);

  pinMode(PIN_MTR1_DIR_FWD,OUTPUT);
  pinMode(PIN_MTR1_DIR_REV,OUTPUT);
  pinMode(PIN_MTR2_DIR_FWD,OUTPUT);
  pinMode(PIN_MTR2_DIR_REV,OUTPUT);
  digitalWrite(PIN_MTR1_DIR_FWD, LOW);
  digitalWrite(PIN_MTR1_DIR_REV, LOW);
  digitalWrite(PIN_MTR2_DIR_FWD, LOW);
  digitalWrite(PIN_MTR2_DIR_REV, LOW);

  speedFwd = 100;   
  speedTurn = 100;

  int speedAccel = 100;
  int speedMin  = SPEED_MIN;
  mtrLeft.setParams(speedAccel, speedMin, PIN_MTR1_PWM);
  mtrRight.setParams(speedAccel, speedMin, PIN_MTR2_PWM);

  //Serial.println(F("Initialize Display with background text"));
  //if (DISPLAY_PRESENT) { initDisplay(); }
  printStep = 0;

  loadCommandQueue();
  usScanLong = 0;
  usScanAvg = 0;
  timerusScan = 0;
  scanCount = 0;
  usLongResetCount = 0;
  //Serial.println(F("....End Setup"));

  usLast = micros();

}

//======================================================================================
// The following function will execute then exit.  The Ardunio will constantly call this 
// function.  The function should not have delays as this will effect the motor's speed.
//======================================================================================
void loop() {
  long distance;
  int speed;
  float fspd;
  long ldelta;
  long delayWait;
  int itmp;

    // this block calculates the number microseconds since this function's last execution
  unsigned long current = micros();
  usecElapsed = current - usLast;
  usLast = current;
  if (usecElapsed > usScanLong) usScanLong = usecElapsed;
  timerusScan += usecElapsed;
  scanCount++;
  if (timerusScan > 1000000) {
    usScanAvg = timerusScan / scanCount;
    timerusScan = 0;
    scanCount = 0;
    usLongResetCount++;
    if (usLongResetCount > 10) {
      usScanLong = 0;
      usLongResetCount = 0;
    }
  }

  // update motor speed and status
  mtrLeft.updateMotion(usecElapsed);
  mtrRight.updateMotion(usecElapsed);

    // updates the display every 200000us or 0.2 seconds
  /*if (msTimerPrint > 200000) {
    if (DISPLAY_PRESENT) { updateDisplay(); }
    msTimerPrint = 0;
  }*/
  msTimerPrint += usecElapsed;

  int newCmd = false;
  if (cmdQueue.firstScan()) {
    newCmd = true;
    //Serial.print(F("New Vehicle Cmd = "));
    //Serial.println(cmdQueue.current());
  }

  int pbStart = !digitalRead(PIN_PB_START);

  if (pbStart) {
    timerPBStartOn  += usecElapsed;
    timerPBStartOff = 0;
  } else {
    timerPBStartOn = 0;
    timerPBStartOff += usecElapsed;
  }
  
  if (cmdQueue.current() > VEHICLE_START && cmdQueue.current() < VEHICLE_ABORT) {
    if (timerPBStartOn > 100000) {
      cmdQueue.clear();
      cmdQueue.add(VEHICLE_ABORT);
      mtrLeft.stop();
      mtrRight.stop();
      setMotorOutputs();
    }
  }

  if (flagTimeRun) timerRunTime += usecElapsed;

  switch (cmdQueue.current()) {
    case VEHICLE_START_WAIT :
      if (timerPBStartOn > 100000) {
        cmdQueue.next();
      }
      break;
    case VEHICLE_START :
      timerRunTime = 0;
      if (timerPBStartOff > 100000) {
        cmdQueue.next();
        flagTimeRun = 1;
        flagLastMoveFwd = 0;
      }
      break;
    case VEHICLE_FORWARD :
      if (newCmd) {
        distance = ((long) cmdQueue.getParameter1() * (long) ENCODER_COUNTS_PER_REV / (long) MM_PER_REV);
        speed = speedFwd;
        mtrLeft.startMove(distance,speed);
        mtrRight.startMove(distance,speed);
        setMotorOutputs();
      }
      
      syncWheels();

      if (mtrLeft.isStopped() && mtrRight.isStopped()) {
        setMotorOutputs();
        cmdQueue.next();
      }
      break;
    case VEHICLE_TURN_RIGHT :
      if (newCmd) {
        distance = ENCODER_COUNTS_90_DEG;
        speed = speedTurn;
        mtrLeft.startMove(distance,speed * -1);
        mtrRight.startMove(distance,speed);
        setMotorOutputs();
      }

      syncWheels();

      if (mtrLeft.isStopped() && mtrRight.isStopped()) {
        setMotorOutputs();
        cmdQueue.next();
      }      
      
      break;
    case VEHICLE_TURN_LEFT :
      if (newCmd) {
        distance = ENCODER_COUNTS_90_DEG;
        speed = speedTurn;
        mtrLeft.startMove(distance,speed);
        mtrRight.startMove(distance,speed * -1);
        setMotorOutputs();
      }

      syncWheels();

      if (mtrLeft.isStopped() && mtrRight.isStopped()) {
        setMotorOutputs();
        cmdQueue.next();
      }      
      break;
    case VEHICLE_SET_MOVE_SPEED :
      speedFwd = cmdQueue.getParameter1();
      cmdQueue.next();
      break;
    case VEHICLE_SET_TURN_SPEED :
      speedTurn = cmdQueue.getParameter1();
      cmdQueue.next();
      break;
    case VEHICLE_SET_ACCEL :
      mtrLeft.setAccel(cmdQueue.getParameter1());
      mtrRight.setAccel(cmdQueue.getParameter1());
      cmdQueue.next();
      break;
    default :
    case VEHICLE_FINISHED :
      if (newCmd) {
        mtrLeft.stop();
        mtrRight.stop();
        setMotorOutputs();
      }
      flagTimeRun = 0;
      break;
    case VEHICLE_ABORT :
      mtrLeft.stop();
      mtrRight.stop();
      setMotorOutputs();
      flagTimeRun = 0;
      if (timerPBStartOff > 200000) {
        loadCommandQueue();
      }
      break;
  }

}
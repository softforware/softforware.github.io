#include <Servo.h>
#include <math.h>


Servo left;
Servo right;

//Need to find out time to rotate, assuming around 2 seconds / 2000 ms
//Delay is times in milliseconds
double ROTATION_TIME = 2150;

void rotateCW(float theta) {
  double rotation = 1.75 * (theta/360.0) * ROTATION_TIME;
  left.write(0);
  right.write(0);
  delay(rotation/2);
  left.write(90);
  right.write(90);
}

void rotateCCW(float theta) {
  double rotation = 0.97 * (theta/360.0) * ROTATION_TIME;
  left.write(180);
  right.write(180);
  delay(rotation);
  left.write(90);
  right.write(90);
}

void forward(float distance) {
 
  double x = distance * (850/10.0);
  left.write(180);
  right.write(0);
  delay(x/2);
  left.write(90);
  right.write(90);
  left.write(180);
  right.write(0);
  delay(x/2);
  left.write(90);
  right.write(90);
  
  }
  
void backward(float distance) {
  double x = distance * (850/10.0);
  left.write(0);
  right.write(180);
  delay(x/2);
  left.write(90);
  right.write(90);
  left.write(0);
  right.write(180);
  delay(x/2);
  left.write(90);
  right.write(90);

  }




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  left.attach(9);
  right.attach(10);
  forward(20); 
  delay(100); 
  forward(20); 
  delay(100);
  backward(20); 
  delay(100);
  rotateCW(90); 
  delay(100);
  rotateCCW(90);
  delay(100);

}

void loop() {
   
}


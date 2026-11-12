// Salinity detector: main sketch
// Reads the amplified cell voltage, converts it back to the cell voltage,
// converts that to NaCl concentration with an exponential calibration fit,
// and lights an LED for the concentration range.

float gain = 37;    // measured with gain_calibration.ino
float V0 = 0.91;    // reference voltage subtracted before amplification

void setup() {
  pinMode(A8, INPUT);   // measurement input pin
  pinMode(A11, INPUT);  // calibration input pin
  pinMode(A12, INPUT);  // calibration input pin
  pinMode(7, OUTPUT);   // red LED
  pinMode(6, OUTPUT);   // green LED
  pinMode(5, OUTPUT);   // blue LED
  Serial.begin(9600);
}

void loop() {
  float V_in = analogRead(A8) * 5.0 / 1023;

  float V = V_in / gain + V0;  // cell voltage before amplification

  // calibration fit: ppm = a * e^(b * V)
  double a = 6.2865 * pow(10, -9);
  double b = 27.1601;

  double ppm = a * exp(b * V);

  Serial.print("Voltage = ");
  Serial.print(V, 3);
  Serial.println(" V");

  Serial.print("Concentration = ");
  Serial.print(ppm, 0);
  Serial.println(" ppm");
  Serial.println();

  digitalWrite(7, LOW);
  digitalWrite(6, LOW);
  digitalWrite(5, LOW);

  if (-50 < ppm && ppm < 1500) {
    digitalWrite(5, HIGH);  // blue LED
  }
  if (1500 < ppm && ppm < 3500) {
    digitalWrite(6, HIGH);  // green LED
  }
  if (3500 < ppm && ppm < 10000) {
    digitalWrite(7, HIGH);  // red LED
  }

  delay(500);  // read every 0.5 seconds
}

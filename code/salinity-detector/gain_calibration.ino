// Salinity detector: gain calibration
// Measures the op-amp's actual gain: amplified output (A8) divided by the
// difference between its two inputs (A11, A12). Used to set `gain` in the
// main sketch.

void setup() {
  pinMode(A8, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);

  Serial.begin(9600);
}

void loop() {
  float V_in = analogRead(A8) * 5.0 / 1023;
  Serial.print("delta V amplified = ");
  Serial.println(V_in);

  float V1 = analogRead(A11) * 5.0 / 1023;
  float V2 = analogRead(A12) * 5.0 / 1023;
  Serial.print("V1 = ");
  Serial.println(V1);
  Serial.print("V2 = ");
  Serial.println(V2);
  float c = 0;
  Serial.print("Gain = ");
  Serial.println((V_in - c) / (abs(V2 - V1)));
  Serial.println();

  delay(500);
}

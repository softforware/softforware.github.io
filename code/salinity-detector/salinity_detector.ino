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

// ADC counts near either rail mean the amplifier has clipped
const int ADC_LOW_LIMIT  = 2;     // ~10 mV: cell is below the 0.91 V reference
const int ADC_HIGH_LIMIT = 1021;  // ~4.99 V: amplifier output is saturated

void setLeds(bool red, bool green, bool blue) {
  digitalWrite(7, red ? HIGH : LOW);
  digitalWrite(6, green ? HIGH : LOW);
  digitalWrite(5, blue ? HIGH : LOW);
}

void loop() {
  static bool blinkOn = false;
  blinkOn = !blinkOn;  // toggles every loop, used for out-of-range blinking

  int raw = analogRead(A8);
  float V_in = raw * 5.0 / 1023;

  float V = V_in / gain + V0;  // cell voltage before amplification

  // calibration fit: ppm = a * e^(b * V)
  double a = 6.2865 * pow(10, -9);
  double b = 27.1601;

  double ppm = a * exp(b * V);

  Serial.print("Voltage = ");
  Serial.print(V, 3);
  Serial.println(" V");

  if (raw <= ADC_LOW_LIMIT) {
    // Amplifier output is pinned at 0 V, so the real value is unknown and lower
    Serial.println("Concentration = BELOW RANGE");
    setLeds(false, false, blinkOn);           // blink blue
  } else if (raw >= ADC_HIGH_LIMIT || ppm > 10000) {
    // Amplifier saturated, or above the highest calibrated range
    Serial.println("Concentration = ABOVE RANGE");
    setLeds(blinkOn, false, false);           // blink red
  } else {
    Serial.print("Concentration = ");
    Serial.print(ppm, 0);
    Serial.println(" ppm");

    // Ranges share their boundaries so every value lights exactly one LED
    if (ppm < 1500) {
      setLeds(false, false, true);            // blue:  below 1500 ppm
    } else if (ppm < 3500) {
      setLeds(false, true, false);            // green: 1500 to 3500 ppm
    } else {
      setLeds(true, false, false);            // red:   3500 to 10000 ppm
    }
  }
  Serial.println();

  delay(500);  // read every 0.5 seconds
}

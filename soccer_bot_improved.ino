/*
 * Bluetooth-Controlled Soccer Robot — revised firmware (2026)
 *
 * This is a corrected rewrite of the original 2023 competition sketch, kept in this repo
 * as firmware/soccer_bot_original.ino. Four defects are addressed:
 *
 *   1. motorLeftA was never configured as an output (a duplicated pinMode call).
 *   2. Commands 'I' and 'G' were labelled "forward right/left" but spun the robot in place.
 *   3. No speed control — the motors were full-on or off.
 *   4. No failsafe — a dropped Bluetooth link latched the last command.
 *
 * Board: Arduino Uno.
 *
 * WIRING NOTE: the Bluetooth module has moved off the hardware UART (pins 0/1) onto
 * SoftwareSerial, so the module no longer has to be unplugged to upload code.
 * Arduino pin 2 (RX) <- module TX.  Arduino pin 3 (TX) -> module RX.
 * If the module is 3.3 V logic, put a divider on its RX line; 5 V straight in can damage it.
 */

#include <SoftwareSerial.h>

// ---------------------------------------------------------------- pin map
const uint8_t PIN_BT_RX = 2;   // Arduino receives on this pin
const uint8_t PIN_BT_TX = 3;   // Arduino transmits on this pin

const uint8_t PIN_RIGHT_A = 8;    // right motor forward
const uint8_t PIN_RIGHT_B = 9;    // right motor reverse
const uint8_t PIN_LEFT_B  = 10;   // left motor reverse
const uint8_t PIN_LEFT_A  = 11;   // left motor forward   <-- was never set to OUTPUT

const uint8_t PIN_EN_RIGHT = 5;   // H-bridge enable, must be a PWM pin
const uint8_t PIN_EN_LEFT  = 6;   // H-bridge enable, must be a PWM pin

// ---------------------------------------------------------------- tuning
const unsigned long COMMAND_TIMEOUT_MS = 300;  // stop if no command arrives within this window
const int SPEED_DEFAULT = 200;                 // 0-255
const int SPEED_MIN     = 80;                  // below this the TT motors stall
const float TURN_RATIO  = 0.40;                // inner-wheel speed during a curve

SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);

int  speedLevel    = SPEED_DEFAULT;
unsigned long lastCommandMs = 0;
bool isStopped     = true;

// ---------------------------------------------------------------- setup
void setup() {
  pinMode(PIN_RIGHT_A, OUTPUT);
  pinMode(PIN_RIGHT_B, OUTPUT);
  pinMode(PIN_LEFT_A,  OUTPUT);   // the fix: this line was a duplicate of PIN_RIGHT_B
  pinMode(PIN_LEFT_B,  OUTPUT);
  pinMode(PIN_EN_RIGHT, OUTPUT);
  pinMode(PIN_EN_LEFT,  OUTPUT);

  stopMotors();

  bt.begin(9600);
  Serial.begin(9600);             // USB, free for debugging now
  Serial.println(F("Soccer bot ready"));
}

// ---------------------------------------------------------------- main loop
void loop() {
  if (bt.available() > 0) {
    handleCommand(bt.read());
    lastCommandMs = millis();
  }

  // Failsafe: if the link drops mid-command, do not keep driving.
  if (!isStopped && (millis() - lastCommandMs > COMMAND_TIMEOUT_MS)) {
    stopMotors();
    Serial.println(F("timeout - motors stopped"));
  }
}

// ---------------------------------------------------------------- commands
void handleCommand(char c) {
  const int s     = speedLevel;
  const int inner = (int)(s * TURN_RATIO);

  switch (c) {
    case 'F': drive( s,      s     ); break;   // forward
    case 'B': drive(-s,     -s     ); break;   // backward
    case 'R': drive( 0,      s     ); break;   // pivot right (right wheel held)
    case 'L': drive( s,      0     ); break;   // pivot left  (left wheel held)
    case 'I': drive( inner,  s     ); break;   // forward-right curve
    case 'G': drive( s,      inner ); break;   // forward-left curve
    case 'X': drive( s,     -s     ); break;   // spin right in place
    case 'Y': drive(-s,      s     ); break;   // spin left in place
    case 'S': stopMotors();          break;    // stop

    default:
      // '0'..'9' sets the speed level without changing direction
      if (c >= '0' && c <= '9') {
        speedLevel = map(c - '0', 0, 9, SPEED_MIN, 255);
        Serial.print(F("speed = ")); Serial.println(speedLevel);
      }
      break;
  }
}

// ---------------------------------------------------------------- motor driver
// Signed speeds: positive drives forward, negative reverse, zero coasts.
void drive(int right, int left) {
  right = constrain(right, -255, 255);
  left  = constrain(left,  -255, 255);

  digitalWrite(PIN_RIGHT_A, right > 0 ? HIGH : LOW);
  digitalWrite(PIN_RIGHT_B, right < 0 ? HIGH : LOW);
  analogWrite (PIN_EN_RIGHT, abs(right));

  digitalWrite(PIN_LEFT_A, left > 0 ? HIGH : LOW);
  digitalWrite(PIN_LEFT_B, left < 0 ? HIGH : LOW);
  analogWrite (PIN_EN_LEFT, abs(left));

  isStopped = (right == 0 && left == 0);
}

void stopMotors() {
  digitalWrite(PIN_RIGHT_A, LOW);
  digitalWrite(PIN_RIGHT_B, LOW);
  digitalWrite(PIN_LEFT_A,  LOW);
  digitalWrite(PIN_LEFT_B,  LOW);
  analogWrite (PIN_EN_RIGHT, 0);
  analogWrite (PIN_EN_LEFT,  0);
  isStopped = true;
}

#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

// ── Pins ─────────────────────────────────────────────
const int irEmi   = 4;
const int irRec   = 5;
const int trigger = 11;
const int echo    = 12;

// ── Servos ───────────────────────────────────────────
Servo wheelRF, wheelRR, wheelLF, wheelLR;
Servo irServo;

// ── IR Servo angles ──────────────────────────────────
const int IR_CENTER = 90;
const int IR_LEFT   = 135;
const int IR_RIGHT  = 45;
const int SCAN_SETTLE_MS = 150; // ms to wait after servo moves before reading

// ── I2C ──────────────────────────────────────────────
#define SLAVE_ADDR 9

// ── Override state ───────────────────────────────────
bool overrideActive = false;
int  fakeIR         = 0;
int  fakeUS         = 100;

// ── Sensor readings ──────────────────────────────────
int  onLine     = 0;
long distanceUS = 0;

// ── Robot state ──────────────────────────────────────
enum State { FOLLOWING, SCANNING, TURNING, STOPPED };
State robotState = FOLLOWING;

// ── Constants ────────────────────────────────────────
const int STOP_DISTANCE = 20; // cm
const int TURN_DURATION = 400; // ms to turn before re-checking
const int LEFT  = 0;
const int RIGHT = 1;

unsigned long turnStartTime = 0;
int turnDirection = LEFT;

// ── Forward declarations ──────────────────────────────
void turn(int dir);
void driveForward();
void stopWheels();
long readUltrasound();
int  readIR();
int  scanForLine();

// ─────────────────────────────────────────────────────
void receiveEvent(int bytes) {
    if (bytes < 1) return;
    byte cmd = Wire.read();
    if (cmd == 0x01) {
        overrideActive = !overrideActive;
    } else if (cmd == 0x02 && bytes >= 3) {
        fakeIR = Wire.read();
        fakeUS = Wire.read();
    }
    while (Wire.available()) Wire.read();
}

void requestEvent() {
    byte buf[2];
    buf[0] = (byte)onLine;
    buf[1] = (byte)constrain(distanceUS, 0, 255);
    Wire.write(buf, 2);
}

// ─────────────────────────────────────────────────────
void setup() {
    Serial.begin(9600);

    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
    digitalWrite(trigger, LOW);

    pinMode(irEmi, OUTPUT);
    pinMode(irRec, INPUT);
    digitalWrite(irEmi, HIGH);

    wheelRF.attach(6);
    wheelLF.attach(8);
    wheelRR.attach(7);
    wheelLR.attach(9);
    irServo.attach(10);

    irServo.write(IR_CENTER);
    stopWheels();
    randomSeed(analogRead(A0)); // floating pin for entropy

    Serial.println("Slave ready");
}

// ─────────────────────────────────────────────────────
void loop() {
    // ── Read sensors ──
    if (overrideActive) {
        onLine     = fakeIR;
        distanceUS = fakeUS;
    } else {
        distanceUS = readUltrasound();
        onLine     = readIR();
    }

    // ── Obstacle check overrides everything ──
    if (distanceUS > 0 && distanceUS <= STOP_DISTANCE) {
        stopWheels();
        irServo.write(IR_CENTER);
        robotState = STOPPED;
        Serial.println("STOPPED: obstacle");
        delay(100);
        return;
    }

    // ── State machine ──
    switch (robotState) {

        case FOLLOWING:
            if (onLine) {
                driveForward();
            } else {
                stopWheels();
                robotState = SCANNING;
                Serial.println("Line lost -> SCANNING");
            }
            break;

        case SCANNING: {
            int found = scanForLine(); // blocks until scan is done
            if (found == -1) {
                // Line not found at all, stay stopped
                Serial.println("Line not found, staying stopped");
                robotState = STOPPED;
            } else {
                turnDirection = found;
                robotState = TURNING;
                turnStartTime = millis();
                Serial.print("Turning: ");
                Serial.println(turnDirection == LEFT ? "LEFT" : "RIGHT");
            }
            break;
        }

        case TURNING:
            turn(turnDirection);
            if (millis() - turnStartTime >= TURN_DURATION) {
                stopWheels();
                irServo.write(IR_CENTER);
                // Re-check after turn
                delay(SCAN_SETTLE_MS);
                onLine = readIR();
                robotState = onLine ? FOLLOWING : SCANNING;
            }
            break;

        case STOPPED:
            // Wait until line reappears directly under sensor
            if (onLine) {
                robotState = FOLLOWING;
                Serial.println("Line found -> FOLLOWING");
            }
            break;
    }

    // ── Debug ──
    Serial.print("State: "); Serial.print(robotState);
    Serial.print(" | Override: "); Serial.print(overrideActive);
    Serial.print(" | US: "); Serial.print(distanceUS);
    Serial.print("cm | Line: "); Serial.println(onLine);

    delay(50);
}

// ─────────────────────────────────────────────────────
// Sweeps servo left then right, returns LEFT, RIGHT,
// or a random choice if line found on both sides.
// Returns -1 if line not found on either side.
int scanForLine() {
    bool foundLeft  = false;
    bool foundRight = false;

    // Look left
    irServo.write(IR_LEFT);
    delay(SCAN_SETTLE_MS);
    foundLeft = (readIR() == 1);

    // Look right
    irServo.write(IR_RIGHT);
    delay(SCAN_SETTLE_MS);
    foundRight = (readIR() == 1);

    // Return servo to center
    irServo.write(IR_CENTER);
    delay(SCAN_SETTLE_MS);

    if (foundLeft && foundRight) {
        int choice = (random(2) == 0) ? LEFT : RIGHT;
        Serial.println("Line both sides -> random pick");
        return choice;
    } else if (foundLeft) {
        return LEFT;
    } else if (foundRight) {
        return RIGHT;
    } else {
        return -1;
    }
}

int readIR() {
    return (digitalRead(irRec) == LOW) ? 1 : 0;
}

long readUltrasound() {
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    long t = pulseIn(echo, HIGH, 30000);
    return t / 59;
}

void driveForward() {
    wheelRF.write(90);
    wheelRR.write(0);
    wheelLF.write(180);
    wheelLR.write(180);
}

void stopWheels() {
    wheelRF.write(355);
    wheelRR.write(89);
    wheelLF.write(91);
    wheelLR.write(87);
}

void turn(int dir) {
    if (dir == LEFT) {
        wheelRF.write(180);
        wheelRR.write(180);
        wheelLF.write(0);
        wheelLR.write(0);
    } else {
        wheelRF.write(0);
        wheelRR.write(0);
        wheelLF.write(180);
        wheelLR.write(180);
    }
}
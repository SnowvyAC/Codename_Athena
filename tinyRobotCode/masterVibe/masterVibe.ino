#include <Wire.h>

#define SLAVE_ADDR  9
#define RESPONSE_SZ 2

// ── Override values to inject ────────────────────────
int  fakeIR = 1;   // 1 = on line
int  fakeUS = 100; // cm, well above stop threshold

bool overrideOn = false;

// ────────────────────────────────────────────────────
void setup() {
    Wire.begin(); // master
    Serial.begin(9600);
    Serial.println("Master ready");
    Serial.println("Commands: 't' = toggle override | 'i' = set IR | 'u' = set US distance");
}

// ────────────────────────────────────────────────────
void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();

        if (cmd == 't') {
            // Toggle override on slave
            Wire.beginTransmission(SLAVE_ADDR);
            Wire.write(0x01);
            Wire.endTransmission();
            overrideOn = !overrideOn;
            Serial.print("Override toggled -> ");
            Serial.println(overrideOn ? "ON" : "OFF");

        } else if (cmd == 'i') {
            // Set fake IR value
            Serial.println("Enter IR value (0 or 1):");
            while (!Serial.available());
            fakeIR = Serial.parseInt();
            sendFakeValues();

        } else if (cmd == 'u') {
            // Set fake ultrasound distance
            Serial.println("Enter US distance in cm (0-255):");
            while (!Serial.available());
            fakeUS = Serial.parseInt();
            sendFakeValues();
        }
    }

    // ── Poll slave for current effective sensor state ──
    Wire.requestFrom(SLAVE_ADDR, RESPONSE_SZ);
    if (Wire.available() >= RESPONSE_SZ) {
        byte irVal = Wire.read();
        byte usVal = Wire.read();
        Serial.print("Slave reports | Line: ");
        Serial.print(irVal);
        Serial.print(" | US: ");
        Serial.print(usVal);
        Serial.println("cm");
    }

    delay(500);
}

// ────────────────────────────────────────────────────
void sendFakeValues() {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(0x02);
    Wire.write((byte)constrain(fakeIR, 0, 1));
    Wire.write((byte)constrain(fakeUS, 0, 255));
    Wire.endTransmission();
    Serial.print("Sent fake values -> IR: ");
    Serial.print(fakeIR);
    Serial.print(" | US: ");
    Serial.print(fakeUS);
    Serial.println("cm");
}
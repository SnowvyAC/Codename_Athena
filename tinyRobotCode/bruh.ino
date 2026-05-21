#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

//--Pins
const int irEmi = 4; //IR emitter
const int irRec = 5; //IR receiver
const int trigger = 11; //ultrasound trigger
const int echo = 12; //ultrasound echo

//--Servo
Servo wheelRF;
Servo wheelRR;
Servo wheelLF;
Servo wheelLR;
Servo irServo;

//Variables
int onLine;
int speed;

//I2C
#define SLAVE_ADDR 9
#define ANSWERSIZE 5
String answer = "Hello";

void setup() {

    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    Serial.begin(9600);
    //Ultrasound distance meter setup
    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
    digitalWrite(trigger, LOW);
    //Servo setup
    wheelRF.attach(6);
    wheelRR.attach(8);
    wheelLF.attach(7);
    wheelLR.attach(9);
    irServo.attach(10);
    //IR sensor setup
    pinMode(irEmi, OUTPUT);
    pinMode(irRec, INPUT);
    digitalWrite(irEmi, HIGH);

    void turn(dir)
    {
        if(dir == left){
            wheelRF.write(180);
            wheelRR.write(180);
            wheelLF.write(0);
            wheelLR.write(0);
        } else if(dir == right){
            wheelRF.write(0);
            wheelRR.write(0);
            wheelLF.write(180);
            wheelLR.write(180);
        }
    }
}

void loop() {

    long timeEcho;
    long distanceUS;

    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    timeEcho = pulseIn(echo, HIGH);
    distanceUS = timeEcho/59;  
    
    if(irRec == LOW){
        onLine = 1;
    } else {
        onLine = 0;
    }
    if(onLine == 0 && irServo == 0){
        ;
    }

    //Debug
    Serial.println("Distancia: " + String(distanceUS) + "cm");
    Serial.println("On Line: " + String(onLine));
    delay(100);
  


}
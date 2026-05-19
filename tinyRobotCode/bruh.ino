//Libraries
#include <Arduino.h>
#include <Servo.h>
//Pins
const int irEmi = 4; //IR emitter
const int irRec = 5; //IR receiver
const int trigger = 11; //ultrasound trigger
const int echo = 12; //ultrasound echo
//Servo
Servo wheelRF;
Servo wheelRR;
Servo wheelLF;
Servo wheelLR;
Servo ir;

void setup() {

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
    ir.attach(10);
}

void loop() {

    long t; //tiempo que demora en llegar el eco
    long d; //distancia en centímetro 

    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    t = pulseIn(echo, HIGH); //obtenemos el  
    d = t/59;               

    //Debug

    Serial.println("Distancia: " + String(d) + "cm");
    delay(100);
  



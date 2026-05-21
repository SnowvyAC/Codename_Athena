#include <Arduino.h>
#include <Servo.h>

String status = "Libre";
int timeEcho = 0;
int distanceUS = 0;
bool US = false;

Servo ruedaIzquierda;
Servo ruedaDerecha;

const int IR = 10;
const int Trigger = 11;
const int Echo = 12;

void caminar(String a) {
    if (a == "adelante") {
        ruedaIzquierda.write(0);
        ruedaDerecha.write(180);
    }
    else if (a == "atras") {
        ruedaIzquierda.write(180);
        ruedaDerecha.write(0); 
    }
    else if (a == "izquierda") {
        ruedaIzquierda.write(180);
        ruedaDerecha.write(180);
    }
    else if (a == "derecha") {
        ruedaIzquierda.write(0);
        ruedaDerecha.write(0);
    }
    else {
        ruedaIzquierda.write(90);
        ruedaDerecha.write(90);
    }
}

void ping() {
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger, LOW);

    timeEcho = pulseIn(Echo, HIGH);
    distanceUS = timeEcho/59;

    if (distanceUS <= 5) {
        US = true;
    }
    else{
        US = false;
    }
}

void vista() {
    if (digitalRead(IR) == LOW && US == false) {
        status = "Libre";
    }
    else {
        status = "Bloqueado";
    }
}

void setup() {
    
    Serial.begin(9600);

    pinMode(IR, INPUT);
    pinMode(Trigger, OUTPUT);
    pinMode(Echo, INPUT);

    ruedaIzquierda.attach(8);
    ruedaDerecha.attach(9);

}

void loop() {

    ping();
    vista();

    if (status == Libre) {
        caminar(adelante);
    }
    else if (status == Bloqueado) {
        caminar(izquierda);
        delay(5000);
        caminar(derecha);
    }
    //debug
    Serial.println("Estado:" + status);
    Serial.println("linea:" + String(IR));
    Serial.println("Camino abierto:" + String(US));

}
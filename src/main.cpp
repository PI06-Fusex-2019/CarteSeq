#include <Arduino.h>
#include <Servo.h>

/*Definition des E/S du PCB:*/
#define lecture_jack 6
#define switch_trappe 8
#define servo 9
#define led_trappe 2
#define led_predeco A5
#define led_deco 4
#define led_para A7
Servo trappe; //Initialisation Servo de la trappe

void setup() {
  // put your setup code here, to run once:

  trappe.attach(servo);
  trappe.write(10);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  trappe.write(160);
}
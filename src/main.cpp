#include <Arduino.h>
#include <Servo.h>

/*Definition des E/S du PCB:*/
#define lecture_jack 6
#define switch_trappe 8
#define servo 9
#define led_trappe 2 
#define led_predeco A5
#define led_deco 4
#define led_para A3

Servo trappe; //Initialisation Servo de la trappe

int temps(0);
bool jack_off(false); //Indique que le jack est enlevé
bool switchT(false); //Indique la position de l'interrupteur de trappe
bool trappe_on(false); //Indique que la trappe est vérrouillée


void setupled()
{
  pinMode(led_para, OUTPUT);
  pinMode(led_predeco, OUTPUT);
  pinMode(led_deco, OUTPUT);
  pinMode(led_trappe, OUTPUT);

  digitalWrite(led_trappe, LOW); //Indique que la trappe est verrouillée
  digitalWrite(led_predeco,LOW); //Indique que tout est pret pour le décollage
  digitalWrite(led_deco, LOW); //Indique que le décolage est detecté
  digitalWrite(led_para, LOW); //Indique que le parachute est déployé

}

void setup() {
  
  setupled();

  pinMode(lecture_jack, INPUT);
  trappe.attach(servo);
  trappe.write(10); //position fermé

  pinMode(switch_trappe, INPUT);
  switchT=digitalRead(switch_trappe);
  

  
}

void loop() {

  //commande manuel de la trappe
  if (!jack_off && digitalRead(switch_trappe)!=switchT) //la commande est limité au cas où le jack est enclenché.
  {
    if (trappe_on)
    {
      trappe_on=!trappe_on;
      switchT=digitalRead(switch_trappe);
      trappe.write(100); //on ouvre la trappe
      digitalWrite(led_trappe,LOW);
    

    }
    else
    {
      trappe_on=!trappe_on;
      switchT=digitalRead(switch_trappe);
      trappe.write(10); //on ferme la trappe
      digitalWrite(led_trappe,HIGH);
    }
    

  }

  if (trappe_on && !jack_off && digitalRead(lecture_jack)==false) //detection de l'arrachage du cable 
  {
    jack_off=true;
    temps=millis(); //on initialise le compte à rebours
    digitalWrite(led_deco,HIGH);
    
  }
  
  if (jack_off && millis()-temps > 5600) //On ouvre le parachute  apres 5.6 seconde ~ temps pour arriver à l'apogé, peut être prévoir plus
  {
  
  trappe.write(160); //on ouvre le locket
  digitalWrite(led_para,HIGH); //le parachute est deployé
  digitalWrite(led_trappe,LOW);
  }
}



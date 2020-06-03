#include <Arduino.h>
#include <Servo.h>
#include "comm_intern.h"

/*Definition des E/S du PCB:*/
#define lecture_jack 6
#define switch_trappe 8
#define servo 9
#define led_trappe 2 
#define led_predeco A5
#define led_deco 4
#define led_para A3

/* Définition du bus série UART communiquant avec la carte expérience */
#define SERIALSEQ Serial

//VARIABLES COMMUNICATION INTERNE
bool seq_decollage_detect = false;
bool exp_decollage_detect = false;
bool msg_decollage_detect = false;
bool apogee_detectee = false;
bool seq_decollage_detectParExp = false;
bool acquittement_seq_exp = false; //Acquittement de la carte séquenceur envers la carte experience
bool acquittement_exp_seq = false; //Acquittement de la carte experience envers la carte séquenceur

bool acquittement_apogee = false;
bool acquittement_decollage_exp = false;

bool parachute_deploye = false;
bool msg_parachute = false;
int t_msg_seq = 0;
int t_msg_exp = 0;

int msg;
int msgB0;
int msgB1;

Servo trappe; //Initialisation Servo de la trappe

int temps(0);
bool jack_off(false); //Indique que le jack est enlevé
bool switchT(false); //Indique la position de l'interrupteur de trappe
bool trappe_on(false); //Indique que la trappe est vérrouillée

void fctEmetteurSeq();

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
  
  Serial.begin(9600); //Ouverture de la liaison série UART avec carte exp
  
}

void loop() {


  //Boucle de reception des données sur UART
  if(SERIALSEQ.available()>0){
    msg = SERIALSEQ.read();
    if(msg == MARQUEUR){
      delay(10);
      msgB0 = SERIALSEQ.read();
      msgB1 = SERIALSEQ.read();
    }
    if(msgB0 == msgB1){ //Le message est valide ssi msgB0 == msgB1
      switch (msgB0)
      {
      case EXP_ACQUITTEMENT:
        acquittement_exp_seq = true;
        break;
      case EXP_DECOLLAGE:
        //La carte expérience a détecté le décollage
        seq_decollage_detect = true;
        seq_decollage_detectParExp = true;
        
        //Emission d'une message d'acquittement
        SERIALSEQ.write(MARQUEUR);
        SERIALSEQ.write(SEQ_ACQUITTEMENT_DECOLLAGE);
        SERIALSEQ.write(SEQ_ACQUITTEMENT_DECOLLAGE);
        break;
      case EXP_APOGEE:
        parachute_deploye = true;
        //La carte expérience transmet l'info : apogee atteint, on doit donc déployer le parachute
        SERIALSEQ.write(MARQUEUR);
        SERIALSEQ.write(SEQ_ACQUITTEMENT_APOGEE);
        SERIALSEQ.write(SEQ_ACQUITTEMENT_APOGEE);
        break;
      default:
        break;
      }
    }
  }

  fctEmetteurSeq(); 

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

  //BOUCLES DE DETECTION DE DECOLLAGE
  //DETECTION DE DECOLLAGE PAR ARRACHEMENT DU JACK
  if (trappe_on && !jack_off && digitalRead(lecture_jack)==false && !seq_decollage_detectParExp) //detection de l'arrachage du cable 
  {
    jack_off=true;
    temps=millis(); //on initialise le compte à rebours
    digitalWrite(led_deco,HIGH);
    
    seq_decollage_detect = true; //Envoi de l'information à la carte exp
  }
  //DETECTION DU DECOLLAGE PAR LA CARTE EXP
  if(seq_decollage_detectParExp && seq_decollage_detect && !jack_off){
    jack_off=true;
    temps=millis(); //on initialise le compte à rebours
    digitalWrite(led_deco,HIGH);
  }
  //on en peut pas entrer dans les deux boucles car jack_off passe à 1 dans tous les cas
  //FIN DE BOUCLES DE DETECTION DE DECOLLAGE
  
  /* BOUCLE DE DEPLOIEMENT DU PARACHUTE */
  //DEPLOIEMENT APRES TIMING || DEPLOIEMENT APRES TRANSMISSION DE LA CARTE EXP (DETECTION DE L'APOGEE)
  if (jack_off && millis()-temps > 5600 && !parachute_deploye || jack_off && parachute_deploye) //On ouvre le parachute  apres 5.6 seconde ~ temps pour arriver à l'apogé, peut être prévoir plus
  {
  
  trappe.write(160); //on ouvre le locket
  digitalWrite(led_para,HIGH); //le parachute est deployé
  digitalWrite(led_trappe,LOW);
  //Transmission de l'information à la carte expérience
  parachute_deploye = true;

  }
}

void fctEmetteurSeq(){
  //FONCTION ENVOI CARTE SEQUENCEUR -> Detection de decollage
  if(seq_decollage_detect && !msg_decollage_detect && !seq_decollage_detectParExp){
    //Si on detecte le decollage, on envoie le message à la carte expérience

    msg_decollage_detect = true;
    t_msg_seq = millis();

    SERIALSEQ.write(MARQUEUR);
    SERIALSEQ.write(SEQ_DECOLLAGE);
    SERIALSEQ.write(SEQ_DECOLLAGE);
  }

  //ATTENTE D'ACQUITTEMENT APRES MESSAGE DE DECOLLAGE
  if(!acquittement_exp_seq && msg_decollage_detect && millis()-t_msg_seq >500){
    msg_decollage_detect = false;
  }

  if(parachute_deploye && !msg_parachute){
    SERIALSEQ.write(MARQUEUR);
    SERIALSEQ.write(SEQ_PARACHUTE);
    SERIALSEQ.write(SEQ_PARACHUTE);
    msg_parachute = true;
  }
}




/*
Projet LED ARDUINO/ANDROID	
*/


#include "FlexiTimer2.h"


static int etatLed13 = 0;
static int etatLed12 = 0;
static int boutonLed13 = 1;
static int boutonLed12 = 2;
char msg;
int led13 = 13;
int led12 = 12;
int btn = 2;
volatile int state = LOW;      // The input state toggle


void setup() {
	Serial.begin(115200);
	pinMode(led13, OUTPUT); 
	pinMode(led12, OUTPUT);
	pinMode(btn, INPUT);
	attachInterrupt(0, capteurRoueEvent, RISING); // attache l'interruption externe n°0 à la fonction blink
	FlexiTimer2::set(10, serialTimer);// initialise Timer1 période 500 milliseconde	
	FlexiTimer2::start();
}

void serialTimer(void){
// renvoie le car recu
// appel au  10 millisecondes {
	if (Serial.peek() != -1) {		
		do {
			msg = Serial.read();
			Serial.print(msg);
			
			if(msg == boutonLed13)
			{
				if ( etatLed13 == 0)
				{
					digitalWrite(led13, HIGH);   // set the LED on
					etatLed13=1;
					//Serial.print(50);
					//Serial.print("\n");
				}
				else
				{
					digitalWrite(led13, LOW);   // set the LED off
					etatLed13=0;
				}
			}
			if(msg == boutonLed12)
			{
				if ( etatLed12 == 0)
				{
					digitalWrite(led12, HIGH);   // set the LED on
					etatLed12=1;					
				}
				else
				{
					digitalWrite(led12, LOW);   // set the LED off
					etatLed12=0;
				}
			}
		} while (Serial.peek() != -1);		
			Serial.print("\n");		
	}	
}


void capteurRoueEvent(void)
{
	Serial.println("vitesse:50");	 
}


void loop(){	
}





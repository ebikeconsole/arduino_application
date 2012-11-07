/********************************************* 
Projet Ebike Console
Utilisation d'un terminal Android 
et d'une board Arduino


Cr�ateur : Sragneau
Date : 03/11/2012
**********************************************/

// --- Inclusion des librairies utilis�es ---
#include <FlexiTimer2.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <FreqPeriodCounter.h>

// DEFINITIONS
//#define wheelCircumference 2.16
#define debounceTime_ms 1 // pulse > 3ms
#define wheelTimeOut_ms 2000
#define numberOfMagnetsPerWheel 1

// --- D�claration des constantes ---
float deuxPi = 6.28318; // Deux Pi (Calcul de la distance)
float Pi = 3.14159;		// Pi (Calcul de la distance)
int PULSE_TIMEOUT = 1000000;

// --- constantes des broches ---
const byte wheelSensorPin = 2; // variable de broche
int currentSensorPin = A0;	  
int voltBatPin = A1;		   //Mesure de tenson

// --- D�claration des variables globales ---
char msgAndroid;

//Variables de temps 
volatile int compt_millisec=0;   
int seconde=0;           					 
int minute=0;            
int heure=0;
int jour=0;
volatile double dureeTotal=0;
int mouvement = 1;
int cptMouvement;

//Variable du temps total (Eeprom)
int secondeTotal=0;				//non-effa�able		 
int minuteTotal=0;				//non-effa�able
int heureTotal=0;				//non-effa�able   
int jourTotal=0;				//non-effa�able

//Variable velo
volatile double rps;			//Tour par seconde
volatile double rpsTotal;		//Tour par seconde cumul� 
double vitesse;					//Vitesse instantan�e
double vitesseMoyenne;			//Vitesse moyenne					eeprom   effa�able 
float distance;					//Distance en cours					eeprom   effa�able
float distanceTotal;			//Distance Totale (jamais effac�)	eeprom   non-effa�able
double hauteurRoue;				//Rayon de roue						eeprom   param�trable
float  wheelCircumference;		//Circonf�rence
float k;						//Caract�ristique roue (nombre de magnet)

//Variable Batterie
int consoBat;					//consommation en mA				eeprom	 effa�able
int consoMaxBat;				//consommation max mA				eeprom	 param�trable
float tensionBat;				//tension en V 15.5V
float tensionMiniBat;			//tension mini 40.0V				eeprom	 param�trable

//Variables ACS715
int sensorValue = 0;			// value read from the carrier board
int outputValue = 0;			// output in milliamps

//Mesure Tension
float batVolt;

//Gestion Eeprom
int infoSave = 0;
 

FreqPeriodCounter wheelSpeed(wheelSensorPin, millis, 0);


//**************** FONCTION SETUP = Code d'initialisation *****
void setup() {

	//Ouverture du port USB
	Serial.begin(115200);

	//D�finition des entr�es/sorties arduino	
	pinMode(wheelSensorPin, INPUT);

	// on associe l'interruption externe n�0 pin2 � la fonction capteurRoueEvent
	attachInterrupt(0, wheelSensorEvent, CHANGE); 

	//Creation d'une m�thode qui se d�clanche toutes les ms
	//Utilis� pour traitement USB
	FlexiTimer2::set(1, Timer);			  	
	FlexiTimer2::start();


	//R�cup�ration des valeurs dans l'eeprom				
	EEPROM_readAnything(0, hauteurRoue);		//Rayon de roue
	wheelCircumference = hauteurRoue * 2 * Pi;
	k = (float)3600 * wheelCircumference / numberOfMagnetsPerWheel ; // kmh = k / period
	
	Serial.print("hauteur de roue : ");
	Serial.println(hauteurRoue, DEC);		//Debug Eeprom rayon


}// fin de la fonction setup()


/********************************************* 
Timer() 
Ecoute des messages entrants par USB
Cacul du temps 
**********************************************/
void Timer(void)
{
	// Cacul du temps
	if (mouvement == 1){ 
		compt_millisec=compt_millisec+1; // s'incr�mente toutes les millisecondes
		if (compt_millisec==1000)seconde=seconde+1, compt_millisec=0; // incr�mentation des secondes... toutes les 1000 millisecondes et RAZ
		//--- gestion des variables de l'heure ---
		if (seconde==60) minute++,seconde=0;
		if (minute==60) heure++, minute=0;
		if (heure==24) jour++, heure=0;	
	}

	// Mesure de vitesse
	wheelSpeed.poll(); 
	refreshVitesseMoyenne();
	refreshDistance();
	refreshDistanceTotal();

    // Mesure du courant
	sensorValue = analogRead(currentSensorPin);
	outputValue = (((long)sensorValue * (5000/1024)) - 500)/0.133;

	//Mesure de Tension	 
	batVolt = analogRead(voltBatPin) * 0.00488;	  //(5000/1014)
	batVolt = batVolt * 10;						  //   R1/R2
	
	// Gestion des flux Arduino/Android
	if (Serial.peek() != -1) {		
		do {
			msgAndroid = Serial.read();
			//Serial.print(msg); //Debug 

			switch (msgAndroid)
			{
			case 1: ;
			case 2: ;
			case 3: ;
			//...
			}

			
		} while (Serial.peek() != -1);		
		Serial.print("\n");		
	}// Fin Gestion des flux Arduino/Android	
}// Fin m�thode timer

/********************************************* 
capteurRoueEvent() 
M�thode d�clanch�e � chaque tour de roue
On compte le nombre de tour de roue
**********************************************/
void wheelSensorEvent(void)
{		
	wheelSpeed.poll();
	//Nb tour de roue
	++rps;		//A Diviser par deux (Front Montant et Descendant)
	++rpsTotal;	//A Diviser par deux (Front Montant et Descendant)
}

/***********************************************
//Enregistre  les valeurs dans l'eeprom
//Raffraichie les valeurs � envoyer � l'android
//Envoie les donn�es
//Se d�roule toutes les secondes
***********************************************/
void loop(){
	//Calcul de temps
	dureeTotal=seconde + minute*60 + heure*3600;
	
	//Calcul vitesse
	Serial.print("  ");Serial.print(rpsTotal);Serial.print("tr total");  //Debug rps		
	Serial.print("  ");Serial.print(k);Serial.print("k");  //Debug rps

	Serial.print("  ");Serial.print(minute);Serial.print(":");Serial.print(seconde);	//Debug timer
	Serial.print("  ");Serial.print(kmh(),DEC);Serial.print("km/h");					//Debug vitesse instantan�e
	Serial.print("  ");Serial.print(vitesseMoyenne,DEC);Serial.print("km/h");			//Debug vitesse instantan�e
	Serial.print("  ");Serial.print(dureeTotal);Serial.print("s");						//Debug dur�e total
	//Serial.print("  ");Serial.print(cptMouvement);Serial.print("cpt");					//Debug cptMouvement
	Serial.print("  ");Serial.print(distance);Serial.print("km");					    //Debug distance
	Serial.print("  ");Serial.print(distanceTotal);Serial.print("km_total");			//Debug distance total
	
	Serial.print("\n");
	
	if((int)kmh() == 0)
	{		
		++cptMouvement;
		if(cptMouvement>2){
			mouvement=0;	// On stope l'incr�mentation du temps
			if(infoSave == 0)
			{
				//Lancer la sauvegarde sur l'eeprom
				//..
				infoSave = 1;
			}
		}
	}
	else{
		mouvement=1;		// On remet l'incr�mentation
		cptMouvement=0;
		infoSave = 0;
	}
	delay(1000);
}


void refreshVitesseMoyenne(){	
	vitesseMoyenne = ((rpsTotal/2) * k ) / (1000 * dureeTotal);	
}

inline int wheelPeriod()
{ if(wheelSpeed.elapsedTime > wheelTimeOut_ms) return 0;
  else return wheelSpeed.period;
}


inline float kmh(){ 
    return (float)k /wheelPeriod();  
}
 
inline float rpm()
{ return (float)60000 / wheelPeriod();
}

void refreshDistance(){	
	distance = (rps/2) * wheelCircumference;
}

void refreshDistanceTotal(){	
	distanceTotal += distance;
}
// --- Fin du programme ---





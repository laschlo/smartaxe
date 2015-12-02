/*
 * Sketch2.ino
 *
 * Created: 10/20/2015 6:08:31 PM
 *Zmieniona linia
 * Author: Cichocki
 * I ta tez zmieniona linia
 */ 

#define PUMP_OFF		1
#define PUMP_ON			0
#define PUMP_CWU_ON		PUMP_ON
#define PUMP_CWU_OFF	PUMP_OFF
#define PUMP_CO_ON		PUMP_ON
#define PUMP_CO_OFF		PUMP_OFF

#define PUMP_CWU_PIN	5
#define PUMP_CO_PIN		6

#define TEMP_FIRE_MIN	45
#define TEMP_FIRE_MAX	70
#define TEMP_WATER_MAX	50

#define DEBUG

uint16_t	tempWater = 0;
uint16_t	tempFire = 0;

void cwuOn(void){
	digitalWrite(PUMP_CWU_PIN,PUMP_ON);
#ifdef DEBUG
	Serial.println("CWU PUMP ON");
#endif
}

void cwuOff(void){
	digitalWrite(PUMP_CWU_PIN,PUMP_OFF);
#ifdef DEBUG
	Serial.println("CWU PUMP OFF");
#endif
}

void coOn(void){
	digitalWrite(PUMP_CO_PIN,PUMP_ON);
#ifdef DEBUG
	Serial.println("CO PUMP ON");
#endif
}
void coOff(void){
	digitalWrite(PUMP_CO_PIN,PUMP_OFF);
#ifdef DEBUG
	Serial.println("CO PUMP OFF");
#endif
}

void setup()
{

	  Serial.begin(9600);
	  pinMode(PUMP_CO_PIN,OUTPUT);
	  pinMode(PUMP_CWU_PIN,OUTPUT);
	  cwuOff();
	  digitalWrite(PUMP_CO_PIN,PUMP_OFF);
}

void loop()
{

	  tempWater = analogRead(0);
	  tempWater = map(tempWater,0,1023,20,90);
	  Serial.print("Water temperature: ");
	  Serial.println(tempWater);
	  
	  tempFire = analogRead(1);
	  tempFire = map(tempFire,0,1023,20,90);
	  Serial.print("Fireplace temperature: ");
	  Serial.println(tempFire);
	  
	  if(tempFire >= TEMP_FIRE_MIN){ //Minimum temperature for fire-place reached!
		  if(tempFire <= TEMP_FIRE_MAX){ //We don't want to exceed maximum temperature for fire-place
			  if(tempWater <= TEMP_WATER_MAX){
				  coOff();
				  if(tempWater <= tempFire){
					  cwuOn();
				  }else{//tempWater > tempFire
					  cwuOff();
				  }
			  }else{//tempWater > TEMP_WATER_MAX - Water temperature reached desired value so we can heat 
				  cwuOff();
				  coOn();
			  }
		  }else{// tempFire > TEMP_FIRE_MAX) - Fire-place reached max temperature so turn both pumps on to cool down
			  cwuOn();
			  coOn();
		  }
	  }else{//tempFire < TEMP_FIRE_MIN
		  cwuOff();
		  coOff();
	  }
	  
	  delay(1000);

}

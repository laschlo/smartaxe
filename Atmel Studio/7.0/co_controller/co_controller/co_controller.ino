/* 
Uses library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0) */
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 8
#define TEMPERATURE_PRECISION 9

#define PUMP_OFF		0
#define PUMP_ON			1
#define PUMP_CWU_PIN	5
#define PUMP_CO_PIN		6

#define TEMP_FIRE_MIN	45
#define TEMP_FIRE_MAX	69
#define TEMP_WATER_MAX	50

#define	ONE_SECOND	1000
#define ONE_MINUTE	(60*ONE_SECOND)
#define PCO_DELAY	(6*ONE_MINUTE)
#define PCO_WORK_TIME	(2*ONE_MINUTE)


#define DEBUG

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

struct Thermometer {
	DeviceAddress	address;
	uint16_t		currentTemperature;
	uint16_t		previousTemperature;
};

Thermometer	thermometerFire, thermometerWater;
uint8_t WaterTempRise = TRUE;
uint8_t WaterHisteresis_HIGH = 1;
uint8_t WaterHisteresis_LOW = 5;

unsigned long	currentMillis,prevMillisCo,prevMillisDelay;

LiquidCrystal_I2C	lcd(0x20,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack

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
	// activate LCD module
	Serial.begin(9600);
	pinMode(PUMP_CO_PIN,OUTPUT);
	pinMode(PUMP_CWU_PIN,OUTPUT);	
	
	//Set up LCD
	lcd.begin (16,2); // for 16 x 2 LCD module
	lcd.setBacklightPin(3,POSITIVE);
	lcd.setBacklight(HIGH);		
	
	// Start up the Dallas library
	sensors.begin();
	
	prevMillisDelay = millis();

#ifdef DEBUG
	Serial.println("Dallas Temperature IC Control Library Demo");
	
	// locate devices on the bus
	Serial.print("Locating devices...");
	Serial.print("Found ");
	Serial.print(sensors.getDeviceCount(), DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors.isParasitePowerMode()) Serial.println("ON");
	else Serial.println("OFF");
#endif	

	// method 1: by index
	if (!sensors.getAddress(thermometerFire.address, 0)) Serial.println("Unable to find address for Device 0");
	if (!sensors.getAddress(thermometerWater.address, 1)) Serial.println("Unable to find address for Device 1");

	// method 2: search()
	// search() looks for the next device. Returns 1 if a new address has been
	// returned. A zero might mean that the bus is shorted, there are no devices,
	// or you have already retrieved all of them.  It might be a good idea to
	// check the CRC to make sure you didn't get garbage.  The order is
	// deterministic. You will always get the same devices in the same order
	//
	// Must be called before search()
	//oneWire.reset_search();
	// assigns the first address found to thermometerFire
	//if (!oneWire.search(thermometerFire)) Serial.println("Unable to find address for thermometerFire");
	// assigns the seconds address found to thermometerWater
	//if (!oneWire.search(thermometerWater)) Serial.println("Unable to find address for thermometerWater");

#ifdef DEBUG
	// show the addresses we found on the bus
	Serial.print("Device 0 Address: ");
	printAddress(thermometerFire.address);
	Serial.println();

	Serial.print("Device 1 Address: ");
	printAddress(thermometerWater.address);
	Serial.println();
#endif	

	// set the resolution to 9 bit
	sensors.setResolution(thermometerFire.address, 9);
	sensors.setResolution(thermometerWater.address, 9);

#ifdef DEBUG
	Serial.print("Device 0 Resolution: ");
	Serial.print(sensors.getResolution(thermometerFire.address), DEC);
	Serial.println();

	Serial.print("Device 1 Resolution: ");
	Serial.print(sensors.getResolution(thermometerWater.address), DEC);
	Serial.println();
#endif
}

#ifdef DEBUG
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
	uint16_t tempC = sensors.getTemp(deviceAddress)/128;
	Serial.print("Temp C: ");
	Serial.print(tempC);
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
	Serial.print("Device Address: ");
	printAddress(deviceAddress);
	Serial.print(" ");
	printTemperature(deviceAddress);
	Serial.println();
}
#endif //DEBUG

void printMainScreen(uint8_t co, uint8_t cwu){
	//lcd.clear();
	lcd.home (); // set cursor to 0,0
	lcd.print("KOM: ");
	lcd.setCursor(0,1);
	lcd.print("CWU:");
	
	lcd.setCursor(5,0);
	lcd.print(co);
	lcd.print("C ");
	
	lcd.setCursor(5,1);
	lcd.print(cwu);
	lcd.print("C ");
	
	//Update pumps status on LCD
	lcd.setCursor(10,0);
	lcd.print("P_CO:");
	if(digitalRead(PUMP_CO_PIN)){
		lcd.print("X");
	}else{
		lcd.print(" ");
	}
		
	lcd.setCursor(9,1);
	lcd.print("P_CWU:");
	if(digitalRead(PUMP_CWU_PIN)){
		lcd.print("X");
	}else{
		lcd.print(" ");
	}
}

void loop()
{	

	// call sensors.requestTemperatures() to issue a global temperature
	// request to all devices on the bus
#ifdef DEBUG
	Serial.print("Requesting temperatures...");
#endif	

	sensors.requestTemperatures();
	
#ifdef DEBUG
	Serial.println("DONE");
#endif	
	
	
	thermometerFire.currentTemperature = sensors.getTemp(thermometerFire.address)/128;
	thermometerWater.currentTemperature = sensors.getTemp(thermometerWater.address)/128;


#ifdef DEBUG
	Serial.print("Temperatura kominka: ");
	Serial.println(thermometerFire.currentTemperature);
#endif	
	
#ifdef DEBUG
	Serial.print("Temperatura wody: ");
	Serial.println(thermometerWater.currentTemperature);
#endif		
	
	if(thermometerFire.currentTemperature >= TEMP_FIRE_MIN){ //Minimum temperature for fire-place reached!
		if(thermometerFire.currentTemperature <= TEMP_FIRE_MAX){ //We don't want to exceed maximum temperature for fire-place
			if((thermometerWater.currentTemperature < (TEMP_WATER_MAX + WaterHisteresis_HIGH)) && WaterTempRise){
				//coOff();
				
				/*  */
				currentMillis = millis();
				if (currentMillis - prevMillisDelay >= 6*60000UL){
					prevMillisCo = prevMillisDelay + 6*60000UL;
					coOn();
					if(currentMillis - prevMillisCo >= 2*60000UL){
						coOff();
						prevMillisDelay = millis();
					}
					
				}else{
					coOff();
				}
				/* */
				
				if(thermometerWater.currentTemperature <= thermometerFire.currentTemperature){
					cwuOn();
				}else{//thermometerWater > thermometerFire
					cwuOff();
				}
			}else{//thermometerWater > TEMP_WATER_MAX
				if(thermometerWater.currentTemperature <= (TEMP_WATER_MAX - WaterHisteresis_LOW)){
					WaterTempRise = TRUE;
				}else{
					WaterTempRise = FALSE;
					cwuOff();
					coOn();					
				}				
			}
		}else{// thermometerFire > TEMP_FIRE_MAX) - Fire-place reached max temperature so turn both pumps on to cool down
			cwuOn();
			coOn();
		}
	}else{//thermometerFire < TEMP_FIRE_MIN
		cwuOff();
		coOff();
		prevMillisDelay = millis();
	}	

	printMainScreen(thermometerFire.currentTemperature,thermometerWater.currentTemperature);
	delay(1000);
}

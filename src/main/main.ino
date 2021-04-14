//------------------------------------------------------------------------------------------------------------------------------------------------------ REAL TIME CLOCK
#include <Wire.h> // declare I2C bus for RTC and EEPROM
#include <DS1307.h> // declare library for RTC functions
#include <WProgram.h>

int rtc[7]; // declare array of number for saving time and date
byte current_hour;

unsigned long currentMillis; // declare variable for storing current arduino time
unsigned long previousMillisTouch = 0; // declare variable for track time between touches
unsigned long previousMillisUpdate = 0; // variable for track time between refresing sensors reading (temperature,humidity)
unsigned long previousMillisFlow = 0; // variable for track time between refresing sensors reading (flow rate)
unsigned long previousMillisSchedule = 0; // variable for track time between chedule events
unsigned long previousMillisLED = 0; // variable for track time between chedule events
int sensor_update; //delay for update reading from sensor
//___________________________________________________________________________________________________________________ end of REAL TIME CLOCK
//------------------------------------------------------------------------------------------------------------------------------------------------------- LIGHTS
const byte LED_Pin = 11;

struct LIGHT
{ 
  byte status;  // 0 = light off, 1 = light on, 2 = sunrise, 3 = sunset 
  boolean button_active; // if buttons on screen are active or not
  byte hour_ON;  // declare variable for start lighting period
  byte minute_ON; // declare variable for start lighting period
  byte hour_OFF; // declare variable for stop lighting period
  byte minute_OFF; // declare variable for stp lighting period
  byte dimming; // variable to know how long should sunrise and sunset be
  byte brightness; // value to store percentage of emited light 
  boolean status_check; //value to activate light status checking
  byte dimming_brightness; //value for dimming LED
  long brightness_max; //max value for dimming LED
  long delay; //delay for dimming
} 
LED; // declare variables for store Light settings
//___________________________________________________________________________________________________________________ end of LIGHT
//================================================================================================================================================================================ LIGHT VALUE
byte ResetVariables;
byte ResetVariablesEEPROM = 0;

byte LED_hour_ON_eeprom = 1;
byte LED_minute_ON_eeprom = 2;
byte LED_hour_OFF_eeprom = 3;
byte LED_minute_OFF_eeprom = 4;
byte LED_dimming_eeprom = 5;
byte LED_brightness_eeprom = 6;


void ResetDefaultValuesEEPROM()
{
LED.hour_ON=11; WriteToEEPROM(LED_hour_ON_eeprom,LED.hour_ON); 
LED.minute_ON=0; WriteToEEPROM(LED_minute_ON_eeprom,LED.minute_ON); 
LED.hour_OFF=20; WriteToEEPROM(LED_hour_OFF_eeprom,LED.hour_OFF);
LED.minute_OFF=0; WriteToEEPROM(LED_minute_OFF_eeprom,LED.minute_OFF); 
LED.dimming=30; WriteToEEPROM(LED_brightness_eeprom,LED.dimming);  
LED.brightness=30; WriteToEEPROM(LED_brightness_eeprom,LED.brightness);  
}
//___________________________________________________________________________________________________________________  END OF LIGHT VALUE
//================================================================================================================================================================================ BLUETOOTH VALUE
#include <SoftwareSerial.h>
SoftwareSerial bluetooth(2, 3); // RX, TX
String readSerialString;
String readBluetoothString;
//___________________________________________________________________________________________________________________ end of BLUETOOTH VALUE
//================================================================================================================================================================================ EEPROM
void ReadFromEEPROM(byte AdressInEEPROM, byte& Variable)
{
    Wire.beginTransmission(0x50); Wire.write(AdressInEEPROM); Wire.endTransmission(); Wire.requestFrom(0x50,1); // request 1 byte from eeprom adress 0x50
    if (Wire.available()) {Variable = Wire.read();} delay(10);
}

void ReadIntegerFromEEPROM(byte AdressInEEPROM,int& Variable)  
{
	byte low; byte high;
	ReadFromEEPROM(AdressInEEPROM,low); 
	ReadFromEEPROM((AdressInEEPROM+1),high);

	Variable = ((high & 0x7f) << 7) | low;
}

void WriteToEEPROM(byte AdressInEEPROM, byte Variable) 
{ 
    Wire.beginTransmission(0x50); Wire.write(AdressInEEPROM); Wire.write(Variable); Wire.endTransmission(); delay(10);
}

void WriteIntegerToEEPROM(byte AdressInEEPROM, const int Variable)
{
	const unsigned char high = ((Variable >> 7) & 0x7f) | 0x80;
	const unsigned char low  = (Variable & 0x7f);

	WriteToEEPROM(AdressInEEPROM, low);
	WriteToEEPROM(AdressInEEPROM+1, high);
}
//___________________________________________________________________________________________________________________ end of BLUETOOTH EEPROM


//===================================================================================================================================================================================== SMARTUP
void Smartup()
{ 
  
  ReadFromEEPROM(ResetVariablesEEPROM, ResetVariables);
  if (ResetVariables != 42)
    {ResetDefaultValuesEEPROM();ResetVariables=42;WriteToEEPROM(ResetVariablesEEPROM, ResetVariables);}
     
  //----------------------------------------------------------------------------------------------------------------------------------------------------- LED
  //--------------------------------------- LED should be OFF 
  if ( (rtc[2] >  LED.hour_OFF) || (rtc[2]  <  LED.hour_ON) || ( (rtc[2] ==  LED.hour_OFF) && (rtc[1]  >= LED. minute_OFF)) || ((rtc[2] ==  LED.hour_ON) && (rtc[1] <  LED. minute_ON)) ) 
       {LED.status=0; LED.dimming_brightness=0; analogWrite(LED_Pin,LED.dimming_brightness); Light_delay();} // if light should be off   
       
       //--------------------------------------- LED should be SUNRISE
  else if (((LED. minute_ON+LED.dimming) < 60) && (rtc[2]==LED.hour_ON) && ((rtc[1] >=LED. minute_ON) && (rtc[1] < (LED. minute_ON+LED.dimming))) )
       {LED.status=1; LED.brightness_max=round((LED.brightness*255)/100); analogWrite(LED_Pin,LED.brightness_max); }
  
  else if  (((LED. minute_ON+LED.dimming) >= 60) && (rtc[2] == (LED.hour_ON)) && (rtc[1] >LED. minute_ON) )              
       {LED.status=1; LED.brightness_max=round((LED.brightness*255)/100); analogWrite(LED_Pin,LED.brightness_max);}  
  
  else if (((LED. minute_ON+LED.dimming) >= 60) && (rtc[2] == (LED.hour_ON+1)) && ((rtc[1] < ((LED. minute_ON+LED.dimming)-60))) )                 
       {LED.status=1; LED.brightness_max=round((LED.brightness*255)/100); analogWrite(LED_Pin,LED.brightness_max);}   
       
 //--------------------------------------- LED should be SUNSET
  else if  (((LED. minute_OFF+LED.dimming) < 60)  && (rtc[2]== LED.hour_OFF) && ((rtc[1] > (LED. minute_OFF)) && (rtc[1] < (LED. minute_OFF+LED.dimming))) )
      {LED.status=0; LED.dimming_brightness=0;  analogWrite(LED_Pin,LED.dimming_brightness); Light_delay();}
      
  else if  (((LED. minute_OFF+LED.dimming) >= 60) && (rtc[2]== LED.hour_OFF) && (rtc[1] > (LED. minute_OFF)))
      {LED.status=0; LED.dimming_brightness=0;  analogWrite(LED_Pin,LED.dimming_brightness); Light_delay();} 
      
  
  else if  (((LED. minute_OFF+LED.dimming) >= 60) && (rtc[2]== (LED.hour_OFF+1)) && ((rtc[1] < ((LED. minute_OFF+LED.dimming)-60))) )
      {LED.status=0; LED.dimming_brightness=0;  analogWrite(LED_Pin,LED.dimming_brightness); Light_delay();}      
  
  //--------------------------------------- LED should be ON
  else {LED.status=5; LED.dimming_brightness=0; // LED.hour_ON=rtc[2]; LED.minute_ON=rtc[1]+1;
        LED.brightness_max=round((LED.brightness*255)/100); LED.delay = round(60000/LED.brightness_max);} // if light should be on
  //_______________________________________________________________________ end of LED    
 }
//============================================================================================================================================================================== END OF SMARTUP
//================================================================================================================================================================================ LIGHT DELAY
void Light_delay()
{
  LED.brightness_max=round((LED.brightness*255)/100);
  LED.delay = round((LED.dimming*60000)/LED.brightness_max);
}
//============================================================================================================================================================================ END OF LIGHT DELAY

//================================================================================================================================================================================ END OF SETUP
void setup()
{
//----------------------------------------------------------------------------------------------------------------------------------------------------- LED setup
pinMode(LED_Pin, OUTPUT);
pinMode(13, OUTPUT);
digitalWrite(13,0);
//_____________________________________________________________________________________________________________________ end of LED setup  
RTC.get(rtc,true); // read current time for smartup functions
Serial.begin(9600);
bluetooth.begin(9600);
Smartup();
}
//================================================================================================================================================================================ END OF SETUP
//======================================================================================================================================================================================== LOOP 
void loop()
{  
Bluetooth_check();

 
  currentMillis = millis(); // save current millis
  //if (rtc[2]==0 && rtc[1]==0)  {Light_value();}
  
  if ((currentMillis - previousMillisSchedule) >= 60000) //------------------------------------------ check schedule events every minute 
     {previousMillisSchedule=currentMillis; RTC.get(rtc,true);   if (rtc[2]==LED.hour_ON && rtc[1] == LED.minute_ON  && LED.status==0)
     {Light_delay(); LED.dimming_brightness=0; LED.status=2;}

  if (rtc[2]==LED.hour_OFF && rtc[1] == LED.minute_OFF && LED.status==1)
     {Light_delay(); LED.dimming_brightness=LED.brightness_max; LED.status=3;}} // lights sunset ON 
   

  if (((LED.status==2) || (LED.status==3)|| (LED.status==5)) && ((millis() - previousMillisLED) >= LED.delay))
     {previousMillisLED=millis(); switch (LED.status)
       {case 2:  if(LED.dimming_brightness < LED.brightness_max) {LED.dimming_brightness++;}break;     
        case 3:  if(LED.dimming_brightness > 0) {LED.dimming_brightness--;}break;
        case 5:  if(LED.dimming_brightness < LED.brightness_max) {LED.dimming_brightness++;}break;}
        
      analogWrite(LED_Pin,LED.dimming_brightness);
      
      if(LED.status==2 && LED.dimming_brightness==LED.brightness_max){LED.status=1;}
      else if(LED.status==3 && LED.dimming_brightness==0){LED.status=0;}
      else if(LED.status==5 && LED.dimming_brightness==LED.brightness_max){LED.status=1;}}

}
//================================================================================================================================================================================= END OF LOOP
//================================================================================================================================================================================= CHANGE TIME
void Change_time(String message)
{
  int newMinute, newHour, newDate, newMonth, newYear;
  String tag;
  //char* buf = "1:90&2:80&3:180";
  const char* buf = message.c_str();
  
  
  int n = sscanf(buf, "%s;%d:%d;%d.%d.%d", &tag, &newHour, &newMinute, &newDate, &newMonth, &newYear);
  Serial.println(n);
  Serial.println(newMinute);
  Serial.println(newHour);
  Serial.println(newDate);
  Serial.println(newMonth);
  Serial.println(newYear);
  
  
/*  
  RTC.stop(); // stop the clock
  RTC.set(DS1307_SEC,0); // set new second
  RTC.set(DS1307_MIN,newMinute); // set new minutes
  RTC.set(DS1307_HR,newHour); // set new hour
//  RTC.set(DS1307_DOW,newDay); //set new day
  RTC.set(DS1307_DATE,newDate); // set new date
  RTC.set(DS1307_MTH,newMonth); // set new month
  RTC.set(DS1307_YR,newYear); // set new year
  RTC.start(); // start the clock
  */
}
//========================================================================================================================================================================== END OF CHANGE TIME
//=================================================================================================================================================================================== BLUETOOTH
void Bluetooth_check()
{ 
  
while (Serial.available()> 0) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    readSerialString += c;
 }  if (readSerialString.length() >0) {
    //Serial.print("Requested command: ");
    Serial.println(readSerialString);
    bluetooth.println(readSerialString);
    readSerialString="";
    delay(1000);
 }
  
  
  
  
// Read HM10 respond.
  while (bluetooth.available()> 0) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = bluetooth.read();
    readBluetoothString += c;
  } if (readBluetoothString.length() >0) {
    Serial.print("Android command: ");
    Serial.println(readBluetoothString);
   
    if (readBluetoothString == "inputs"){
      String ledStatus="";
      if (LED.status==0){ledStatus="led off";}
      else {ledStatus="led on";}
      char timeDate[27];  
      sprintf(timeDate,"%02d:%02d;%02d.%02d.%04d;%02d:%02d;%02d:%02d", rtc[2],rtc[1],rtc[4],rtc[5],rtc[6],LED.hour_ON,LED.minute_ON,LED.hour_OFF,LED.minute_OFF); 

      BluetoothSend(String("ArduinoOutputs")+';'+timeDate+';'+LED.dimming+';'+LED.brightness+';'+ledStatus);}
         
    else if (readBluetoothString == "turn led on"){analogWrite(LED_Pin,LED.brightness_max);BluetoothSend("led on");}
    else if (readBluetoothString == "turn led off"){analogWrite(LED_Pin,0);BluetoothSend("led off");}
    else if (readBluetoothString.indexOf("update time") > 0){Change_time(readBluetoothString);BluetoothSend("time changed");}
  
   
    readBluetoothString="";
    delay(1000);
  }
}  
void BluetoothSend(String message)
{ 
    Serial.println(message+'\n');
    delay(1000);
    bluetooth.print(message+'\n');
    delay(1000);
}  
// Send user input to HM10.


//============================================================================================================================================================================ END OF BLUETOOTH


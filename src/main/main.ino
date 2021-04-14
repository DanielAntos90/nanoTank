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
void Light_value()
{
LED.hour_ON=13;
LED.minute_ON=0;
LED.hour_OFF=20;
LED.minute_OFF=30;
LED.dimming=30;
LED.brightness=20; 
}
//============================================================================================================================================================================ END OF LIGHT VALUE

//===================================================================================================================================================================================== SMARTUP
void Smartup()
{ 
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
//_____________________________________________________________________________________________________________________ end of LED setup  
Light_value();
RTC.get(rtc,true); // read current time for smartup functions
//Serial.begin(9600);
//Change_time();
Smartup();
}
//================================================================================================================================================================================ END OF SETUP
//======================================================================================================================================================================================== LOOP 
void loop()
{  
//  char cas[9];  
  // zapíše do pole znaků cas hodnoty z rtc
//  sprintf(cas, "%02d:%02d:%02d", rtc[2],rtc[1],rtc[0]);  
//  Serial.println(cas); // odesle čas na ser. port  

  
  currentMillis = millis(); // save current millis
  if (rtc[2]==0 && rtc[1]==0)  {Light_value();}
  
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
void Change_time()
{
  RTC.stop(); // stop the clock
  RTC.set(DS1307_SEC,0); // set new second
  RTC.set(DS1307_MIN,54); // set new minutes
  RTC.set(DS1307_HR,20); // set new hour
/*  RTC.set(DS1307_DOW,current.day); //set new day
  RTC.set(DS1307_DATE,current.date); // set new date
  RTC.set(DS1307_MTH,current.month); // set new month
 RTC.set(DS1307_YR,current.year); // set new year
*/   RTC.start(); // start the clock
}
//========================================================================================================================================================================== END OF CHANGE TIME

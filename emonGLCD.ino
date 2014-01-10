//------------------------------------------------------------------------------------------------------------------------------------------------
// emonGLCD Implmementation
// This is a continuation from example project HomeEnergyMonitor
// which is part of https://github.com/openenergymonitor/EmonGLCD

// emonGLCD documentation http://openEnergyMonitor.org/emon/emonglcd

// GLCD library by Jean-Claude Wippler: JeeLabs.org
// 2010-05-28 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
//
// Author: Miloslav Fritz
// Licenced under GNU GPL V3

// THIS SKETCH REQUIRES:

// Libraries in the standard arduino libraries folder:
//
//	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
//	- JeeLib		https://github.com/jcw/jeelib
//	- RTClib		https://github.com/jcw/rtclib
//	- GLCD_ST7565		https://github.com/jcw/glcdlib
//
// Other files in project directory (should appear in the arduino tabs above)
//	- icons.ino
//	- templates.ino
//
//-------------------------------------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h>
#include <GLCD_ST7565.h>
#include <avr/pgmspace.h>
GLCD_ST7565 glcd;

#include <RTClib.h>                 // Real time clock (RTC) - used for software RTC to reset kWh counters at midnight
#include <Wire.h>                   // Part of Arduino libraries - needed for RTClib
RTC_Millis RTC;

//--------------------------------------------------------------------------------------------
// RFM12B Settings
//--------------------------------------------------------------------------------------------
#define MYNODE 20            // Should be unique on network, node ID 30 reserved for base station
#define freq RF12_868MHZ     // frequency - match to same frequency as RFM12B module (change to 868Mhz or 915Mhz if appropriate)
#define group 210 

unsigned long fast_update;

int dew_point1, dew_point2;

//---------------------------------------------------
// Data structures for transfering data between units
//---------------------------------------------------

typedef struct {int temp1, temp2, humidity, battery; } PayloadTH1;
PayloadTH1 emonth1;
typedef struct {int temp, humidity; } PayloadTH2;
PayloadTH2 emonth2;

const int greenLED=6;               // Green tri-color LED
const int redLED=9;                 // Red tri-color LED
const int LDRpin=4;    		    // analog pin of onboard lightsensor 
int cval_use;
byte GREEN;

// last hour average temp
int last_hour_avg;
// number of samples - needed for calculation of average
byte samples;

// history of values for past 24 hours
struct node *head;

int current_hour;

//-------------------------------------------------------------------------------------------- 
// Flow control
//-------------------------------------------------------------------------------------------- 
unsigned long last_emonbase, last_sensor1, last_sensor2;                   // Used to count time from last emontx update

//--------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------
void setup()
{
  delay(500); 				   //wait for power to settle before firing up the RF
  rf12_initialize(MYNODE, freq,group);
  delay(100);				   //wait for RF to settle befor turning on display
  glcd.begin(0x19);
  glcd.backLight(200);

  pinMode(greenLED, OUTPUT); 
  pinMode(redLED, OUTPUT);

  // Serial.begin(9600);

}

//--------------------------------------------------------------------------------------------
// Loop
//--------------------------------------------------------------------------------------------
void loop()
{

  if (rf12_recvDone())
  {
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)  // and no rf errors
    {
      int node_id = (rf12_hdr & 0x1F);
      
      if (node_id == 15)			//Assuming 15 is the emonBase node ID
      {
        RTC.adjust(DateTime(2013, 1, 1, rf12_data[1], rf12_data[2], rf12_data[3]));
        last_emonbase = millis();
        digitalWrite(greenLED, HIGH);
        GREEN = 1;
      } 
      else if (node_id == 19)   //19 is temperature node
      { 
        emonth1 = *(PayloadTH1*) rf12_data;
        last_sensor1 = millis();
        digitalWrite(greenLED, HIGH);
        GREEN = 1;
        dew_point1 = calculate_dew_point(emonth1.temp1, emonth1.humidity);
        // update average value
        if (last_hour_avg == 0)
          last_hour_avg = emonth1.temp1;
        else
        {
          last_hour_avg = (emonth1.temp1 + (samples * last_hour_avg))/(samples - 1);
          samples++;
        }
      }
      else if (node_id == 20)  // 20 is another temperature node
      { 
        emonth2 = *(PayloadTH2*) rf12_data;
        last_sensor2 = millis();
        digitalWrite(greenLED, HIGH);
        GREEN = 1;
        dew_point2 = calculate_dew_point(emonth2.temp, emonth2.humidity);
      }
    }
  }

  byte hour, minute;

  //--------------------------------------------------------------------------------------------
  // Display update every 200ms
  //--------------------------------------------------------------------------------------------
  if ((millis()-fast_update)>200)
  {
    fast_update = millis();
    
    DateTime now = RTC.now();
    hour = now.hour();
    minute = now.minute();

    draw_temp_page(emonth1.temp1, emonth1.humidity, dew_point1);
    draw_temperature_time_footer(emonth2.temp, emonth2.humidity, dew_point2, hour,minute, last_emonbase, last_sensor1, last_sensor2);
    draw_graph(head);
    glcd.refresh();
    
    if (((hour > 22) ||  (hour < 5)) && last_emonbase != 0)
    {
      glcd.backLight(0);
    }
    else 
    {
      int LDR = analogRead(LDRpin);                     // Read the LDR Value so we can work out the light level in the room.
      int LDRbacklight = map(LDR, 0, 1023, 50, 250);    // Map the data from the LDR from 0-1023 (Max seen 1000) to var GLCDbrightness min/max
      LDRbacklight = constrain(LDRbacklight, 0, 255);   // Constrain the value to make sure its a PWM value 0-255
      cval_use = cval_use + (LDRbacklight - cval_use)>>1;        //smooth transitions of brightness
      glcd.backLight(cval_use);
    }
    
    if (GREEN == 1)
    {
      digitalWrite(greenLED, LOW);
      GREEN = 0;
    }

   
  
    // update every hour - add last hour average
    if (last_emonbase != 0 && current_hour != hour)
    {
      current_hour = hour;
  
      if (last_sensor1 != 0) {
        // reset the averaging
        samples = 0;
        
        if (head == NULL)
          head = list_new(last_hour_avg);
        else
          head = list_insert_max(last_hour_avg, 24, head);
        
        last_hour_avg = 0;
      }
  
    }
  }
}

int calculate_dew_point(int temp, int hum)
{
  float humf = hum / 10.0;
  float tempf = temp / 10.0;
  
  return int((pow(humf/100, 0.125) * (112 + 0.9 * tempf) + 0.1 * tempf - 112)*10);
}

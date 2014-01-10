#include "utility/font_metric04.h"
#include "utility/font_metric02.h"
#include "utility/font_metric01.h"


void draw_temp_page(int temp, int hum, int dew)
{
  glcd.clear();
  
  char str[10];
  glcd.setFont(font_metric04);
  dtostrf(temp/10.0,0,1,str);
  strcat(str,"*");
  glcd.drawString(0,15, str);
  
  glcd.setFont(font_metric02);
  glcd.drawString_P(0,0, PSTR("TEPLOTA"));
  
  itoa(hum/10,str, 10);
  strcat(str, "%");
  glcd.drawString(102,15,str);
  glcd.drawString_P(75,15,PSTR("HUM"));
  
  itoa(dew/10,str, 10);
  strcat(str,"*");
  glcd.drawString(102,29,str);
  glcd.drawString_P(75,29,PSTR("DEW"));
    
}


//------------------------------------------------------------------
// Draws a footer showing sensor2 temp, humidity, and calculated dew point and clock
//------------------------------------------------------------------
void draw_temperature_time_footer(int temp, int hum, int dew, int hour, int minute, unsigned long last_base, unsigned long last_1, unsigned long last_2)
{
  glcd.drawLine(0, 47, 128, 47, WHITE);     //middle horizontal line 

  char str[10];
  // Draw Temperature
  glcd.setFont(font_metric02);
  itoa((int)temp/10, str, 10);
  strcat(str,"*");
  glcd.drawString(0,50,str);  
  
  // Humidity and dew point
  glcd.setFont(font_metric01);
  dtostrf(hum/10.0,0,1,str);
  strcat(str,"%");
  glcd.drawString_P(26,50,PSTR("HUM"));
  glcd.drawString(42,50,str);
               
  dtostrf(dew/10.0,0,1,str);
  strcat(str,"*");
  glcd.drawString_P(26,57,PSTR("DEW"));
  glcd.drawString(42,57,str);
  
  // Time
  char str2[5];
  char *s = str;
  if (hour<10)
  { 
    str[0] = '0';
    s++;
  }    
  itoa(hour,s,10);
  if  (minute<10) strcat(str,":0"); else strcat(str,":");
  itoa(minute,str2,10);
  strcat(str,str2); 
  glcd.setFont(font_metric02);
  glcd.drawString(87,50,str);
  
  // write '!TXB' in case nothing comes from base station in 10 minutes
  if (last_base == 0 || (millis() - last_base) > 600000)
  {
    glcd.setFont(font_metric01);
    glcd.drawString_P(65,57,PSTR("!TXB"));
  }
  
  // write '!TX1[2]' in case nothing comes from tx station is 5.5 minutes
  strcpy(str, "!TX");
  boolean w = false;
  if (last_1 == 0 || (millis() - last_1) > 350000)
  {
    strcat(str, "1");
    w = true;
  }  
  if (last_2 == 0 || (millis() - last_2) > 350000)
  {
    strcat(str, "2");
    w = true;
  }
  if (w)
  {
    glcd.setFont(font_metric01);
    glcd.drawString(65,50, str);
  }
}

//------------------------------------------------------------------
// Draws a graph with history values
//------------------------------------------------------------------
void draw_graph(struct node* list)
{
  // first get min/max from all values
  int min = 10000, max = 0;

  struct node *curr = list;
  while (curr->next)
  {
    if (curr->value < min) min = curr->value;
    if (curr->value > max) max = curr->value;
    curr = curr->next;
  }

  curr = list;
  int i = 0;
  while (curr->next)
  {
    // calculate the height of the bar
    byte h = map(curr->value, min, max, 1, 12);
    h = constrain(h, 1, 12);
    glcd.drawRect(56+(i*3+1), 12-h, 2, h, WHITE);
    
    curr = curr->next;
    i++;
  }
}
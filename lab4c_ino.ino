// Farhan Ali - mfa0092
// Lab 4
// Due 04 08 2020

#include "Energia.h" // Core library for code-sense
#include "SPI.h" // Include application, user and local libraries
#define HX8353E 
#include "Screen_HX8353E.h"
Screen_HX8353E myScreen;

#include<stdio.h> 
#include<string.h>

// Pin Map
const int LEDG = 38;            // Green LED pin
const int B_ONE = 33;           // pushbutton one
const int B_TWO = 32;           // pushbutton two
const int xpin = 23;            // x-axis of the accelerometer
const int ypin = 24;            // y-axis
const int zpin = 25;            // z-axis (only on 3-axis models)
const int buzzer = 40;          // Buzzer
// Variables
int ledStateGreen = LOW;        // Green LED state
int seconds = 0;                // Seconds counter
int minutes = 0;
int hours = 12;
int AM_PM = 0;                  // AM = 0, PM = 1
String time_of_day;
char buffer[12];
unsigned int freq1 = 500;       // First Frequency
unsigned int freq2 = 550;       // Alternative Frequency
unsigned long timeout_arrayX[5];
unsigned long timeout_arrayY[5];
unsigned long timeout_arrayZ[5];
unsigned long timeout_sumX = 0;
unsigned long timeout_sumY = 0;
unsigned long timeout_sumZ = 0;
unsigned long timeout_avgX = 0;
unsigned long timeout_avgY = 0;
unsigned long timeout_avgZ = 0;
unsigned long prevX = 0;
unsigned long prevY = 0;
unsigned long prevZ = 0;
int i = 0;                       // array index
// Time based
unsigned long previousMillis_second = 0;
unsigned long previousMillis_blink = 0;
unsigned long previousMillis_Interupt = 0;
unsigned long currentMillis; 
unsigned long interval_second = 1000;   // Tick, 1 second
unsigned long interval_blink = 500;     // half a second
unsigned long interval_Interupt = 500;  // locks out new interrupt for half second
 
// Flags
int FLAG_MINUTE = 0;
int ISRH = 0;
int ISRM = 0;
int TIMEOUT_FLAG = 1;                   // Screen starts off
int MOVE_FLAG = 0;

// Hours Incrementing Function returns void
void Hours_Increment()
{
  if(minutes > 59 && FLAG_MINUTE == 0){minutes=0;hours++;}
  else{FLAG_MINUTE =0;}
  if(hours > 12){myScreen.clear(blackColour);hours=1;} 
} 
// Minutes Incrementing Function returns void
void Minutes_Increment()
{
  if(seconds > 59){ seconds=0; minutes++;}
}
// Seconds Incrementing Function returns void
void Seconds_Increment()
{
  seconds++;
} 
void AM_or_PM()
{
  if(hours == 12 && seconds == 0){AM_PM = !AM_PM;}
  
  if (AM_PM == 0){time_of_day = " AM";}
   else{time_of_day = " PM";}
}

// Blink function, void return
void blinkLed(const int &ledPin,int &ledState) 
{
              // if the LED is off turn it on and vice-versa:
              if (ledState == LOW)
              {
                ledState = HIGH;
              }
              else 
              {
                ledState = LOW;
              }
              
              // set the LED with the ledState of the variable:
              digitalWrite(ledPin, ledState);
 }
 void Acc_Average()
 {  
           if(i <= 4)
           {
                timeout_arrayX[i] = analogRead(xpin)-2048;
                timeout_sumX += timeout_arrayX[i];
                timeout_arrayY[i] = analogRead(ypin)-2048;
                timeout_sumY += timeout_arrayY[i];
                timeout_arrayZ[i] = analogRead(zpin)-2048;
                timeout_sumZ += timeout_arrayZ[i];
                i++;
           }  
                if(i == 4)
                {
                  timeout_avgX = timeout_sumX/(i+1);
                  timeout_avgY = timeout_sumY/(i+1);
                  timeout_avgZ = timeout_sumZ/(i+1);
                 
                }// resets i back to 0 after taking average, each iteration of i is one second
                
 }
 void Acc_Previous()
 {
  prevX = analogRead(xpin)-2048;
  prevY = analogRead(ypin)-2048;
  prevZ = analogRead(zpin)-2048;
 }
 void reset_Count()
 {
      timeout_sumX = 0;
      timeout_sumY = 0;
      timeout_sumZ = 0;
      timeout_avgX = 0;
      timeout_avgY = 0;
      timeout_avgZ = 0;
      i = -1;
 }
 
 void Display()
 {
   if(TIMEOUT_FLAG == 0){
     snprintf(buffer, sizeof(buffer),"%d:%02d:%02d" , hours, minutes, seconds); // Converts PrintF into a String
     myScreen.gText(32,54,buffer + time_of_day); }
   else{ myScreen.clear(blackColour);}
 }
 void Sleep()
 {  
   if(
       TIMEOUT_FLAG == 0 &&
       ((timeout_avgX <= (timeout_arrayX[0] + 10)) ||  (timeout_avgX >= (timeout_arrayX[0] - 10))) &&
       ((timeout_avgY <= (timeout_arrayY[0] + 10)) ||  (timeout_avgY >= (timeout_arrayY[0] - 10))) &&
       ((timeout_avgZ <= (timeout_arrayZ[0] + 20)) ||  (timeout_avgZ >= (timeout_arrayZ[0] - 20))) &&
        i == 4)
        { 
           TIMEOUT_FLAG = 1;
           i=-1;
        }
 }
 void Lift()
 {
   Serial.print(TIMEOUT_FLAG);
    if(TIMEOUT_FLAG == 1 && MOVE_FLAG == 0 && (((analogRead(ypin) - 2048 ) <= -325)))   //|| ((analogRead(ypin) - 2048) <= -350 )))
                {      
                      TIMEOUT_FLAG = 0;
                      
                }
 }
 void AnyMovement()
 {
     if(TIMEOUT_FLAG == 0 && (prevX > ((analogRead(xpin) - 2048 ) + 10)) || (prevX < ((analogRead(xpin) - 2048 ) - 10)) &&
             (prevY > ((analogRead(ypin) - 2048 ) + 10)) || (prevY < ((analogRead(ypin) - 2048 ) - 10))&&
             (prevZ > ((analogRead(zpin) - 2048 ) + 10)) || (prevZ < ((analogRead(zpin) - 2048 ) - 10)))
             {   
               reset_Count();
               MOVE_FLAG = 0;
             } 
             else{ MOVE_FLAG = 1;}
 }
 void ISR_hour(){ISRH = 1;}
 void Hour_ISR_Execution()
 {
   if(ISRH == 1){TIMEOUT_FLAG = 0;reset_Count(); hours++; if(hours == 12){AM_PM = !AM_PM;}
   tone(buzzer, freq1 , 100);tone(buzzer, freq2 , 100);ISRH=0;}
 }
 void ISR_min(){ISRM = 1;}
 void Min_ISR_Execution()
 {
    if(ISRM == 1){TIMEOUT_FLAG = 0; reset_Count(); minutes++;tone(buzzer, freq1 , 100);tone(buzzer, freq2 , 100);
              if(minutes > 59){minutes=0; FLAG_MINUTE = 1;} ISRM=0;}
 }
void setup()
{
  Serial.begin(9600);
  // put your setup code here, to run once:
  myScreen.begin();
  myScreen.setOrientation(0);
  
  analogReadResolution(12); // resolution to 12 bits
  
    // initialize the pushbutton pin as an input:
  pinMode(LEDG, OUTPUT);  
  
  pinMode(B_ONE, INPUT_PULLUP);     
  pinMode(B_TWO, INPUT_PULLUP); 
  
  attachInterrupt(B_ONE, ISR_hour, LOW);
  attachInterrupt(B_TWO, ISR_min, LOW);
  
}

void loop()
{
  currentMillis = millis();
  
     // Interupts w/ lockout timer
     if(
     currentMillis - previousMillis_Interupt >=  interval_Interupt) // Lockout timer to avoid new interrupts, like a debouncer for half second, will register one press per half second
     { 
            previousMillis_Interupt = currentMillis;
           
            Hour_ISR_Execution();    // Executes the ISR Routine from the Flag
            Min_ISR_Execution();     // Executes the ISR Routine from the Flag
           
     }
      // main loop
         
         if(currentMillis - previousMillis_blink >= interval_blink) // How long led should be on or off, each cycle is 500ms or half a second
          {
              previousMillis_blink = currentMillis;
              blinkLed(LEDG,ledStateGreen); 
          }
         if(currentMillis - previousMillis_second >=  interval_second ) // Clock Tick is 1 Second
         { 
                previousMillis_second = currentMillis;
           
                Seconds_Increment();  // Increments Seconds
                Minutes_Increment();  // Increments Minutes
                Hours_Increment();    // Increments Hour
                AM_or_PM();           // Time of Day
                Acc_Average();        // Computes Average movement over 5 Seconds
                AnyMovement();        // Sensing any movement
                Lift();               // Device has been lifted at 45 degrees            
                Sleep();              // Sleep Timeout
                Acc_Previous();       // Stores previous reading
                Display();            // Display ON/OFF
               
         }
        
           
           

  
}

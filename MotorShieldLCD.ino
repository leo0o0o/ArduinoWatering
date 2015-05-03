
#include <LiquidCrystal.h>

unsigned long Watch, _micro, time = micros();
unsigned long Clock = 0, R_clock;
boolean Reset = false, Stop = false, Paused = false;
volatile boolean timeFlag = false;
boolean restart=false;

//time for watering
int secondsForWatering = 5;
int vectorSize = 2;
int P1hoursTimer[] = {24, 24, 24};
int P1minutesTimer[] = {0, 0, 0};
int P1secondsTimer[] = {0, 0, 0};
int P1wateringDuration[] = {12,12,12};

int P2hoursTimer[] = {48, 48, 48};
int P2minutesTimer[] = {0, 0, 0};
int P2secondsTimer[] = {0, 0, 0};
int P2wateringDuration[] = {15,15,15};

int programActive = 2;


int index = 0;
boolean isWatering = false;

LiquidCrystal lcd(12, 11, 5, 4, 6, 2);

int HourPlusButtonState = 0; 
int HourMinusButtonState = 0; 
int ProgramButtonState = 0; 

/*
Function	Channel A	Channel B
Direction	Digital 12	Digital 13
Speed (PWM)	Digital 3	Digital 11
Brake	        Digital 9	Digital 8
Current Sensing	Analog 0	Analog 1
*/

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  startCountdown();  
  initializeMotors();
}

void loop()
{
  CountDownTimer(); // run the timer
    
  // this prevents the time from being constantly shown.
  if (TimeHasChanged()) 
  {
      HourPlusButtonState = digitalRead(7);
      HourMinusButtonState = digitalRead(13);
  
      // check if the pushbutton is pressed.
      // if it is, the buttonState is HIGH:
      if (HourPlusButtonState == HIGH && HourMinusButtonState == LOW) {     
        int hours = ShowHours();        
        hours++;
        SetTimer(hours, ShowMinutes(), ShowSeconds());
      } 
      
      if (HourMinusButtonState == HIGH && HourPlusButtonState == LOW) {     
        int hours = ShowHours();
        int minutes = ShowMinutes();
        if(hours>0){
          hours--;
        }
        else {
          if(minutes>10){
           minutes = minutes - 10;
          }
        }
        SetTimer(hours,minutes, ShowSeconds());
      }
     
     if (HourMinusButtonState == HIGH && HourPlusButtonState == HIGH) {
       if(programActive==1) {
         programActive=2;
       }
       else {
         programActive = 1;         
       }
       index=0;
       startCountdown();  
     } 
     
     if(isWatering!=true && Stop!=true){
       analogWrite(3, 0);   
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print(ShowHours());
       lcd.print("h:");
       lcd.print(ShowMinutes());
       lcd.print("m:");
       lcd.print(ShowSeconds());
       lcd.print("s");
       lcd.setCursor(0,1);
       lcd.print("Program: ");
       lcd.print(programActive);
     }
     else if(Stop==true){
       if(isWatering==false){
         lcd.clear();
         lcd.setCursor(0,1);
         lcd.print("Watering...");
         isWatering = true;
         analogWrite(3, 255);   
         SetTimer(0,0,20);
         StartTimer();
         StopTimerAt(0,0,0);
       }
       else {
         //Stop the motor and change direction to avoid continuous spin
          analogWrite(3, 0); 
          delay(1000);  
          digitalWrite(12, LOW); //Establishes forward direction of Channel A
          analogWrite(3, 255);   
          delay(5000);
          analogWrite(3, 0); 
          digitalWrite(12, HIGH); //Establishes forward direction of Channel A
          
          isWatering = false;
          lcd.setCursor(0,1);
          //Watering done. It's time to reset the Timer
          index++;
          //check if a day is passed
          if(index>=vectorSize){
            index=0;
          }
          startCountdown();
          Serial.println(ShowHours());
       }
     }
    
  }
  
}


void initializeMotors(){
   //Setup Channel A
   pinMode(12, OUTPUT); //Initiates Motor Channel A pin
   pinMode(9, OUTPUT); //Initiates Brake Channel A pin
  
   digitalWrite(12, HIGH); //Establishes forward direction of Channel A
   digitalWrite(9, LOW);   //Disengage the Brake for Channel A
   analogWrite(3, 0);   //Spins the motor on Channel A at full speed
}

void startCountdown(){
  if(programActive==1){
    SetTimer(P1hoursTimer[index],P1minutesTimer[index],P1secondsTimer[index]); 
  }
  else if(programActive==2){
    SetTimer(P2hoursTimer[index],P2minutesTimer[index],P2secondsTimer[index]); 
  }
  StartTimer();
  StopTimerAt(0,0,0);
}

boolean CountDownTimer()
{
  static unsigned long duration = 1000000; // 1 second
  timeFlag = false;

  if (!Stop && !Paused) // if not Stopped or Paused, run timer
  {
    // check the time difference and see if 1 second has elapsed
    if ((_micro = micros()) - time > duration ) 
    {
      Clock--;
      timeFlag = true;

      if (Clock == 0) // check to see if the clock is 0
        Stop = true; // If so, stop the timer

     // check to see if micros() has rolled over, if not,
     // then increment "time" by duration
      _micro < time ? time = _micro : time += duration; 
    }
  }
  return !Stop; // return the state of the timer
}

void ResetTimer()
{
  SetTimer(R_clock);
  Stop = false;
}

void StartTimer()
{
  Watch = micros(); // get the initial microseconds at the start of the timer
  Stop = false;
  Paused = false;
}

void StopTimer()
{
  Stop = true;
}

void StopTimerAt(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
  if (TimeCheck(hours, minutes, seconds) )
    Stop = true;
}

void PauseTimer()
{
  Paused = true;
}

void ResumeTimer() // You can resume the timer if you ever stop it.
{
  Paused = false;
}

void SetTimer(unsigned long hours, unsigned int minutes, unsigned int seconds)
{
  // This handles invalid time overflow ie 1(H), 0(M), 120(S) -> 1, 2, 0
  unsigned int _S = (seconds / 60), _M = (minutes / 60);
  if(_S) minutes += _S;
  if(_M) hours += _M;
  
  Clock = (hours * 3600L) + (minutes * 60) + (seconds % 60);
  
  
  R_clock = Clock;
  Stop = false;
}

void SetTimer(unsigned int seconds)
{
 // StartTimer(seconds / 3600, (seconds / 3600) / 60, seconds % 60);
 Clock = seconds;
 R_clock = Clock;
 Stop = false;
}

int ShowHours()
{
  return Clock / 3600;
}

int ShowMinutes()
{
  return (Clock / 60) % 60;
}

int ShowSeconds()
{
  return Clock % 60;
}

unsigned long ShowMilliSeconds()
{
  return (_micro - Watch)/ 1000.0;
}

unsigned long ShowMicroSeconds()
{
  return _micro - Watch;
}

boolean TimeHasChanged()
{
  return timeFlag;
}

// output true if timer equals requested time
boolean TimeCheck(unsigned int hours, unsigned int minutes, unsigned int seconds) 
{
  return (hours == ShowHours() && minutes == ShowMinutes() && seconds == ShowSeconds());
}

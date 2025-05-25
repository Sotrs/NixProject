#include <DS3231.h>

#include <Adafruit_MCP23X17.h>
#include <DS3231.h> // by Andrew Wickert
#include <Wire.h>

//COM 8 Is the desktop nixie
// IMPORTANT DURING ACTUAL IMPLEMENTATION THE FIRST BIT 
// THE MOST RIGHT BIT SHOULD BE CONNECTED TO THE LAST OUTPUT OF EACH DIGIT
//!!!! ONE THE DECODER 7441 THE FIRST BIT IS INPUT A
// HOURS1 THE FIRST BIT IS OUT5->INPUT A
// HOURS2 THE FIRST BIT IS OUT9->INPUT A
// MINS1 THE FIRST BIT IS OUT13->INPUT A
// MINS2 THE FIRST BIT IS OUT17 BUT I MAY CHANGE IT TO IMPLIMENT OUTPUT EXTENDER FOR SECONDS
// SECS1 THE FIRST BIT IS IDK BECAUSE I WILL IMPLIMENT OUTPUT EXTENDER
// SAME FOR SECS2

Adafruit_MCP23X17 mcp;
const int Hours1[4] = {2,3,4,5};  // First digit ABCD INPUTS IN DECODER, IN REALITY IT IS DCBA(Binary starting from the right) AS IT IS WRITTEN IN THE ARRAY
const int Hours2[4] = {6,7,8,9};  // Second Digit
const int Mins1[4] = {10,11,12,13}; // Third Digit
const int Mins2[4] = {14,15,16,17}; // Fourth Digit
const int Secs1[4] = {8,9,10,11}; //MCP PINS 8,9,10,11
const int Secs2[4] = {12,13,14,15}; //MCP PINS 12,13,14,15

byte Hours; //will hold the decimal value of time for the RTC communication
byte Minutes;
byte Seconds;
DS3231 myRTC; //rtc object 

bool century = false;
bool h12 = false;
bool hPM;

int CurrentMode = 0; //0 PRINT TIME(Running Mode), 1 SET TIME, 2 DIGIT INTEGRITY TESTING, 3 Number Animation/Cathode cure, 4 EXPERIMENTAL TESTING <-------------------------------------------------------------------
bool SetScreenFlag = true;

int TimeButton = 5; //Set time button D2->2 | ON MCP TimeButton is Pin 5(Physical 26)
int ButtonState;
int TimeButtonCount = -1;
int LastButtonState = LOW;
unsigned long LastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long DebounceDelay = 50;    // In milliseconds the debounce time | increase if the output flickers 


int SetButton = 6 ;//Set Button D3->3 | ON MCP IT SetButton is Pin 6(Physical 27)
int SetButtonState;
int SetButtonCount = 0;
int LastSetButtonState = LOW;
unsigned long LastSetDebounceTime = 0;

byte BCD[10][4] = { // D,C,B,A Binary written as in theory for easier understanding
  {0,0,0,0}, //binary 0
  {0,0,0,1}, //1
  {0,0,1,0}, //2
  {0,0,1,1}, //3
  {0,1,0,0}, //4
  {0,1,0,1}, //5
  {0,1,1,0}, //6
  {0,1,1,1}, //7
  {1,0,0,0}, //8
  {1,0,0,1} //9
};

byte DIM[1][4] ={ {1,1,1,1} }; // Over limit number 15

void CathodeCure(){ //==========================================================Cathode Curing Function===========================================================================
  uint8_t digits1[]={0,1,2,3,4,5,6,7,8,9}; // One array for each DECIMAL digit
  uint8_t digits2[]={0,1,2,3,4,5,6,7,8,9}; //As each digit will display a different randomised number
  uint8_t digits3[]={0,1,2,3,4,5,6,7,8,9}; //This means 6 different picks,
  uint8_t digits4[]={0,1,2,3,4,5,6,7,8,9};
  uint8_t digits5[]={0,1,2,3,4,5,6,7,8,9};
  uint8_t digits6[]={0,1,2,3,4,5,6,7,8,9}; //last pick
  uint8_t array_size = sizeof(digits1) / sizeof(digits1[0]); //initial array size, all get reduced equally

  while(array_size){ //while array exists(has at least one input)
    TimeButtonRead(TimeButton); //Keep reading input while clock is running
    uint8_t pick1 = random(0, array_size);
    uint8_t pick2 = random(0, array_size);
    uint8_t pick3 = random(0, array_size);
    uint8_t pick4 = random(0, array_size);
    uint8_t pick5 = random(0, array_size);
    uint8_t pick6 = random(0, array_size);
    int currentDigit1 = digits1[pick1];
    int currentDigit2 = digits2[pick2]; 
    int currentDigit3 = digits3[pick3];
    int currentDigit4 = digits4[pick4];
    int currentDigit5 = digits5[pick5];
    int currentDigit6 = digits6[pick6];
    
    Serial.print(currentDigit1);
    Serial.print("  ");
    Serial.print(currentDigit2);
    Serial.print("  ");
    Serial.print(currentDigit3);
    Serial.print("  ");
    Serial.print(currentDigit4);
    Serial.print("  ");
    Serial.print(currentDigit5);
    Serial.print("  ");
    Serial.println(currentDigit6);
    
    for(int i = 0; i<4; i++){
      digitalWrite(Hours1[i],BCD[currentDigit1][i]);
      digitalWrite(Hours2[i],BCD[currentDigit2][i]);
      digitalWrite(Mins1[i],BCD[currentDigit3][i]);
      digitalWrite(Mins2[i],BCD[currentDigit4][i]);
      mcp.digitalWrite(Secs1[i],BCD[currentDigit5][i]);
      mcp.digitalWrite(Secs2[i],BCD[currentDigit6][i]);
    }

  delay(500); //Change this delay to increase or decrease the appear time of each digit
  
  digits1[pick1] = digits1[array_size - 1]; // The last digit takes the currents numbers position before it is erased
  digits2[pick2] = digits2[array_size - 1];
  digits3[pick3] = digits3[array_size - 1];
  digits4[pick4] = digits4[array_size - 1];
  digits5[pick5] = digits5[array_size - 1];
  digits6[pick6] = digits6[array_size - 1];
  array_size--; //Reduce the array by erasing the last digit
  }

 Serial.println("----------------------------------"); 
}

void TimeButtonRead(int Button){ //=================================================TimeButtonRead Reading Function=========================================================
  int reading = mcp.digitalRead(Button);
  
  if(reading != LastButtonState){ //Gets the input from pressing or noise
    LastDebounceTime = millis();   //Resets the Debouncing timer
  }

  if ((millis() - LastDebounceTime) > DebounceDelay) { //If that time is longer than our Preset "Real button press" delay
                                                        // Then it must be a true button press
    if (reading != ButtonState) {
      ButtonState = reading;
      if(ButtonState == LOW){ //Button Pressed Succesfully Do things HERE   
        CurrentMode=1; //Time setting mode
        TimeButtonCount++; //Time button count is the actual number the user wants by each press you increase it which is also the digit on the clock.
        if(SetButtonCount==4){ //In combination with SetDigit() when counting seconds pressing the button you only reset them to 0
          myRTC.setSecond(0);  // reset seconds to 0 on button press
        }
        //Serial.println (TimeButtonCount);
      } //Button is pressed
    } //REAL Button Press
  }
  LastButtonState = reading;
}

void SetButtonRead(int Button){ //=================================================SetButtonRead Reading Function=========================================================
  int reading = mcp.digitalRead(Button);
  
  if(reading != LastSetButtonState){ //Gets the input from pressing or noise
    LastSetDebounceTime = millis();   //Resets the Debouncing timer
  }

  if ((millis() - LastSetDebounceTime) > DebounceDelay) { //If that time is longer than our Preset "Real button press" delay
                                                        // Then it must be a true button press
    if (reading != SetButtonState) {
      SetButtonState = reading;
      if(SetButtonState == LOW){ //Button Pressed Succesfully Do things HERE   ;
        SetButtonCount++;
        //Serial.println (SetButtonCount);
      }//Button is pressed
    }//REAL Button Press
  } 
  LastSetButtonState = reading;
}

void SetDigit(){ //================================================================== SET DIGIT ===================================================================================
      while (SetButtonCount == 0){ //Set Hour1 Digit Hour one has 0,1,2 No Other digits
        //BlinkDots();
        TimeButtonRead(TimeButton);
        SetButtonRead(SetButton);
        if(TimeButtonCount>2){ //timebutton count will keep track of 0,1,2 digits
          TimeButtonCount = 0;
        }
        Serial.println (TimeButtonCount);
        for(int i = 0; i<4; i++){ //This now writes the same time to all DIGITS, we can use the second button to differentiate them
          digitalWrite(Hours1[i],BCD[TimeButtonCount][i]);
        }   
      }
      Hours = TimeButtonCount*10; //First digit is in the 10's After set button is pressed

      TimeButtonCount= 0; //Reset Time button
      while (SetButtonCount == 1){ //Set Hour2 Digit Hour one has 0,1,2,3,4,5,6,7,8,9 Digits
        //BlinkDots();
        TimeButtonRead(TimeButton);
        SetButtonRead(SetButton);
        if(TimeButtonCount>9){
          TimeButtonCount= 0;
        }
        Serial.println (TimeButtonCount);             
        for(int i = 0; i<4; i++){ //This now writes the same time to all DIGITS, we can use the second button to differentiate them
          digitalWrite(Hours2[i],BCD[TimeButtonCount][i]);
        }   
      }
      Hours += TimeButtonCount;// Second Digit is in the 1's
      myRTC.setHour(Hours); //Set Final Hours

      TimeButtonCount= 0; //Reset Time button
      while (SetButtonCount == 2){ //Set Min1 Digit Hour one has 0,1,2,3,4,5 Digits
        //BlinkDots();
        TimeButtonRead(TimeButton);
        SetButtonRead(SetButton);
        if(TimeButtonCount>5){
          TimeButtonCount= 0;
        }
        Serial.println (TimeButtonCount);
        for(int i = 0; i<4; i++){ //This now writes the same time to all DIGITS, we can use the second button to differentiate them
          digitalWrite(Mins1[i],BCD[TimeButtonCount][i]);
        }   
      }
      Minutes = TimeButtonCount*10; //Multiplying by 10 to place it into the First Digit

      TimeButtonCount = 0; //Reset Time button
      while (SetButtonCount == 3){ //Set Min2 Digit Hour one has 0,1,2,3,4,5,6,7,8,9 Digits
        //BlinkDots();
        TimeButtonRead(TimeButton);
        SetButtonRead(SetButton);
        if(TimeButtonCount>9){
          TimeButtonCount= 0;
        }
        Serial.println (TimeButtonCount);
        for(int i = 0; i<4; i++){ //This now writes the same time to all DIGITS, we can use the second button to differentiate them
          digitalWrite(Mins2[i],BCD[TimeButtonCount][i]);
        }   
      }
      Minutes += TimeButtonCount; //And then adding the second Digit 
      myRTC.setMinute(Minutes);

      while (SetButtonCount == 4){ //Resetting seconds
        //BlinkDots();
        Seconds = myRTC.getSecond();
        TimeButtonRead(TimeButton);
        SetButtonRead(SetButton);
        if(TimeButtonCount>9){
          TimeButtonCount= 0;
        }
        for(int i = 0; i<4; i++){ 
          mcp.digitalWrite(Secs1[i],BCD[Seconds/10][i]);
          mcp.digitalWrite(Secs2[i],BCD[Seconds%10][i]);
        }
        Serial.println (Seconds); //DIGITAL WRITE HERE
      }

      if(SetButtonCount > 4){ //Time Set succesfully So going back to counting time and reseting the SetButton, TimeButton and Current Mode
        CurrentMode = 0; //Count Time Mode
        TimeButtonCount = -1; //Reset Time button
        SetScreenFlag=true; //Will make everything zero at first press
        SetButtonCount = 0; //Reset Set Button
      }

}//SetDigit Function END

void WriteTime(){ //========================================================================= WRITE TIME ================================================================================
    if(myRTC.getMinute() % 30 == 0 && Seconds==0){ //Animation every 10 mins
    for(int i=0;i<6;i++)
      CathodeCure();
  }

  if(((myRTC.getMinute() % 5 == 0)  || (myRTC.getMinute() % 10 == 0)) && Seconds==0){ //Animation every 10 mins
    NumberAnimation();
  }

  Hours = myRTC.getHour(h12, hPM);
  Minutes = myRTC.getMinute();  
  Seconds = myRTC.getSecond();
  for(int i = 0; i<4; i++){
    digitalWrite(Hours1[i],BCD[Hours/10][i]);
    digitalWrite(Hours2[i],BCD[Hours%10][i]);
    digitalWrite(Mins1[i],BCD[Minutes/10][i]);
    digitalWrite(Mins2[i],BCD[Minutes%10][i]);
    mcp.digitalWrite(Secs1[i],BCD[Seconds/10][i]); 
    mcp.digitalWrite(Secs2[i],BCD[Seconds%10][i]);
  }

  /*  //DIMMING EXPERIMENT
    for(int i = 0; i<4; i++){ 
    digitalWrite(Hours1[i],DIM[0][i]);
    digitalWrite(Hours2[i],DIM[0][i]);
    digitalWrite(Mins1[i],DIM[0][i]);
    digitalWrite(Mins2[i],DIM[0][i]);
    mcp.digitalWrite(Secs1[i],DIM[0][i]); 
    mcp.digitalWrite(Secs2[i],DIM[0][i]);
  }*/

}//WriteTime Function END



void NumberAnimation(){ //=============================================================== NUMBER ANIMATION ================================================================================
  uint8_t pick1; //THIS PART IS RETARED AS FUCK BUT WORKS...
  uint8_t pick2;
  uint8_t pick3;
  uint8_t pick4; 
  uint8_t pick5; 
  uint8_t pick6; 
  int AniArr[] = {0,0,0,0,0,0}; //Amount of nuber rolls by a set value
  int del=40; //delay time

  while(AniArr[0]<18){ //ALL digits rolling eg. 18 numbers for the first
    pick1 = random(0,9);
    pick2 = random(0,9);
    pick3 = random(0,9);
    pick4 = random(0,9);
    pick5 = random(0,9);
    pick6 = random(0,9);

    for(int i = 0; i<6; i++){
      digitalWrite(Hours1[i],BCD[pick1][i]);
      digitalWrite(Hours2[i],BCD[pick2][i]);
      digitalWrite(Mins1[i],BCD[pick3][i]);
      digitalWrite(Mins2[i],BCD[pick4][i]);
      mcp.digitalWrite(Secs1[i],BCD[pick5][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick6][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[0]++; //Amount of rolls by a set time
  }

  while(AniArr[1]<18){ //Hour1 First Digit LOCKED others rolling
    Hours = myRTC.getHour(h12, hPM);
    pick1 = random(0,9);
    pick2 = random(0,9);
    pick3 = random(0,9);
    pick4 = random(0,9);
    pick5 = random(0,9);
    for(int i = 0; i<4; i++){
      digitalWrite(Hours1[i],BCD[Hours/10][i]);
      digitalWrite(Hours2[i],BCD[pick1][i]);
      digitalWrite(Mins1[i],BCD[pick2][i]);
      digitalWrite(Mins2[i],BCD[pick3][i]);
      mcp.digitalWrite(Secs1[i],BCD[pick4][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick5][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[1]++;
  }

  while(AniArr[2]<18){ //Hour2 Second Digit LOCKED others rolling
    Hours = myRTC.getHour(h12, hPM);
    pick1 = random(0,9);
    pick2 = random(0,9);
    pick3 = random(0,9);
    pick4 = random(0,9);
    for(int i = 0; i<4; i++){
      digitalWrite(Hours2[i],BCD[Hours%10][i]);
      digitalWrite(Mins1[i],BCD[pick1][i]);
      digitalWrite(Mins2[i],BCD[pick2][i]);
      mcp.digitalWrite(Secs1[i],BCD[pick3][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick4][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[2]++;
  }

  while(AniArr[3]<18){ //Minute1 Third Digit LOCKED others rolling
    Minutes = myRTC.getMinute();  
    pick1 = random(0,9);
    pick2 = random(0,9);
    pick3 = random(0,9);
    for(int i = 0; i<4; i++){
      digitalWrite(Mins1[i],BCD[Minutes/10][i]);
      digitalWrite(Mins2[i],BCD[pick1][i]);
      mcp.digitalWrite(Secs1[i],BCD[pick2][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick3][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[3]++;
  }

  while(AniArr[4]<18){ //Minute2 Fourth Digit LOCKED others rolling(Not yet, seconds will be rolling)
    Minutes = myRTC.getMinute();  
    pick1 = random(0,9);
    pick2 = random(0,9);
    for(int i = 0; i<4; i++){
      digitalWrite(Mins2[i],BCD[Minutes%10][i]);
      mcp.digitalWrite(Secs1[i],BCD[pick1][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick2][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[4]++;
  }

    while(AniArr[5]<18){ //Second1 Fifth Digit LOCKED Second2 wont roll FINISHED
    Seconds = myRTC.getSecond();  
    pick1 = random(0,9);
    for(int i = 0; i<4; i++){
      mcp.digitalWrite(Secs1[i],BCD[Seconds/10][i]); 
      mcp.digitalWrite(Secs2[i],BCD[pick1][i]);
    }
    delay(del); //Has a bit of delay so it's visible to the eye
    AniArr[5]++;
  }
}

void PrintDate(){ //========================================================================= PRINT DATE ========================================================================================
  Serial.print (myRTC.getDate(), DEC); //just printing the date in the console
  Serial.print ("(");
  Serial.print (myRTC.getDoW()); //1. Monday - 7. Sunday
  Serial.print (")");
  Serial.print ("/");
  Serial.print (myRTC.getMonth(century), DEC);
  Serial.print ("/");
  Serial.println (myRTC.getYear(), DEC);  
}

void Alarm(){ //========================================================================= ALARM ========================================================================================
  if(myRTC.getDoW() >6){
    if(myRTC.checkIfAlarm(1,false) == 1){ //Checking If alarmflag is raised
     digitalWrite(Sound,0); //Turn on Sound
     digitalWrite(Mosfet,1); //Turn on Tubes
    }
  } 
}

void setup() { //========================================================================= SET UP ========================================================================================
  mcp.begin_I2C(0x20); //Start Mcp OBJECT A0,A1,A2 Pins ALL PULLED TO GND 
  Serial.begin(9600); // make serial monitor match
  Serial.println ("Setup Initiated");
  Serial.print ("Current Mode: ");
  Serial.println (CurrentMode);
  Wire.begin(); //Start I2C Interface
  //myRTC.setClockMode(false);  // set clock to 24H, True for 12H
  startMillis = millis();  //initial start time

  //Set all outputs 
  mcp.pinMode(TimeButton,INPUT_PULLUP); 
  mcp.pinMode(SetButton,INPUT_PULLUP);
  mcp.pinMode(Mosfet,OUTPUT); //MOSFET FOR BLANKING TUBES
  mcp.pinMode(Sound,OUTPUT); //ALARM SOUND CARD Signal
  for(int i=0; i<4; i++){ //Set all outputs for the digits
    pinMode(Hours1[i],OUTPUT);
    pinMode(Hours2[i],OUTPUT);
    pinMode(Mins1[i],OUTPUT);
    pinMode(Mins2[i],OUTPUT);
    mcp.pinMode(Secs1[i],OUTPUT);
    mcp.pinMode(Secs2[i],OUTPUT);
  }  
  digitalWrite(Sound,1); //When sound 0(Low) Alarm will play
  digitalWrite(Mosfet,0); //Tubes are turned off by default(0-Low) When Mosfet is 1 Tubes turn on
  for(int i = 0; i<4; i++){ //Turn all digits to '0'
    digitalWrite(Hours1[i],BCD[0][i]);
    digitalWrite(Hours2[i],BCD[0][i]);
    digitalWrite(Mins1[i],BCD[0][i]);
    digitalWrite(Mins2[i],BCD[0][i]);
    mcp.digitalWrite(Secs1[i],BCD[0][i]);
    mcp.digitalWrite(Secs2[i],BCD[0][i]);
  }
   /*
    myRTC.setYear(25);
    myRTC.setMonth(3);
    myRTC.setDate(16);
    myRTC.setDoW(7); */
} // SETUP END

void loop() { //============================================================================ LOOP ========================================================================================

  switch(CurrentMode){
    case 0: //============================================================================ TIME PRINT MODE =================================================================================
      //calling print time function and serial monitoring
      WriteTime();
      PrintDate();
      TimeButtonRead(TimeButton);
      Serial.print (Hours);
      Serial.print (":");
      Serial.print (Minutes);
      Serial.print (":");
      Serial.println (Seconds);
      
    break;

    case 1: //============================================================================ TIME SET MODE ========================================================================================
      if(SetScreenFlag==true){ //just set the screen ONCE at the first button press
        for(int i = 0; i<4; i++){ //Set Screen 0000
            digitalWrite(Hours1[i],BCD[0][i]);
            digitalWrite(Hours2[i],BCD[0][i]);
            digitalWrite(Mins1[i],BCD[0][i]);
            digitalWrite(Mins2[i],BCD[0][i]);
            mcp.digitalWrite(Secs1[i],BCD[0][i]); 
            mcp.digitalWrite(Secs2[i],BCD[0][i]);
        }
        SetScreenFlag = false;
      }
      SetDigit();
    break;

    case 2: //============================================================================ DIGIT TESTING ========================================================================================
      //Digit Testing
      for(int c=0; c<10; c++){
        for(int i = 0; i<4; i++){ //All digits will turn on for one second
          digitalWrite(Hours1[i],BCD[c][i]);
          digitalWrite(Hours2[i],BCD[c][i]);
          digitalWrite(Mins1[i],BCD[c][i]);
          digitalWrite(Mins2[i],BCD[c][i]);
          mcp.digitalWrite(Secs1[i],BCD[c][i]);
          mcp.digitalWrite(Secs2[i],BCD[c][i]);
        }
        Serial.println(c);
        delay(1000);
      }
    break;

    case 3: //============================================================================ ANIMATIONS ======================================================================================== 
        //CathodeCure();
        NumberAnimation();
    break;

    case 4: //============================================================================ EXPIREMENTAL ========================================================================================
      /* Dimming Testing
      for(int i = 0; i<4; i++){ 
        digitalWrite(Hours1[i],DIM[0][i]);
        digitalWrite(Hours2[i],DIM[0][i]);
        digitalWrite(Mins1[i],DIM[0][i]);
        digitalWrite(Mins2[i],DIM[0][i]);
        mcp.digitalWrite(Secs1[i],DIM[0][i]); 
        mcp.digitalWrite(Secs2[i],DIM[0][i]);
        }
        */
      // Alarm Testing
      //myRTC.checkIfAlarm(1,false);
    break;
  } //SWITCH END
}//MAIN LOOP END
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
//SDA to GPIO 22, SCL to GPIO 21
//Button inputs for set up

#define button1 4 // Left button / #1 
#define button2 16 // Right button / #2 
#define button3 17 // Middleiddle button / Gamemode / Reset
#define led1 32
#define led2 33  
int b1,b2,b3; // Button variables 
int l1, l2; // LEDs 

// Variables to change game mode
int gs = 0; // 0 = Death Clicks, 1 = KotH 
bool lastbuttonstate=LOW; 
bool buttonstate=LOW;

// Death Clicks Variables
int press = 0; //Number of button presses/clicks

// KotH Timer Variables 
unsigned int time1; // Time on left side 
unsigned int time2; // Time on right side 
unsigned long lastMilliS; // Used for setting pauses
int turn; // Used to set states of KotH
bool checkButton; // Checks if button is pressed, true = HIGH, flase = LOW

//Register inputs
void setup()
{
// initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.begin(16,2);

  pinMode(button1,INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  time1 = time2 = 100; // in mS
  
  turn = 0; // Turn set to 0 so KotH timer dones't start on switch
}

void loop()
{
  b1 = digitalRead(button1);
  b2 = digitalRead(button2);
  b3 = digitalRead(button3);
  buttonstate = digitalRead(button3);

  // Game switching mechanism
  if ((buttonstate) && (!lastbuttonstate))
    {
    gs = 1 - gs;
  
    }
  lastbuttonstate=buttonstate;

  switch (gs)
  {
    //Death Clicks
    case 0:
      if((b1 == HIGH) || (b2 == HIGH))
      {
        press++;
        delay(100);
      }
      if (b3 == HIGH)
      {
        // Resets Death clicks and KotH Timers 
        press = 0; 
        lcd.setCursor(0,1);
        lcd.print(press);
        lcd.print("               ");
        delay(250);
        turn = 0; 
        time1 = 100;
        time2 = 100;
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
      }
      lcd.setCursor(0,0);
      lcd.print("Death Count     "); // extra spaces to clear out screen instead of clear
      lcd.setCursor(0,1);
      lcd.print(press);
      break;
      
    // KotH
    case 1:
      checkButton=digitalRead(button1);   // player 1 button press
      if((checkButton) && (turn!=2))
      {
        digitalWrite(led1,HIGH);
        digitalWrite(led2,LOW);
          turn = 2;
          printTime();
          }
          checkButton=digitalRead(button2);   // player 2 button press
          if((checkButton) && (turn!=1))
         {
          digitalWrite(led2,HIGH);
        digitalWrite(led1,LOW);
         turn = 1;
         printTime();
         }
         if (time1==0) // Team 1 End Game
         {
          digitalWrite(led1,HIGH);
        digitalWrite(led2,HIGH);
         turn = 3;
         }
         if (time2==0) // Team 2 End Game
        {
          digitalWrite(led1,HIGH);
        digitalWrite(led2,HIGH);
         turn=4;
       }
      if (turn == 0)
      {
        lastMilliS = millis(); // timers don't start until a button is pressed
        lcd.setCursor(0,0);
        lcd.print("Team 1    Team 2");
        printTime();
      }
      if (millis()-lastMilliS >=100)
        {
          lastMilliS = millis();
          if (turn == 1)
          {
            time2--;
          }
          if (turn == 2)
          {
            time1--;
          }
          printTime();
          if(turn == 3)
          {
            lastMilliS=millis();  // Pause Timer for win condition 
            lcd.setCursor( 0 , 0 );
            lcd.print("Team 1 Wins     ");
          }
          if(turn == 4)
          {
            lastMilliS=millis();
            lcd.setCursor( 0 , 0 );
            lcd.print("Team 2 Wins     ");
          }
          
      }
       break;
  }
}

//fucntion for writing time to LCD
void printTime()
{
// r = right side, l = left side
  int rsecDis1, lsecDis1;
  int rsecDis10, lsecDis10;
  int rminDis1, lminDis1;
  int rminDis10, lminDis10;
  int rsecs, lsecs;
  
  rsecs = time1 / 10;                  // convert 1/10 sec to H:MM:SS
  rsecs = rsecs % 3600;
  rminDis10 = rsecs / 600;
  rsecs = rsecs % 600;
  rminDis1 = rsecs / 60;
  rsecs = rsecs % 60;
  rsecDis10 = rsecs / 10;
  rsecDis1 = rsecs % 10;
  
  lsecs = time2 / 10;                  // convert 1/10 sec to H:MM:SS
  lsecs = lsecs % 3600;
  lminDis10 = lsecs / 600;
  lsecs = lsecs % 600;
  lminDis1 = lsecs / 60;
  lsecs = lsecs % 60;
  lsecDis10 = lsecs / 10;
  lsecDis1 = lsecs % 10;
  
  
  lcd.setCursor(0,1);
  lcd.print(rminDis10);
  lcd.print(rminDis1);
  lcd.print(":");
  lcd.print(rsecDis10);
  lcd.print(rsecDis1);
  lcd.print("      ");
  lcd.print(lminDis10);
  lcd.print(lminDis1);
  lcd.print(":");
  lcd.print(lsecDis10);
  lcd.print(lsecDis1);
}

 




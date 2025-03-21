// Programming Notes
// INPUT_PULLUP causes a reverse logic to avoid using additional hardware
// Button Press = LOW / !b#
// Button Unpressed = HIGH / b#
// lled and rled are LEDs on in the buttons, they are commented out because they are no longer used, but left in code just incase.

#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 column and 2 rows
//SDA to GPIO 21, SCL to GPIO 22

//LED Strip Set Up
#define PIN_NEO_PIXEL 12  // Middle pin of strip to pin D25
#define NUM_PIXELS 55     // Number of LEDs in strip
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

//Button inputs for set up
#define button1 4   // Left button / #1
#define button2 14  // Right button / #2
#define button3 27  // Middleiddle button / Gamemode / Reset
// #define lled 32     //Left LED / RED
// #define rled 33     // Right LED / BLUE
int b1, b2, b3;  // Button variables
// int l1, l2;         // LEDs

// Variables to change game mode
#define mode_deathclicks 0
#define mode_KotH 1
#define mode_hotpot_timed 2
#define mode_hotpot_inf 3

int currentmode = mode_deathclicks;  // Deathclicks, koth, hotpit (pending)
bool lastButtonState = HIGH;
bool buttonstate = HIGH;

// Death Clicks Variables
int press;  //Number of button presses/clicks

// KotH Timer Variables; Left Side = 1, Right Side = 2
unsigned int ltime;        // Time on left side
unsigned int rtime;        // Time on right side
unsigned long lastMilliS;  // Used for setting pauses
int turn;                  // Used to set states of KotH
bool checkButton;          // Checks if button is pressed, true = HIGH, false = LOW

// HotPot Variables
unsigned int hptime;
unsigned long buttonpressstart;
const unsigned long requirepressduration = 3000;  // 3 second button press
int countdown;
bool buttonpress = HIGH;

//Register inputs
void setup() {

  lcd.init();  // initialize the lcd
  lcd.backlight();
  lcd.begin(16, 2);

  pinMode(button1, INPUT_PULLUP);  // left button
  pinMode(button2, INPUT_PULLUP);  // right button
  pinMode(button3, INPUT_PULLUP);  // middle
  // pinMode(lled, OUTPUT);    // left button led
  // pinMode(rled, OUTPUT);    // right button led

  strip.begin();
}

// used to switch gamemode, to add a game mode change the number after %
void GameModeSwitch() {
  buttonstate = digitalRead(button3);
  if (b3 == LOW && lastButtonState) {
    currentmode = (currentmode + 1) % 4;
    lastButtonState = LOW;

    // digitalWrite(lled, LOW);
    // digitalWrite(rled, LOW);
    colorLEDChange(0, 0, 0);

    //Reset All counters on middle button press
    countdown = 0;
    press = 0;
    turn = 0;
    ltime = rtime = 4200;
    hptime = 900;
    buttonpressstart = 0;

  } else if (b3 == HIGH) {
    lastButtonState = HIGH;
  }
}

void playDeathClicks() {
  if (!b1 || !b2) {
    press++;  // increase death count if b1 or b2 pressed
    delay(500);
  }
  if (!b3)  // when gamemode is switched reset death counter to 0
  {
    press = 0;
    lcd.setCursor(0, 1);
    lcd.print(press);
    lcd.print("               ");
    delay(500);
  }
  lcd.setCursor(0, 0);
  lcd.print("Death Count     ");  // extra spaces to clear out screen instead of clear
  lcd.setCursor(0, 1);
  lcd.print(press);
}

void playKotH() {
  // Right Side
  if ((!b1) && (turn != 2)) {
    //digitalWrite(lled, HIGH);
    //digitalWrite(rled, LOW);
    turn = 2;
    printTime();
  }
  // Left Side
  if ((!b2) && (turn != 1)) {
    //digitalWrite(lled, LOW);
    //digitalWrite(rled, HIGH);
    turn = 1;
    printTime();
  }
  if (ltime == 0) {
    //digitalWrite(lled, HIGH);
    //digitalWrite(rled, HIGH);
    blinkred();
    turn = 3;
  }
  if (rtime == 0) {
    //digitalWrite(lled, HIGH);
    //digitalWrite(rled, HIGH);
    blinkblue();
    turn = 4;
  }
  if (turn == 0) {
    lastMilliS = millis();  // timers don't start until a button is pressed
    lcd.setCursor(0, 0);
    lcd.print("Team R    Team B");
    printTime();
  }
  if (millis() - lastMilliS >= 100)  // right side button pressed, left side timer counts down
  {
    lastMilliS = millis();
    if (turn == 1) {
      rtime--;
      colorLEDChange(0, 0, 255);  // Turn Strip blue
    }
    if (turn == 2)  // left side button pressed, right side timer counts  down
    {
      ltime--;
      colorLEDChange(255, 0, 0);  // Turn Strip red
    }
    printTime();
    // Turns 3 and 4 Pause Timer for win condition
    if (turn == 3) {
      lastMilliS = millis();
      lcd.setCursor(0, 0);
      lcd.print("Team R Wins     ");
    }
    if (turn == 4) {
      lastMilliS = millis();
      lcd.setCursor(0, 0);
      lcd.print("Team B Wins     ");
    }
  }
}

void playHotPot() {
  if (!b1 || !b2) {
    turn = 1;
    printHPT();
    lcd.setCursor(5, 1);
    lcd.print("           ");
    printHPT();
  }
  if (turn == 0) {
    lastMilliS = millis();  // timers don't start until a button is pressed
    lcd.setCursor(0, 0);
    lcd.print("  HOT POT TIME  ");
  }
  if (millis() - lastMilliS >= 100) {
    lastMilliS = millis();
    if (turn == 1) {
      if (countdown < 5) {
        countdown++;
        blinkblue();
      } else if (countdown < 10) {
        countdown++;
        blinkyellow();
      }
      if ((countdown == 10) && (hptime >= 450)) {
        hptime--;
        colorLEDChange(255, 0, 0);  // turn strip red
        printHPT();
      }
      if ((countdown == 10) && (hptime < 450)) {
        colorLEDChange(0, 255, 0);  // turn strip green
        hptime--;
        printHPT();

        if ((!b1 || !b2) && !buttonpress) {
          buttonpressstart = millis();
          buttonpress = HIGH;
          //digitalWrite(lled, HIGH);
        } else if ((b1 && b2) && buttonpress) {
          buttonpress = LOW;
        } else if (buttonpress) {
          if (millis() - buttonpressstart >= requirepressduration) {
            blinkgreen();
            turn = 3;
          }
        }
      }
    }
    if ((countdown == 10) && (hptime == 0)) {
      turn = 4;
    }
  }
  if (turn == 3) {
    lastMilliS = millis();  // Pause Timer for win condition
    lcd.setCursor(0, 1);
    lcd.print("    CAPTURED    ");
    blinkgreen();
  }
  if (turn == 4) {
    lastMilliS = millis();  // Pause Timer for win condition
    lcd.setCursor(0, 1);
    lcd.print("   NO CAPTURE   ");
    blinkred();
  }
}

void playHotPotInf() {
  if (!b1 || !b2) {
    turn = 1;
    printHPT();
    lcd.setCursor(5, 1);
    lcd.print("           ");
    printHPT();
  }
  if (turn == 0) {
    lastMilliS = millis();  // timers don't start until a button is pressed
    lcd.setCursor(0, 0);
    lcd.print("  HOT POT INF");
  }
  if (millis() - lastMilliS >= 100) {
    lastMilliS = millis();
    if (turn == 1) {
      if (countdown < 5) {
        countdown++;
        blinkblue();
      } else if (countdown < 10) {
        countdown++;
        blinkyellow();
      }
      if ((countdown == 10) && (hptime >= 450)) {
        hptime--;
        colorLEDChange(255, 0, 0);  // turn strip red
        printHPT();
      }
      if ((countdown == 10) && (hptime < 450)) {
        colorLEDChange(0, 255, 0);  // turn strip green
        hptime--;
        printHPT();

        if ((!b1 || !b2) && !buttonpress) {
          buttonpressstart = millis();
          buttonpress = HIGH;
          //digitalWrite(lled, HIGH);
        } else if ((b1 && b2) && buttonpress) {
          buttonpress = LOW;
        } else if (buttonpress) {
          if (millis() - buttonpressstart >= requirepressduration) {
            blinkgreen();
            turn = 3;
          }
        }
      }
    }
  }
  if (turn == 3) {
    lastMilliS = millis();  // Pause Timer for win condition
    lcd.setCursor(0, 1);
    lcd.print("    CAPTURED    ");
    blinkgreen();
  }
}

void printTime() {
  // r = right side, l = left side
  int rsecDis1, lsecDis1;
  int rsecDis10, lsecDis10;
  int rminDis1, lminDis1;
  int rminDis10, lminDis10;
  int rsecs, lsecs;

  rsecs = rtime / 10;  // convert 1/10 sec to H:MM:SS
  rsecs = rsecs % 3600;
  rminDis10 = rsecs / 600;
  rsecs = rsecs % 600;
  rminDis1 = rsecs / 60;
  rsecs = rsecs % 60;
  rsecDis10 = rsecs / 10;
  rsecDis1 = rsecs % 10;

  lsecs = ltime / 10;  // convert 1/10 sec to H:MM:SS
  lsecs = lsecs % 3600;
  lminDis10 = lsecs / 600;
  lsecs = lsecs % 600;
  lminDis1 = lsecs / 60;
  lsecs = lsecs % 60;
  lsecDis10 = lsecs / 10;
  lsecDis1 = lsecs % 10;


  lcd.setCursor(0, 1);
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

void printHPT() {
  int hpsecDis1;
  int hpsecDis10;
  int hpminDis1;
  int hpminDis10;
  int hpsecs;

  hpsecs = hptime / 10;  // convert 1/10 sec to H:MM:SS
  hpsecs = hpsecs % 3600;
  hpminDis10 = hpsecs / 600;
  hpsecs = hpsecs % 600;
  hpminDis1 = hpsecs / 60;
  hpsecs = hpsecs % 60;
  hpsecDis10 = hpsecs / 10;
  hpsecDis1 = hpsecs % 10;

  lcd.setCursor(0, 1);
  lcd.print(hpminDis10);
  lcd.print(hpminDis1);
  lcd.print(":");
  lcd.print(hpsecDis10);
  lcd.print(hpsecDis1);
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);  //  Set pixel's color (in RAM)
    strip.show();                   //  Update strip to match
    delay(wait);                    //  Pause for a moment
  }
}

void colorLEDChange(int redSetting, int greenSetting, int blueSetting) {
  colorWipe(strip.Color(redSetting, greenSetting, blueSetting), 0);
}

void blinkyellow() {
  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 252, 227, 3);
  }
  strip.show();
  delay(500);

  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 0, 0);
  }
  strip.show();
  delay(500);
}

void blinkred()  // LED strip blinks red. 1 second 1, 1 second off
{
  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 255, 0, 0);
  }
  strip.show();
  delay(1000);

  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 0, 0);
  }
  strip.show();
  delay(1000);
}

void blinkgreen() {
  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 255, 0);
  }
  strip.show();
  delay(1000);

  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 0, 0);
  }
  strip.show();
  delay(1000);
}
void blinkblue()  // LED strip blinks blue. 1 second 1, 1 second off
{
  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 0, 255);
  }
  strip.show();
  delay(1000);

  for (int led = 0; led < NUM_PIXELS; led++) {
    strip.setPixelColor(led, 0, 0, 0);
  }
  strip.show();
  delay(1000);
}

void loop() {
  b1 = digitalRead(button1);
  b2 = digitalRead(button2);
  b3 = digitalRead(button3);

  GameModeSwitch();
  switch (currentmode) {
    case mode_deathclicks:
      playDeathClicks();
      break;
    case mode_KotH:
      playKotH();
      break;
    case mode_hotpot_timed:
      playHotPot();
      break;
    case mode_hotpot_inf:
      playHotPotInf();
      break;
    default:
      break;
  }
}
/*************************************************************************
//
//  JONATHON TAYLOR - LIGHT UP CLOCK 2.0 - PROGRAMMABLE COLOR CLOCK
//
/*************************************************************************
 
  - Connections for DS1307RTC
    - SDA   --   A4
    - SCL   --   A5
    - VCC   --   5V or 3.3V
    - GND   --   GND
    
  - Connections for RGB LED
    - RED Connection       --    D11
    - GREEN Connection     --    D10
    - BLUE Connection      --    D9
    - GND                  --    GND
    
  - Connections for 4-Digit Display TM1637
    - VCC   --  5V
    - GND   --  GND
    - CLK   --  D13
    - DIO   --  D12
    
  - Connections for 3 Buttons
    - Menu      --    D6
    - Plus +    --    D7
    - Minus -   --    D8
    - GND       --    GND

  - Connections for Switch
    - DisplayOn     --    D5
    - GND           --    GND
   
  - Useful Colors
    - RED:    (255,0,0)
    - GREEN:  (0,255,0)
    - BLUE:   (0,0,255)
    - YELLOW: (200,80,0)
    - PURPLE: (255,0,255)
    - PINK:   (255,0,10)
    - AQUA:   (0,255,255)
    - ORANGE: (255,30,0)
**********************************************************/

#include <Wire.h>
#include "RTCModule.h"
#include <Arduino.h>
#include "TM1637.h"
#include <EEPROM.h>
#include <Bounce2.h>

#define CLK 13
#define DIO 12

#define L1BRIGHTNESS 0
#define L2BRIGHTNESS 1
#define L3BRIGHTNESS 2
#define L4BRIGHTNESS 3
#define L5BRIGHTNESS 4

//************RTC*********************//
RTC_DS1307 RTC;

//************Display*****************//
TM1637 display(CLK, DIO);

//********DisplayOn Switch************//
int DisplayOn = 5; // Switch On/Off

//************Buttons*****************//
Bounce bouncer1 = Bounce(); 
Bounce bouncer2 = Bounce(); 
Bounce bouncer3 = Bounce(); 

int Button1 = 6; // Button SET MENU'
int Button2 = 7; // Button +
int Button3 = 8; // Button -

//************RGB Outputs*************//
int redPin = 11;
int greenPin = 10;
int bluePin = 9;

//************Variables***************//
const byte ALLDAYS = B01111111;
const byte WEEKDAYS = B01111100;
const byte WEEKENDS = B00000011;

byte currentHour;
byte currentMinute;
byte currentSecond;
byte currentDayOfWeek;
byte currentDay;
byte currentMonth;
byte currentYear;

int8_t Digits[] = {0,1,2,3};

byte colorOn = 0;
const byte numColors = 9;
const byte numSettings = 7;
                                   
byte colorArray[numColors][numSettings] = {
{4,L5BRIGHTNESS,ALLDAYS,6,40,6,50},      //RED ALL 6:40-6:50
{2,L5BRIGHTNESS,ALLDAYS,6,50,7,0},       //YEL ALL 6:50-7:00
{11,L5BRIGHTNESS,ALLDAYS,7,0,8,0},      //GRN ALL 7:00-8:00
{6,L4BRIGHTNESS,ALLDAYS,19,30,19,50},    //PUR ALL 19:30-19:50
{3,L3BRIGHTNESS,ALLDAYS,19,50,20,10},    //ORANGE ALL 19:50-20:10
{4,L2BRIGHTNESS,ALLDAYS,20,0,20,30},     //RED ALL 20:00-20:30
{7,L5BRIGHTNESS,ALLDAYS,20,0,20,30},     //BLUE ALL 20:00-20:30
{9,L5BRIGHTNESS,ALLDAYS,20,0,20,30},     //AQUA ALL 20:00-20:30
{5,L5BRIGHTNESS,ALLDAYS,20,0,20,30}      //PINK ALL 20:00-20:30
};

byte numColorAlarms = 3; // Max is 9, but starts with first 3 set in morning

int menu = 0;
int colorMenu = 0;
int colonToggle = 0;

static unsigned long nextColonTime;

const uint8_t DISPLAYALL [ ] = {
0x7f,
10, // A
19, // L
19  // L
};

const uint8_t DISPLAYWEEK [ ] = {
24, // M 
25, // M
23, // -
15  // F
};

const uint8_t DISPLAYSASU [ ] = {
5,  // S
10, // A
5,  // S
20  // U
};

const uint8_t DISPLAYL1 [ ] = {
11, // b
28, // r
0x7f,
1   // 1
};

const uint8_t DISPLAYL2 [ ] = {
11, // b
28, // r
0x7f,
2   // 2
};

const uint8_t DISPLAYL3 [ ] = {
11, // b
28, // r
0x7f,
3   // 3
};

const uint8_t DISPLAYL4 [ ] = {
11, // b
28, // r
0x7f,
4   // 4
};

const uint8_t DISPLAYL5 [ ] = {
11, // b
28, // r
0x7f,
5   // 5
};

const uint8_t DISPLAYLOVE [ ] = {
19, // L
0,  // O
20, // V
14  // E
};

const byte totalColors = 13;
const byte MYCOLORS [13] [3] = {
  {255,255,255}, // WHITE = 0
  {255,255,0},   // LIGHT YELLOW = 1
  {200,80,0},    // YELLOW = 2
  {200,40,0},    // ORANGE = 3
  {255,0,0},     // RED = 4
  {255,0,80},    // PINK = 5
  {255,0,255},   // PURPLE = 6
  {0,100,255},   // LIGHT BLUE = 7
  {0,0,255},     // BLUE = 8
  {0,255,255},   // AQUA = 9
  {70,255,0},    // LIGHT GREEN = 10
  {0,255,0},     // GREEN = 11
  {100,100,255}  // BLUE WHITE = 12
};

const int BRIGHTNESSLEVEL [5] = {
120,
220,
420,
720,
1023  
};

/*************************************************************************
//
//  SETUP AT BEGINNING
//
//************************************************************************/

void setup () {
  
  Serial.begin(9600);
  
  // RGB Outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
  
  // Button Inputs
  pinMode(Button1,INPUT_PULLUP);
  pinMode(Button2,INPUT_PULLUP);
  pinMode(Button3,INPUT_PULLUP);

  // Switch Input
  pinMode(DisplayOn,INPUT_PULLUP);
  
  bouncer1.attach(Button1); 
  bouncer1.interval(5); 
  bouncer2.attach(Button2); 
  bouncer2.interval(5); 
  bouncer3.attach(Button3); 
  bouncer3.interval(5); 
  
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  display.set(0);//0-7 BRIGHT_TYPICAL 
  display.init();
  
  // Set the 500ms colon timer (500 = 1 second)
  nextColonTime = millis() + 500L;  
  
  // Write the starting colorArray to EEPROM first time uploading, 
  // *** then comment out first line since we want to read current state every future reboot
  longTermSave();  
  longTermRetrieve();
}


/**************************************************************************
//
//  MAIN LOOP
//
//*************************************************************************/

void loop () {
  
  // Refresh date/time
  GetDateTime();
  
  // Check if menu button has been pressed (unless we are in the colors menu we'll ignore here)
  if(menu < 4)  {
    if(bouncer1.update() && bouncer1.read() == LOW) {
      Serial.println("Menu Pushed");
      menu = menu + 1;
    }
  }
    
  // Test menu options...
  if (menu == 0) {
    // COLOR TESTER
    if(digitalRead(Button2) == LOW && digitalRead(Button3) == LOW) {
        Serial.println("Color Tester Pushed");
        ColorTesterSalute();
    }
    NormalTimeDisplay();  // Subroutine for just showing the time...
  }
  else if (menu == 1) {
    Serial.println("Hour Set");
    DisplayColor(0,0,0,0);
    SetHour();          // Subroutine for setting the hour...
  }
  else if (menu == 2) {
    Serial.println("Minute Set");
    DisplayColor(0,0,0,0);
    SetMinute();        // Subroutine for setting the minute...
  }
  else if (menu == 3) {
    Serial.println("Number of Color Alarms Set");
    DisplayColor(0,0,0,0);
    SetNumColorAlarms();  // Subroutine for setting the number of colors
  }
  else if (menu >= 4 && menu < (4 + numColorAlarms)) {
    SetColor(menu-4);    // Subroutine for setting each of the colors...
  }
  else {    // Finished with the menu, save settings long-term to EEPROM
    longTermSave();
    menu = 0;
  }
  delay(10);
}


/**************************************************************************
//
//  DATE AND TIME SETTING AND RETRIEVING FUNCTIONS
//
//*************************************************************************/

void GetDateTime() 
{
  Serial.println("Getting Time");
  DateTime now = RTC.now(); 
  
  currentHour = now.hour();
  currentMinute = now.minute();
  currentSecond = now.second();
  currentDayOfWeek = now.dayOfWeek();
  currentDay = now.day();
  currentMonth = now.month();
  currentYear = now.year();
}

void SetDateTime()
{
  Serial.println("Setting Time");
  RTC.adjust(DateTime(currentYear,currentMonth,currentDay,currentHour,currentMinute,0));
}

void NormalTimeDisplay() 
{
  Serial.println("Display Time");
  int currentTime;  // in minutes through the day
  int beginTime;
  int endTime;
  
  currentTime = currentHour*60 + currentMinute;
  
  if(currentHour > 12) {
    currentHour = currentHour - 12;
  }
  if(currentHour == 0) {
    currentHour = 12;
  }
  DisplayDigits(currentHour, currentMinute, 0);
  
  colorOn = 0;
  
  // Check the color alarms and set color if in window
  for (int i=0; i < numColorAlarms; i++) {
    // First check if this day of the week is enabled
    if((B10000000 >> (currentDayOfWeek+1)) & colorArray[i][2]) {
      // Next check if the current time is between the start and end time
      beginTime = colorArray[i][3]*60 + colorArray[i][4];
      endTime = colorArray[i][5]*60 + colorArray[i][6];
      if(beginTime <= currentTime && currentTime < endTime) {
        DisplayColor(MYCOLORS[colorArray[i][0]][0],MYCOLORS[colorArray[i][0]][1],MYCOLORS[colorArray[i][0]][2],colorArray[i][1]);
        colorOn = 1;
      }
    }
  }
  
  if(colorOn == 0) {
    DisplayColor(0,0,0,0);
  }
}

void SetHour()
{
  if(bouncer2.update() && bouncer2.read() == LOW) {
    if(currentHour == 23) {
      currentHour = 0;
    }
    else {
      currentHour = currentHour + 1;
    }
  }
  else if(bouncer3.update() && bouncer3.read() == LOW) {
    if(currentHour == 0) {
      currentHour = 23;
    }
    else {
      currentHour = currentHour - 1;
    }
  }
  // Display the new time, then delay
  DisplayDigits(currentHour,currentMinute,1);
  delay(10);
  // Then save the time to the RTC
  SetDateTime();
}

void SetMinute()
{
  if(bouncer2.update() && bouncer2.read() == LOW) {
    if(currentMinute == 59) {
      currentMinute = 0;
    }
    else {
      currentMinute = currentMinute + 1;
    }
  }
  else if(bouncer3.update() && bouncer3.read() == LOW) {
    if(currentMinute == 0) {
      currentMinute = 59;
    }
    else {
      currentMinute = currentMinute - 1;
    }
  }
  // Display the new time, then delay
  DisplayDigits(currentHour,currentMinute,2);
  // Then save the time to the RTC
  delay(10);
  SetDateTime();
}

void SetNumColorAlarms()
{
  if(bouncer2.update() && bouncer2.read() == LOW) {
    if(numColorAlarms != numColors) {
      numColorAlarms = numColorAlarms + 1;
    }
  }
  else if(bouncer3.update() && bouncer3.read() == LOW) {
    if(numColorAlarms != 0) {
      numColorAlarms = numColorAlarms - 1;
    }
  }
  // Display the number of color alarms, then delay
  DisplayText(numColorAlarms,1);
  delay(10);
}

/**************************************************************************
//
//  MENU FOR SETTING EACH COLOR FUNCTIONS
//
//*************************************************************************/

void SetColor(int colorNum)
{
  // First, display the color we are dealing with
  DisplayColor(MYCOLORS[colorArray[colorNum][0]][0],MYCOLORS[colorArray[colorNum][0]][1],MYCOLORS[colorArray[colorNum][0]][2],colorArray[colorNum][1]);
  
  // Set Color            -- colorMenu 0
  // Set Brightness       -- colorMenu 1
  // Set Days of week     -- colorMenu 2 (ALLDAYS, WEEKDAYS, WEEKENDS)
  // Set On Time          -- colorMenu 3 = hour, 4 = min
  // Set Off Time         -- colorMenu 5 = hour, 6 = min

  
  // Menu cycles through the color menu for each color
  if(bouncer1.update() && bouncer1.read() == LOW) {
    colorMenu = colorMenu + 1;
  }

  // Test Color Menu options...
  if (colorMenu == 0) {                                              // Change the Color for this setting
    if(bouncer2.update() && bouncer2.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == (totalColors-1)) {
        colorArray[colorNum][colorMenu] = 0;
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] + 1;
      }
    }
    else if(bouncer3.update() && bouncer3.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == 0) {
        colorArray[colorNum][colorMenu] = (totalColors-1);
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] - 1;
      }
    }
    DisplayText(colorNum + 1,0);
    delay(10);
  }
  else if (colorMenu == 1) {                                              // BRIGHTNESS OPTION
    //Plus Button = higher brightness
    if(bouncer2.update() && bouncer2.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == L1BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L2BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L2BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L3BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L3BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L4BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L4BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L5BRIGHTNESS;
      }
    }
    //Minus Button = lower brightness
    if(bouncer3.update() && bouncer3.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == L2BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L1BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L3BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L2BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L4BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L3BRIGHTNESS;
      }
      else if(colorArray[colorNum][colorMenu] == L5BRIGHTNESS) {
        colorArray[colorNum][colorMenu] = L4BRIGHTNESS;
      }
    }
    int option = 0;
    if(colorArray[colorNum][colorMenu] == L1BRIGHTNESS) {
      option = 13;
    }
    else if(colorArray[colorNum][colorMenu] == L2BRIGHTNESS) {
      option = 14;
    }
    else if(colorArray[colorNum][colorMenu] == L3BRIGHTNESS) {
      option = 15;
    }
    else if(colorArray[colorNum][colorMenu] == L4BRIGHTNESS) {
      option = 16;
    }
    else {
      option = 17;
    }
    DisplayText(option,0);
    delay(10);
  }
  else if (colorMenu == 2) {                                              // DAYS OF WEEK OPTION
    if((bouncer2.update() && bouncer2.read() == LOW) || (bouncer3.update() && bouncer3.read() == LOW)) {
      if(colorArray[colorNum][colorMenu] == ALLDAYS) {
        colorArray[colorNum][colorMenu] = WEEKDAYS;
      }
      else if (colorArray[colorNum][colorMenu] == WEEKDAYS) {
        colorArray[colorNum][colorMenu] = WEEKENDS;
      }
      else {
        colorArray[colorNum][colorMenu] = ALLDAYS;
      }
    }
    int option = 0;
    if(colorArray[colorNum][colorMenu] == ALLDAYS) {
      option = 10;
    }
    else if (colorArray[colorNum][colorMenu] == WEEKDAYS) {
      option = 11;
    }
    else {
      option = 12;
    }
    DisplayText(option,0);
    delay(10);
  }
  else if (colorMenu == 3 || colorMenu == 5) {                            // Set On Time Hour or Off Time Hour
    if(bouncer2.update() && bouncer2.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == 23) {
        colorArray[colorNum][colorMenu] = 0;
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] + 1;
      }
    }
    else if(bouncer3.update() && bouncer3.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == 0) {
        colorArray[colorNum][colorMenu] = 23;
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] - 1;
      }
    }
    // Display the set hour, then delay
    DisplayDigits(colorArray[colorNum][colorMenu],0,1);
    delay(10);
  }
  else if (colorMenu == 4 || colorMenu == 6) {                            // Set On Time Minute or Off Time Minute
    if(bouncer2.update() && bouncer2.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == 59) {
        colorArray[colorNum][colorMenu] = 0;
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] + 1;
      }
    }
    else if(bouncer3.update() && bouncer3.read() == LOW) {
      if(colorArray[colorNum][colorMenu] == 0) {
        colorArray[colorNum][colorMenu] = 59;
      }
      else {
        colorArray[colorNum][colorMenu] = colorArray[colorNum][colorMenu] - 1;
      }
    }
    // Display the set minute, then delay
    DisplayDigits(0,colorArray[colorNum][colorMenu],2);
    delay(10);
  }
  else {
    colorMenu = 0;
    menu = menu + 1;
  }
}

/**************************************************************************
//
//  DISPLAY FUNCTIONS
//
//*************************************************************************/

void DisplayDigits( byte hour, byte minute, byte setting)
{
  // If the display is not set, then clear and return, unless we are in the menu
  if(digitalRead(DisplayOn) == 1 && menu == 0) {
    display.clearDisplay();
    display.point(POINT_OFF);
    return;
  }
    
  int8_t Digit0 = minute %10 ;
  int8_t Digit1 = (minute % 100) / 10 ;
  int8_t Digit2 = hour % 10 ;
  int8_t Digit3 = (hour % 100) / 10 ;

  if (setting == 1 ) {
    Digit1 = 0x7f ;
    Digit0 = 0x7f ;
  } else if (setting == 2) {
    Digit3 = 0x7f ;
    Digit2 = 0x7f ;
  }
  Digits[3] = Digit0 ;
  Digits[2] = Digit1 ;
  Digits[1] = Digit2 ;
  Digits[0] = Digit3 ;

  // All segments on
  display.display(Digits);
  
  // If we are in the menu, set the colon off
  if(menu != 0) {
    display.point(POINT_OFF);
  }
  // Otherwise alternate the colon on and off every 1/2 second
  else if(nextColonTime < millis()){
    if(colonToggle == 0) {
      colonToggle = 1;
      display.point(POINT_ON);
    } 
    else {
      colonToggle = 0;
      display.point(POINT_OFF);
    }
    // Set the 500ms colon timer (500 = 1 second)
    nextColonTime = millis() + 500L;    
  }
}

void DisplayText(int textCode, int numAlarmsSetting) {
  
  if (textCode >= 0 && textCode <= 9) {          // Display "COL1-9"
    // Use COL# for color setting of the color
    uint8_t displayNumColor [4] = {
      18, // C
      0,  // O
      19, // L
      0   // 0
    };
    // But use AL # for num alarms setting
    if (numAlarmsSetting == 1) {
      displayNumColor [0] = 10;    // A
      displayNumColor [1] = 19;    // L
      displayNumColor [2] = 0x7f;  // 
    };
    displayNumColor[3] = textCode;
    display.display(displayNumColor);
  }
  else if (textCode == 10) {          // Display "ALL"
    display.display(DISPLAYALL);
  }
  else if (textCode == 11) {          // Display "M-F"
    display.display(DISPLAYWEEK);
  }
  else if (textCode == 12) {          // Display "SASU"
    display.display(DISPLAYSASU);
  }
  else if (textCode == 13) {          // Display "br 1"
    display.display(DISPLAYL1);
  }
  else if (textCode == 14) {          // Display "br 2"
    display.display(DISPLAYL2);
  }
  else if (textCode == 15) {          // Display "br 3"
    display.display(DISPLAYL3);
  }
  else if (textCode == 16) {          // Display "br 4"
    display.display(DISPLAYL4);
  }
  else if (textCode == 17) {          // Display "br 5"
    display.display(DISPLAYL5);
  }
  else if (textCode == 18) {          // Display "LOVE"
    display.point(POINT_OFF);
    display.display(DISPLAYLOVE);
  }
}

void DisplayColor(int red, int green, int blue, int brightness)
{
  analogWrite(redPin, red/1023.*BRIGHTNESSLEVEL[brightness]);
  analogWrite(greenPin, green/1023.*BRIGHTNESSLEVEL[brightness]);
  analogWrite(bluePin, blue/1023.*BRIGHTNESSLEVEL[brightness]);  
}

void ColorTesterSalute()
{
  DisplayText(18,0);
  for (int i = 0; i < totalColors; i++) {
    DisplayColor(MYCOLORS[i][0],MYCOLORS[i][1],MYCOLORS[i][2],L5BRIGHTNESS);
    delay(1000);
  }
  for (int i = 0; i < totalColors; i++) {
    DisplayColor(MYCOLORS[i][0],MYCOLORS[i][1],MYCOLORS[i][2],L5BRIGHTNESS);
    delay(750);
  }
  for (int i = 0; i < totalColors; i++) {
    DisplayColor(MYCOLORS[i][0],MYCOLORS[i][1],MYCOLORS[i][2],L5BRIGHTNESS);
    delay(500);
  }
  for (int j = 0; j < 16; j++) {
    for (int i = 0; i < totalColors; i++) {
      DisplayColor(MYCOLORS[i][0],MYCOLORS[i][1],MYCOLORS[i][2],L5BRIGHTNESS);
      delay(75);
    }
  }
  menu = 0;
}

/*************************************************************************
//
//  EEPROM FUNCTIONS
//
//************************************************************************/

void longTermSave() {
  // Saves the colorArray of colors into the EEPROM memory
  for (int i = 0; i < numColors; i++) {
    for (int j = 0; j < numSettings; j++) {
      byte sendByte = colorArray[i][j];
      int memLoc = (i * numSettings) + j;  
      EEPROM.write(memLoc,sendByte);
    }
  }
  // Saves the number of Color Alarms enabled
  byte sendByte = numColorAlarms;
  int memLoc = (numColors * numSettings);  
  EEPROM.write(memLoc,sendByte);
}

void longTermRetrieve() {
  // Reads the saved colorArray from EEPROM into memory
  for (int i = 0; i < numColors; i++) {
    for (int j = 0; j < numSettings; j++) {
      int memLoc = (i * numSettings) + j;  
      byte getByte = EEPROM.read(memLoc);
      colorArray[i][j] = getByte;
    }
  }
  // Retrieves the number of Color Alarms enabled
  int memLoc = (numColors * numSettings);  
  numColorAlarms = EEPROM.read(memLoc);
}

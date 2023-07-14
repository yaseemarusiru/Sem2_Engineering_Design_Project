//----HX711 Amplifier--------
#include "HX711.h"
#include <SoftwareSerial.h>

//----OLED Dislpay-----------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//----Time Libraries---------
#include <Time.h>
#include <TimeLib.h>

//----HX711 Amplifier---------
#define DT 8
#define SCK 4

//----------OLED Dislpay-------------
#define SCREEN_WIDTH 128 //In pixels
#define SCREEN_HEIGHT 64 //In pixels

//Hardware SPI
#define OLED_DC     9
#define OLED_CS     A2
#define OLED_RESET  10
//scl  13
//sda  11

// Number of snowflakes in the animation
#define NUMFLAKES     30 
//Logo dimensions
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
//For the drawSnowflakes function
#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

HX711 loadCell;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

SoftwareSerial mySerial(2,3);

String data = "" ;
char c;
float scalingFactor = 400; //538
float weight = 50000; //dummy weight - should take from wifi module
float recomendedAmount = weight*0.067;
float percentageOfWater = 0;
float w1,w2;
float consumptionTotal = 0;
float prevConsumptionTotal = 0;

bool flag1 = true, flag5 = true;
int x=14+ 46*(1-0.01*percentageOfWater);
int m, currentHour1, currentMinute1;

char teamName[]={'S','P','A','R','T','A','N','S'};
String stratString[]={" Smart"," Water","Traking"," Device"};
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

void setup() {

  loadCell.begin(DT,SCK);  //initialize the pin modes and sets the gain to 128
  loadCell.tare();  //0 will be the weight measured at this time
  loadCell.set_scale(scalingFactor);  //used to divide the reading to measure weight in grams
  
  mySerial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC)) { //wait unitil OLED is ready
    for(;;);
  }

  //initialize with the Adafruit flash screen
  display.display();
  delay(1000); // Pause for 1 seconds

  //display team name
  displaySpartans();
  display.invertDisplay(false);
  display.clearDisplay();

  //display starting string
  displayStartString();
  display.clearDisplay();

  //get initial weight reading
  w1 = loadCell.get_units(7); //initial weight
}

void loop() {
  
  delay(4000);  //to obtain accurate readings from the Load Cell
  
  //read serial monitor to check for any msg from wifi module
  while(mySerial.available()>0){
      delay(10);
      c = mySerial.read();
      data += c; 
  } 
  
  //if msg's first chararcter is "W" it's the weight user entered through telegram
  if (data[0]=="W"){
    int w1 = data[1] - '0';
    int w2 = data[2] - '0';
    weight = 10*w1+w2;
    data = "";
  }
  
  //if msg's first chararcter is "T" it's the current time obtained by NTP through WiFi module
  if ((data[0]=='T') && flag5) {
    int t1 = data[1] - '0';
    int t2 = data[2] - '0';
    int t3 = data[3] - '0';
    int t4 = data[4] - '0';
    int t5 = data[5] - '0';
    int t6 = data[6] - '0';
    setTime(10*t1+t2,10*t3+t4,10*t5+t6,12,7,23);
  
    data = "";
    flag5 = false;  //to make sure that the time is obtained from the WiFi module only once
  }

  display.clearDisplay();
  display.setTextColor(WHITE,BLACK);
  
  data = "";

  w2 = loadCell.get_units(7);

  displayConsumption();  //display the precentage of water consumed in the OLED screen

  if (w2<10) {  //if the weight difference is a lesser number it's probably because of a small reading error due to the environmental conditions
    w2=w1;
  }

  if (w1-w2>20) {  //to neglect such errors
    prevConsumptionTotal = consumptionTotal;  //keep track of previous consumption to send msg to the user
    consumptionTotal += (w1-w2);  //update the change
    percentageOfWater = consumptionTotal*100/recomendedAmount;
  }

  w1 = w2;
  if (prevConsumptionTotal!=consumptionTotal){  //if there's a consumption of water, inform user about it
    prevConsumptionTotal = consumptionTotal;
    int consumptionInt = consumptionTotal;
    mySerial.print(consumptionInt);
  }
}

void displaySpartans(){
  for (int i=0;i<8;i++) {
    display.clearDisplay();
    display.drawRect(0,0,128,64,WHITE);
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(50,20);
    display.print(teamName[i]);
    display.invertDisplay(true);
    display.display();
    delay(200);
  }
}

void displayStartString(){
  for (int i=0;i<4;i++){
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE,BLACK);
    display.setCursor(0,20);
    display.println(stratString[i]);
    display.display();
    delay(500);
  }
  display.clearDisplay();
  return;  
}

void displayConsumption(){
  if (flag1) {
    m=0;
    flag1 = false; 
  }
  if((m!=x) && (flag1)){
    drawSnowflakes(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); 
    m=x;
  }

  x=14+ 46*(1-0.01*percentageOfWater);//set coordinate
  if(percentageOfWater<101){
    display.clearDisplay();
    display.drawRoundRect(0,14,40,49,5,WHITE);
    display.fillRoundRect(4,x,32,(60-x),5,WHITE);
    display.drawRect(16,11,8,3,WHITE);
    display.fillRect(16,11,8,3,WHITE);
    display.setCursor(50,30);
    display.setTextSize(4);
    int perc = percentageOfWater;
    display.print(perc);//display percentage
    display.print("%");
    displayTime();
    display.display();
    delay(200);
  }

  else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10,0);
    display.println();
    display.println(" You Have");
    display.println("  Drunk ");
    display.println(" Enough");
    displayTime();
    display.display();
    delay(1500);
  }
}

void displayTime(){
  display.setCursor(40,0);
  display.setTextSize(2);

  currentHour1 = hour();
  currentMinute1 = minute();
  if ((currentHour1==0) && (currentMinute1==0)){
    display.setTextSize(1);
    display.print("SPARTANS");
    display.display();
  }
  else {
  if(currentHour1<12){
    display.print(currentHour1);
    display.print(":");
    if (currentMinute1<10){
      display.print("0");
    }

    display.print(currentMinute1);
    
    display.print("am");  //display time am
    display.display();
  }
  else if (currentHour1==12) {
    display.print(currentHour1);
    display.print(":");
    if (currentMinute1<10){
      display.print("0");
    }

    display.print(currentMinute1);
    
    display.print("pm");  //display time pm
    display.display();
  }
  else {
    display.print(currentHour1-12);
    display.print(":");
    if (currentMinute1<10){
      display.print("0");
    }

    display.print(currentMinute1);
    
    display.print("pm");  //display time pm
    display.display();
  }
  }
}

void testdrawbitmap() {
  display.clearDisplay();
  display.drawBitmap((display.width()  - LOGO_WIDTH ) / 2, (display.height() - LOGO_HEIGHT) / 2,logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}

void drawSnowflakes(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
  }

  for(int k=0;k<25;k++) { // Loop forever...
    display.clearDisplay(); // Clear the display buffer

    // Draw each snowflake:
    for(f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    display.display(); // Show the display buffer on the screen
    delay(200);        // Pause for 1/10 second

    // Then update coordinates of each flake...
    for(f=0; f< NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS]   = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}

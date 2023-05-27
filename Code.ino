#include "HX711.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define DT A2 
#define SCK A3 
#include <SoftwareSerial.h>
#define SCREEN_WIDTH 128 //In pixels
#define SCREEN_HEIGHT 64 //In pixels
#define OLED_DC     9
#define OLED_CS     12
#define OLED_RESET  10
#define NUMFLAKES     30 
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
#define XPOS   0 
#define YPOS   1
#define DELTAY 2

HX711 loadCell;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

SoftwareSerial mySerial(2,3);

float scalingFactor = 537;
String stringWeight;
int weight = atoi(stringWeight.c_str());
float recomendedAmountPerHour = weight*0.033*1000/16;
int percentageOfWater = 0;
float w1,w2;
float consumptionTotal = 0;
float consumptionPerHour = 0;
int prevHour=0, currentHour=0;
int prevMinute=0, currentMinute=0;
bool flag1 = true;
int x=14+ 46*(1-0.01*percentageOfWater);
int m;
String data ;
String newcp =data.substring(0,5);
char c;

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

  //Initializing for the WiFi module
  mySerial.begin(115200);
  //while(!display.begin(SSD1306_SWITCHCAPVCC)) {
    //for(;;); // If display is not ready, don't proceed, loop forever
  //}
  //the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 1 seconds
  //display team name
  displaySpartans();
  display.invertDisplay(false);
  display.clearDisplay();

  //display starting string
  displayStartString();
  display.clearDisplay();
  delay(200);

  loadCell.begin(DT,SCK);  //initialize the pin modes and sets the gain to 128
  loadCell.tare();  //0 will be the weight measured at this time
  loadCell.set_scale(scalingFactor);  //used to divide the reading to measure weight in grams
  w1 = loadCell.get_units(7); //initial weight
  while (!started());
  while (!weightEntered());
}

void loop() {
  mySerialRead();
  while (readTime());
  displayTime();
  mySerialRead();
  showConsumptionOnTelegram();
  w2 = loadCell.get_units(7);
  //updating a consumption
  if (w1-w2>10) {
    consumptionPerHour += (w1-w2) ;
    consumptionTotal += (w1-w2);
    percentageOfWater = consumptionTotal/weight*0.033*1000*100;
  }
 displayConsumption();
  //checking if 1 hour has passed since the user started the device
  if ((currentHour-prevHour == 1) && (currentMinute-prevMinute == 0)){
    if (consumptionPerHour - recomendedAmountPerHour > 20) {
      displayConsumedRecAmount();
      mySerial.print("AA");
    }
    else {
      displayNotConsumedRecAmount();
      mySerial.print("BB");
    }
    consumptionPerHour = 0;
  }
  w1 = w2;
}


void mySerialRead(){
  while(mySerial.available()>0){
    delay(10);
    c = mySerial.read();
    data += c;
  } 
}

bool started(){
  mySerialRead();
  if (data.length()>0) {
    if (newcp == "START") {
      data = "";
      return true;
    }
    data = "";
  }
  return false;
}

bool weightEntered(){
  mySerialRead();
  if (data.length()>0) {
    if (isDigit(data[0]) && isDigit(data[1]) && data.length()<3) {
      stringWeight = data;
      data = "";
      return true;
    } 
  return false;
  }
}

void showConsumptionOnTelegram(){
  if (data.length()>0) {
     if (data == "CU") {  //current consumption
      mySerial.print(percentageOfWater);
      data = "";
    } 
    else if (data == "CL") {  //left to consume
      mySerial.print(100 - percentageOfWater);
      data = "";
    }
    data = "";
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
}

void displayConsumption(){
  if (flag1) {
    m=0;
    flag1 = false; 
  }

  if(m!=x){
    drawSnowflakes(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); 
    m=x;
  }

  x=14+ 46*(1-0.01*(percentageOfWater));//set coordinate
  if(percentageOfWater<101){
    display.clearDisplay();
    display.drawRoundRect(0,14,40,49,5,WHITE);
    display.fillRoundRect(4,x,32,(60-x),5,WHITE);
    display.drawRect(16,11,8,5,WHITE);
    display.fillRect(16,11,8,5,WHITE);
    display.setCursor(50,30);
    display.setTextSize(4);
    display.print(percentageOfWater);//display percentage
    display.print("%");
    displayTime();
    display.display();
    delay(2000);
    //display.clearDisplay();
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

bool readTime(){
  if (data.substring(0,5)=="ASDRT"){
    prevHour = currentHour;
    prevMinute = currentMinute;
    String l=data.substring(5,9);
    String p = data.substring(5,9);
    currentHour = (int) atof(l.substring(0,2).c_str());
    currentMinute = (int) atof(p.substring(2,4).c_str());
    data = "";
    return true;
  }
  return false;
}

void displayTime(){
  display.setCursor(44,0);
  display.setTextSize(2);

  if(currentHour<13){
    display.print(currentHour+String(".")+currentMinute+String("am"));  //display time am
    display.display();
  }
  else {
    display.print(currentHour+String(".")+currentMinute+String("pm"));  //display time pm
    display.display();
      display.print("ok");
  }

  delay(300);
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
    delay(100);        // Pause for 1/10 second

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

void displayConsumedRecAmount(){
  testdrawbitmap();
  display.drawCircle(64, 32, 25, WHITE);
  display.drawCircle(64, 32, 26, WHITE);
  display.drawCircle(64, 32, 27, WHITE);
  display.drawCircle(64, 32, 28, WHITE);
  display.display();
  delay(1500);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10,0);
  display.println();
  display.println("You Have");
  display.println(" Drunk Enough");
  display.println("for Hour!");
  display.display();
  delay(1500);
}

void displayNotConsumedRecAmount(){ 
  testdrawbitmap();  
  display.drawCircle(64, 32, 25, WHITE);   // Draw circles 
  display.drawCircle(64, 32, 26, WHITE);
  display.drawCircle(64, 32, 27, WHITE);
  display.drawCircle(64, 32, 28, WHITE);
  display.display();
  delay(1500);
  display.clearDisplay();
  display.invertDisplay(true);
  display.setTextSize(2);
  display.setCursor(10,0);
  display.println();
  display.println("You Have");
  display.println("Not Drunk");
  display.println(" Enough");
  display.display();
  display.invertDisplay(false);
  delay(10000);
}
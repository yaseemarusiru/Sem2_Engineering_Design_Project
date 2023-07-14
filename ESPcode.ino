#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Wifi network station credentials
#define WIFI_SSID "Redmi 8A Dual 1"
#define WIFI_PASSWORD "adee15057e3a"
// Telegram BOT Token (from Botfather)
#define BOT_TOKEN "5740874696:AAGWgx4S0qCcXfDkugcrJXca9TNVhOHpB-0"

const unsigned long BOT_MTBS = 200; // mean time between scan messages 1000 before 200 now
String chat_id = "1467323353";
String data;
char c;
unsigned long bot_lasttime;          // last time messages' scan has been done

String currhour1; 
String minutes1; 
String secs1;

int currhour;
int minutes;
int secs;

const long off = 19800; // next 3 lines are for time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",off);

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void handleNewMessages(int numNewMessages) {

  for (int i=0; i<numNewMessages; i++) {

    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;

    if (from_name == "") from_name = "Guest";

    if (isDigit(text[0]) && isDigit(text[1]) ) {
      bot.sendMessage(chat_id,"You just entered your weight.");
      Serial.print("W"+text);
    }

  }

}
void arduinomsg(){
  while(Serial.available()>0){
        delay(10);
        c = Serial.read();
        data += c;
  } 

  if (data=="0.00"){
    data = "";
  }
  if (data.length()>0){
    bot.sendMessage(chat_id,"You have drunk "+data+"ml for the day. Keep going :)"); 
    data="";
  }
  data = "";
}
void setup()
{
  Serial.begin(9600);
  
  // attempt to connect to Wifi network:
  configTime(19800, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
 
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  time_t now = time(nullptr);

  while (now < 24 * 3600)
  {
    delay(100);
    now = time(nullptr);
  }

  String wel = "Welcome to Aqua Tracking Bot. You'll get hourly updates of your consumption through me :) \n";
  wel += "\n";
  wel += "Enter your weight in kgs to get the correct recommendations. You can always change the value just by sending it to me";
  wel += "\n";
  wel+= "Countdown starts now";
  bot.sendMessage(chat_id, wel);
  timeClient.begin();
  timeClient.setTimeOffset(19800);
  timeClient.update();
}

void loop()
{
  currhour = timeClient.getHours();
  minutes = timeClient.getMinutes();
  secs = timeClient.getSeconds();
  if (currhour<10) {
    currhour1 += '0';
    currhour1 += String(currhour);
  }
  else {
    currhour1 += String(currhour);
  }

  if (minutes<10) {
    minutes1 += '0';
    minutes1 += String(minutes);
  }
  else {
    minutes1 += String(minutes);
  }

  if (secs<10) {
    secs1 += '0';
    secs1 += String(secs);
  }
  else {
    secs1 += String(secs);
  }

  Serial.print("T"+currhour1+minutes1+secs1);
  
  currhour1 = "";
  minutes1 = "";
  secs1 = "";
  
  if (millis() - bot_lasttime > BOT_MTBS)
  {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  arduinomsg();
}

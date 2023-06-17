#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//for time


// Wifi network station credentials
#define WIFI_SSID "Redmi 8A Dual 1"
#define WIFI_PASSWORD "adee15057e3a"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "5740874696:AAGWgx4S0qCcXfDkugcrJXca9TNVhOHpB-0"

const unsigned long BOT_MTBS = 200; // mean time between scan messages 1000 before 200 now
String chat_id = "1467323353";
String data;
char c;
unsigned long bot_lasttime;          // last time messages' scan has been done

const long off = 19800; // next 3 lines are for time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",off);

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

//String test_photo_url = "https://www.arduino.cc/en/uploads/Trademark/ArduinoCommunityLogo.png";

void handleNewMessages(int numNewMessages) {
  //Serial.print("handleNewMessages ");
  //Serial.println(numNewMessages);

  for (int i=0; i<numNewMessages; i++) {
    //String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/start") {
      bot.sendMessage(chat_id,"Starting your device for the day");
      //Serial.print("START");
    }
    // if (text == "/stop") {
    //   bot.sendMessage(chat_id,"Stopping your device for the day");
    //   Serial.print("STOP");
    // }
    // if (text == "/cu") {
    //   Serial.print("CU");
    //   delay(500);
    //   while(Serial.available()>0){
    //     delay(10);
    //     c = Serial.read();
    //     data += c;
    //   } 
    //   bot.sendMessage(chat_id,"Current consumption ",data); 
    //   data="";
    // }
    // if (text == "/cl") {
    //   Serial.print("CL");
    //   delay(500);
    //   while(Serial.available()>0){
    //     delay(10);
    //     c = Serial.read();
    //     data += c;
    //   } 
    //   bot.sendMessage(chat_id,"Left to consume ",data); 
    //   data="";
    // }
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
      if (data=="100"){
      bot.sendMessage(chat_id,"You have drunk enough water for the past hour. Keep going :)"); 
      data="";
      }
      if (data.length()>0){
        bot.sendMessage(chat_id,"You have drunk"+data+" for the past hour"); 
      data="";
      }
}
void setup()
{
  Serial.begin(9600);
  //Serial.println();

  // attempt to connect to Wifi network:
  configTime(19800, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  // Serial.print("Connecting to Wifi SSID ");
  // Serial.print(WIFI_SSID);
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    //Serial.print(".");
    delay(500);//
  }
  //WiFi.enableInsecureWEP(true);
  //Serial.print("\nWiFi connected. IP address: ");
  //Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  //Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    //Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  //Serial.println(now);
  String wel = "Welcome to Aqua Tracking Bot. You'll get hourly updates of your consumption through me :) \n";
  wel += "\n";
  wel += "Enter your weight in kgs to get the correct recommendations. You can always change the value just by sending it to me";
  bot.sendMessage(chat_id, wel);
  String welo = "Countdown starts now";
  bot.sendMessage(chat_id, welo);
  timeClient.begin();
  timeClient.setTimeOffset(19800);


}

void loop()
{
  timeClient.update();
  int currhour = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int secs = timeClient.getSeconds();
  Serial.print(String(currhour)+String(minutes)+String(secs));
  //Serial.print("ASDRT"+ (String) timeClient.getFormattedTime());
  // time_t now = time(nullptr);
  // int our = hour(now);
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    //timeClient.update();
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  arduinomsg();
}

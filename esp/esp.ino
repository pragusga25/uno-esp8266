#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SoftwareSerial.h>
#include <DHT.h>

SoftwareSerial mcu(D6, D5);
#define DHTPIN D7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
// Wifi network station credentials
#define WIFI_SSID "taufik"
#define WIFI_PASSWORD "pass12345"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "ddddd"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long bot_lasttime;          // last time messages' scan has been done
bool Start = false;
String chat_id;
float hum, temp;
String data = "";
String sensorInfo[5]; // LPG CO SMOKE HUM TEMP
const String INFO_PREF = "INFO: ";

void typingAction(int delay_time){
  bot.sendChatAction(chat_id, "typing");
  delay(delay_time);
}

void sendMsg(String msg){
  bot.sendMessage(chat_id, msg);
}

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if(text == "/help"){
      String help = "Berikut command-command yang dapat kamu gunakan: \n\n";
      help += "\\buzzon: menyalakan buzzer\n";
      help += "\\buzzoff: mematikan buzzer\n";
      help += "\\info: mendapatkan nilai sensor\n";
      sendMsg(help);
    }

    if(text == "/buzzon"){
      mcu.println("buzzon");
      sendMsg("Buzzer Dinyalakan");
    }

    if(text == "/buzzoff"){
      mcu.println("buzzoff");
      sendMsg("Buzzer Dimatikan");
    }

    if(text == "/info" && !isnan(temp) && !isnan(hum)){
      String infoStr = "LPG: " + sensorInfo[0] + "PPM\n";
      infoStr += "CO: " + sensorInfo[1] + "PPM\n";
      infoStr += "ASAP: " + sensorInfo[2] + "PPM\n";
      infoStr += "KELEMBAPAN: " + sensorInfo[3] + "RH\n";
      infoStr += "SUHU: " + sensorInfo[4] + "°C\n";

      sendMsg(infoStr);
    }
  }
}


void setup(){
  Serial.begin(9600);
  mcu.begin(9600);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  Serial.print(" ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  
  client.setInsecure();
  Serial.println();

  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(){
  if (millis() - bot_lasttime > BOT_MTBS){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

    data = "";
    while(mcu.available() > 0){
      delay(10);
      data += char(mcu.read());
    }
    data.trim();

    // if(data != "") Serial.println(data);
    if(data == "warn:fire"){
      Serial.println("HELLO");
      sendMsg("WARNING!!! ADA API YANG MENYALA!!!");
    }else if(data == "fire!!!"){
      sendMsg("TERJADI KEBAKARAN!!! LARI!!!");
    }else if(prefix(INFO_PREF.c_str(), data.c_str())){
      parseData();
    }

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if(!isnan(h) && !isnan(t)){
      hum = h;
      temp = t;
      mcu.println("DHT: " + String(temp) + "-" + String(hum));    
    }
}

void parseData(){
  data.replace("INFO: ", "");

  int dataLen = data.length();

  char charArr[dataLen];
  data.toCharArray(charArr, dataLen + 1);
  
  char *ptr = strtok(charArr, " ");
  for(int i = 0; i < 3; i++){
    sensorInfo[i] = ptr;
    ptr = strtok(NULL, " ");
  }
  sensorInfo[3] = String(hum);
  sensorInfo[4] = String(temp);
}

bool prefix(const char *pre, const char *str){
  return strncmp(pre, str, strlen(pre)) == 0;
}


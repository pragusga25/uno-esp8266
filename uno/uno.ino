#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <MQ2.h>

#define BUZZ_PIN 4
#define FLAME_PIN A0
#define MQ2_PIN A1
#define RELAY_PIN 5
#define LED_PIN 6

const String DHT_PREF = "DHT: ";
const float SMOKE_TRESH = 120;
String data = "";
int fire;
float lpg, co, smoke;
float temp = -1;
float hum = -1;
bool persistBuzz = false;

SoftwareSerial uno(2, 3);
MQ2 mq2(MQ2_PIN);

void readData(void *pvParameters);
void readSensors(void *pvParameters);
void sendData(void *pvParameters);

void setup() {
  Serial.begin(9600);
  uno.begin(9600);
  mq2.begin();
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  xTaskCreate(readData, "readData", 128, NULL, 1, NULL);
  xTaskCreate(readSensors, "readSensors", 128, NULL, 1, NULL);
}

void loop() {}

void readData(void *pvParameters){
  while(1){
    data = "";
    while(uno.available() > 0){
      delay(10);
      data += char(uno.read());
    }
    data.trim();

    if(data == "") continue;

    if(data == "buzzon") buzzOnPersist();
    else if(data == "buzzoff") buzzOffPersist();
    else if(prefix(DHT_PREF.c_str(), data.c_str())){
      data.replace("DHT: ", "");
      int idx = data.indexOf('-');
      temp = data.substring(0, idx).toFloat();
      hum = data.substring(idx + 1, data.length()).toFloat();
    }
    else uno.println("404");

    vDelay(1000);
  }
}

void readSensors(void *pvParameters){
  while(1){
    fire = analogRead(FLAME_PIN);
    lpg = mq2.readLPG();
    co = mq2.readCO();
    smoke = mq2.readSmoke();
    
    if(smoke > SMOKE_TRESH && fire < 100){
      fireOn();
      uno.println("fire!!!");
    }else if(fire >= 100 && smoke <= SMOKE_TRESH){
      fireOff();
    }else if(fire < 100 && smoke <= SMOKE_TRESH){
      uno.println("warn:fire");
    }else{
      String sendStr = "INFO: " + String(lpg) + " " + String(co) + " " + String(smoke);
      uno.println(sendStr);
    }

    vDelay(500);
  }
}


void fireOn(){
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZ_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
}

void fireOff(){
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  if(!persistBuzz) digitalWrite(BUZZ_PIN, LOW);
}

void buzzOnPersist(){
  persistBuzz = true;
  digitalWrite(BUZZ_PIN, HIGH);
}

void buzzOffPersist(){
  persistBuzz = false;
  digitalWrite(BUZZ_PIN, LOW);
}

void vDelay(int ms){
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

bool prefix(const char *pre, const char *str){
    return strncmp(pre, str, strlen(pre)) == 0;
}
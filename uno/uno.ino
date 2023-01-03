#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <MQ2.h>

#define BUZZ_PIN 4
#define FLAME_PIN A0
#define MQ2_PIN A1

String data = "";
int fire;
float temp, hum, lpg, co, smoke;

SoftwareSerial uno(2, 3);
MQ2 mq2(MQ2_PIN);

void buzzer(void *pvParameters);
void readData(void *pvParameters);
void flame(void *pvParameters);
void humtemp(void *pvParameters);
void sendData(void *pvParameters);

void setup() {
  Serial.begin(9600);
  uno.begin(9600);
  mq2.begin();

  xTaskCreate(readData, "readData", 128, NULL, 1, NULL);
  xTaskCreate(buzzer, "buzzer", 128, NULL, 1, NULL);
  xTaskCreate(flame, "flame", 128, NULL, 1, NULL);
  xTaskCreate(humtemp, "humtemp", 128, NULL, 1, NULL);
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

    if(data != ""){
      vDelay(1000);
    }
  }
}

void buzzer(void *pvParameters){
  pinMode(BUZZ_PIN, OUTPUT);
  while(1){
    if(data == "buzzon"){
      digitalWrite(BUZZ_PIN, HIGH);
    }

    if(data == "buzzoff"){
      digitalWrite(BUZZ_PIN, LOW);
    }    
  }
}

void flame(void *pvParameters){
  pinMode(FLAME_PIN, INPUT);
  while(1){
    fire = analogRead(FLAME_PIN);
    Serial.println("Api: " + String(fire));
    if(fire < 100){
      uno.println("warn:fire");
    }

    lpg = mq2.readLPG();
    co = mq2.readCO();
    smoke = mq2.readSmoke();

    Serial.println("CO: " + String(co));
    Serial.println("LPG: " + String(lpg));
    Serial.println("Asap: " + String(smoke));

    vDelay(1000);
  }
}

void humtemp(void *pvParameters){
  while(1){
    String pref = "DHT: ";
    if(prefix(pref.c_str(), data.c_str())){
      data.replace("DHT: ", "");
      int idx = data.indexOf('-');
      temp = data.substring(0, idx).toFloat();
      hum = data.substring(idx + 1, data.length()).toFloat();
      Serial.println("Temperatur: " + String(temp) + "°C");
      Serial.println("Kelembapan: " + String(hum) + "RH");
    }
    vDelay(2000);
  }
}

void sendData(void *pvParameters){
  while(1){
    if(data == "temp"){
      uno.println("temp: " + String(temp));
    }

    if(data == "hum"){
      uno.println("hum: " + String(hum));
    }
  }
}

void vDelay(int ms){
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

bool prefix(const char *pre, const char *str){
    return strncmp(pre, str, strlen(pre)) == 0;
}
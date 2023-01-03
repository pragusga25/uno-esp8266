#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <MQ2.h>

#define BUZZ_PIN 4
#define FLAME_PIN A0
#define MQ2_PIN A1

const String DHT_PREF = "DHT: ";
String data = "";
int fire;
float lpg, co, smoke;
float temp = -1;
float hum = -1;

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
    Serial.println("Data Diterima: " + data);
    if(data == "buzzon") digitalWrite(BUZZ_PIN, HIGH);
    else if(data == "buzzoff") digitalWrite(BUZZ_PIN, LOW);
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

    if(temp != -1 && hum != -1){
      Serial.println("Temperatur: " + String(temp));
      Serial.println("Kelembapan: " + String(hum));
    }

    vDelay(1000);
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
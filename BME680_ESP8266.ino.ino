
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#include <Wire.h>
#include <SPI.h>


#include<ESP8266WiFi.h>

const char* ssid="xxx";//the name of your network
const char* password="xxxx";//the name of yourk network password
const char* server = "api.thingspeak.com";

String apiKey="xxxx"; ///thingspeak api key

WiFiClient client;

Adafruit_BME680 bme;
#define SEALEVELPRESSURE_HPA (1013.25) ///1009.48
void Weather_Connection(void);

void connect(){
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifiConnectStart=millis();
  while(WiFi.status()!=WL_CONNECTED){
    if(WiFi.status()==WL_CONNECT_FAILED){
      Serial.println("Failed to connect to WiFi!");
      delay(10000);
    }

    delay(500);
    Serial.println("...");
    //only try for 5 seconds
      if(millis()-wifiConnectStart>15000){
        Serial.println("Failed to connect to WiFi!!!");
        return;
      }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  connect();
  Weather_Connection();
}

void Weather_Connection()
{
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }


  Serial.print(F("Temperature = "));
  float t=bme.temperature;
  Serial.print(t);
  Serial.println(F(" *C"));

  float p=bme.pressure;
  Serial.print("Pressure = ");
  Serial.print(p/100.0);
  Serial.print(" hPa / ");
  Serial.print((p/100.0)*0.75006);
  Serial.println(" mmHg");

  float h=bme.humidity;
  Serial.print(F("Humidity = "));
  Serial.print(h);
  Serial.println(F(" %"));

  float g=bme.gas_resistance;
  Serial.print(F("Gas = "));
  Serial.print(g / 1000.0);
  Serial.println(F(" KOhms"));

  float alt=bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print(F("Approx. Altitude = "));
  Serial.print(alt);
  Serial.println(F(" m"));

  Serial.println();

  if(client.connect(server,80)){
    String postStr=apiKey;

    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(p/100.0);
    postStr +="&field3=";
    postStr +=String(h);
    postStr +="&field4=";
    postStr +=String(g);
    postStr +="&field5=";
    postStr +=String(alt); 
    postStr += "\r\n\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();

  ESP.deepSleep(300e6);
}

void loop() {
  
}

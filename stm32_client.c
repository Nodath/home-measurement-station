#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <STM32Ethernet.h>

#include "DHT.h"

#include <PubSubClient.h>

#define LEDPIN 5

#define DHTPIN A0 
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE); 


long lastMsg = 0;
char msg[50];
const char topic[] = "test/message";
const char server[] = "192.168.43.196";

EthernetClient ethClient;
PubSubClient client(ethClient);


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Nucleo-144")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}



void setup() {

  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  client.setServer(server, 1883);
  Serial.println("Connecting");
  while(Ethernet.begin() == 0) {
    Serial.print(".");
  }
  Serial.println("Connected");
  dht.begin();  
}
void sendMsg(float h, float t){
  String temp = String(t);
  String humi = String(h);
  String result = temp + " " + humi;

  Serial.println("MQTT connected");
  client.publish(topic, String(result).c_str(), true);
  client.subscribe(topic);
}

void loop() {
  float h = dht.readHumidity();       //Gets Humidity value
  float t = dht.readTemperature();    //Gets Temperature value
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  if (!client.connected()) {
    reconnect();
  }

  sendMsg(h, t);
  
  client.loop();
  delay(500);
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT11

const char* ssid="esp";
const char* password = "haslo8266";
const char *serverHostname = "192.168.43.196";
const char *topic = "test/message";
const char * clientId = "ESP8266";
// DHT Sensor
uint8_t DHTPin = D3;
 
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  
  Serial.begin(115200);
  Serial.println();

  pinMode(DHTPin, INPUT);

  dht.begin();  
  
  Serial.print("Wifi connecting to ");
  Serial.println( ssid );

  WiFi.begin(ssid,password);

  Serial.println();
  Serial.print("Connecting");

  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
  }

  Serial.println();

  Serial.println("Wifi Connected Success!");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP() );
  sendMsg();
  client.setServer(serverHostname, 1883);
  client.setCallback(callback);
  
  
}

void sendMsg(){
    Temperature = dht.readTemperature();
    Humidity = dht.readHumidity();
  
    Serial.println(Temperature);
    Serial.println(Humidity);
    String temp = String(Temperature);
    String humi = String(Humidity);
    String result = temp + " " + humi;
    
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("MQTT connected");
      // Once connected, publish an announcement...
      client.publish(topic, String(result).c_str(), true);
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.printf("MQTT failed, state %s, retry \n", client.state());
      // Wait before retrying
      delay(2500);
    }
 
}

// connect to MQTT server
void connectMQTT() {
  sendMsg();
  // Wait until we're connected
  while (!client.connected()) {
    // Create a random client ID
    //clientId += String(random(0xffff), HEX);
    Serial.printf("MQTT connecting as client %s...\n", clientId);
  }
  
}

void callback(char *topic, byte *payload, unsigned int length) {
  // copy payload to a static string
  static char message[69+1];
  if (length > 69) {
    length = 69;
  }
  strncpy(message, (char *)payload, length);
  message[length] = '\0';
  
  Serial.printf("topic %s, message received: %s\n", topic, message);

}

// the loop function runs over and over again forever
void loop() {
    if (!client.connected()) {
      connectMQTT();
    }
    sendMsg();
    client.loop();
    delay(500);
}

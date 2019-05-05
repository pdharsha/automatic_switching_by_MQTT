#include <PubSubClient.h>
#include <ESP8266WiFi.h>


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "192.168.43.167"
const char* ssid = "Deva";
const char* password = "deva123456";

//LED on ESP8266 GPIO2
const int obstaclePin = D2;

//topic to publish to for controlling the other ESP module
char* lightTopic = "/house/light1";




WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
  //initialize the button pin as an input
  pinMode(obstaclePin, INPUT);

 
  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);
  //start wifi subsystem
  WiFi.begin(ssid, password);

  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();

  //wait a bit before starting the main loop
      delay(2000);
}


void ir() {
  if (digitalRead(obstaclePin) == LOW) 
  {
    client.publish(lightTopic, "1");
    Serial.println("Clear");
    
  }
  else if(digitalRead(obstaclePin) == HIGH)
  {
    client.publish(lightTopic, "0");
    Serial.println("Something is ahead!!");
  }
  delay(200);
}

void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //monitor the button
  ir();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
}


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  //we're not subscribed to any topics so we dont need to have anything here
}


//networking functions

void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        Serial.print("\tMTQQ Connected");
        client.subscribe(lightTopic);
      }

      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); 
      //abort();
      }
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}

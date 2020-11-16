#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>

static String myID = "PumpRelay_v1";
String fA = "\xF0";
char fC = 0;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
float vB[] = {999, 999, 999, 999, 999};


//Router
int port = 3011;
IPAddress ip(192, 168, 10, 101);
EthernetServer server(port);

//Relay Pin Number
const int relayPin = 6;
String rcvbuf = "";
DynamicJsonDocument pSignal(1024);
// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;
// delay between updates, in milliseconds
const unsigned long postingInterval = 10000;  



void setup() {
  
  Serial.begin(9600);  
  Serial.println("Initialize Ethernet with Arduino Ethernet Shield Host: " + myID);
  // start the Ethernet connnection and the server
  Ethernet.begin(mac, ip);
  
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  // Relay signla pin output
  pinMode(relayPin, OUTPUT); 
}

void loop() {
  EthernetClient client = server.available();
    if (client) {
      while (client.connected()) {
        if (client.available()) {         
          char c = client.read();
          rcvbuf += c;
        }
      }
      // give the web browser time to receive the data
      delay(1000);
      client.stop();
      int rcvbuf_len = rcvbuf.length() + 1;
      char json[rcvbuf_len];
      rcvbuf.toCharArray(json, rcvbuf_len);
      deserializeJson(pSignal,json);
      const char* Timestamp = pSignal["Timestmap"];
      int PumpOn = pSignal["PumpOn"];
      Serial.println(PumpOn);
//      rcvbuf = "";
      
      PumpRelay(PumpOn);

  }          
}




void PumpRelay(int PumpOn){
  if (PumpOn == 1){
    // note the time that the connection was made:
    lastConnectionTime = millis();
    while (millis() - lastConnectionTime < postingInterval) {
      digitalWrite(relayPin, HIGH); //릴레이 접점 ON
      Serial.println("On");
    }
    digitalWrite(relayPin, LOW); //릴레이 접점 OFF
    Serial.println("Off");
  }
}

/*
 Web Server for Arduino with RSD (feat. Web client Repeating)

 A simple web server using an Arduino Wiznet Ethernet shield.

 This sketch uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 Adapted 191202 by Eunhan KIM
 200313 v3.0 - Analog pin set to Input, LED Blink, Hardware Reset, Float Voltage Output
 200316 v3.1 - Voltage Fitting
 
*/

#include <SPI.h>
#include <Ethernet.h>

static String myID = "PumpRelay_v1";
String fA = "\xF0";
char fC = 0;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
float vB[] = {999, 999, 999, 999, 999};


float vout = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;
float vin = 0.0;
float vA0 = 0.0;
int i = 0;

//Router
int port = 3011;
IPAddress ip(192, 168, 10, 101);
EthernetServer server(port);

//Relay Pin Number
const int relayPin = 6;


void setup() {
  
  Serial.begin(9600);
  pinMode(A0, INPUT); // Read from AC Power Supply
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
  Serial.print(" My IP address: ");
  Serial.print(Ethernet.localIP());
  Serial.println(" : " + String(port));
  // Relay signla pin output
  pinMode(relayPin, OUTPUT); 
  delay(1000);
}

void loop() {
  String buff;
  EthernetClient client = server.available();
  //Serial.println(i);
    if (client) {
      while (client.connected()) {
        if (client.available()) {         
          char c = client.read();
          if (c == 'D') { // Return DC Voltages
            digitalWrite(relayPin, HIGH); //릴레이 접점 ON
            delay(1000); //1초 대기
            digitalWrite(relayPin, LOW); //릴레이 접점 OFF
            delay(1000); //1초 대기
            
            buff = "<PowerStatus>\n <ERM_ID>" + myID + "</ERM_ID>\n" + " <AnalogChannelStatus> "; 
            buff = buff + toStr1(vA0) + ", ";
            buff = buff + "</AnalogChannelStatus>\n</PowerStatus>\n";
            int fB = buff.length(); // 최대 195자 ?
            Serial.println(buff);
            client.print (fA + char(fB) + fC + fC + fC + buff);
            break;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1000);
  }
}


float vDC(float aCh) {
  value = analogRead(aCh);
  vout = (value*5.0) / 1024.0;
  vin = vout / (R2/(R1+R2)); 
  return vin; 
}

String toStr1(float v) {
  char vCh[5];
  dtostrf(v, 5, 2, vCh);
  return vCh;
}

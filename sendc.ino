/*
  LoRa Duplex communication

  Sends a message every 20 seconds, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFF as the broadcast address.
 
  Uses readString() from Stream class to read payload. The Stream class'
  timeout may affect other functuons, like the radio's callback.

*/
// include libraries
#include <SPI.h>             
#include <LoRa.h>
#include "DHT.h"

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//packet counter
int readingID = 0;  
int counter = 0;
String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device. Merci de mettre une adresse différente des autres
byte destination = 0xCC;      // destination to send to. Merci de voir l'adresse de votre collègue. OxFF adresse pour tous le monde
long lastSendTime = 0;        // last send time
int interval = 20000;         // interval between sends

#define DHTPIN 4     
const int pomp = 25;
int up;
int SENSOR_PIN = 0;     /* select analog pin */
int SENSOR_VAL = 0;      /* variable storing sensor value */

#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(9600);                   // initialize serial
  pinMode(25, OUTPUT);
  while (!Serial);

  Serial.println("LoRa Duplex");

   //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    // Increment readingID on every new reading
    readingID++;
    Serial.println("Starting LoRa failed!");
  }
  Serial.println("LoRa Initialization OK!");
  delay(2000);
  dht.begin();
}

void loop() {
  
if (millis() - lastSendTime > interval) {
    //////////////////////////////////////////////
Serial.print("Sending packet: ");
Serial.println(counter);  
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  //Humidity sensor
  SENSOR_VAL = analogRead(SENSOR_PIN);
  int valeur = map(SENSOR_VAL, 0,4096, 0, 100);
  valeur = 100 - valeur;
  
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(h);
  LoRa.print("|");
  LoRa.print(t);
  LoRa.print("|");
  //LoRa.print("valeur");
  //LoRa.print("|");
  LoRa.print(counter);
  LoRa.print("|");
  LoRa.endPacket();

  counter++;
    
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("% ");
  Serial.print("| Temperature:");
  Serial.print(t);
  Serial.print("°C ");
  Serial.print("| Humidity de sol: ");
  Serial.print(valeur);   
  Serial.println("%");
   
    ////////////////////////////////////////////////////////////
    lastSendTime = millis();  
    
}
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
//     int up = LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + String(incoming));
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  if ( incoming == "up"){
    digitalWrite(25, 1);
  }else if ( incoming == "down" ){
    digitalWrite(25, 0);
  }
  Serial.println();
}



////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

#include <Servo.h>
#include <dht11.h>
#include <SPI.h>    
#include <Ethernet.h>
#include <EthernetUdp.h>

#define DHT11PIN 2

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 1, 177);

EthernetUDP Udp;
dht11 Dht11;
Servo servo;
float nem,sicaklik = 0.0f;
bool checkDoor = false;
bool relays[16];
int basePin = 30;
unsigned int localPort = 8888;
char buf[500];
String buff = "";
bool loaded = false;
int index = 0;
bool data = false;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 


int SizeOfPacket(){
  return Udp.parsePacket();
}

String readPacket(int sizepacket)
{
  Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
  String message = packetBuffer;
  
  return message.substring(0,sizepacket);
}
void sendChars(char* reply)
{
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.flush();
    Udp.write(reply);
    Udp.endPacket();
}

void checkRelays(){
  for(int i = 0; i < 16; i++)
    relays[i] = false;  
}
void updateSensors(){
  Dht11.read(DHT11PIN);
  sicaklik = (float)Dht11.temperature;
  nem = (float)Dht11.humidity;
  checkDoor = digitalRead(7);
   
}
String sayStatus(){
  updateSensors();
  String buf = "";
  for(int i = 0; i < 16; i++)
      buf += (String(i) + ":" + relays[i] + ":");
     
  buf += ("s:"+ String(sicaklik) + ":");
  buf += ("n:"+ String(nem) + ":");
  buf += ("d:"+ String(checkDoor) + ":");
  
  return buf;
}
void openTheDoor(){
    servo.write(120);
    delay(15);
}
void closeTheDoor(){
    servo.write(0);
    delay(15);
  
}
void relayOn(){
  for(int i = 0; i < 16; i++)
  {
    if(relays[i])
      digitalWrite(basePin + i, LOW);  
    else
      digitalWrite(basePin + i, HIGH); 
  }
  
  if(relays[14]) 
    openTheDoor();
  else
    closeTheDoor();
  
}

void setup() {
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
  servo.attach(3);
  checkRelays();
  for(int i = 0; i < 16; i++)
    pinMode(i + basePin,OUTPUT);
}

void loop() {
  int sizepacket = SizeOfPacket();
  if (sizepacket != 0) {
    String message = readPacket(sizepacket);
      if(message == "a")
      {
        message = sayStatus();
        char buf[500];
        message.toCharArray(buf,500);
        sendChars(buf);
      }else{
        Serial.println(message);
        buff = "";
        loaded = false;
        index = 0;
        data = false;
        message.toCharArray(buf,500);
        
        for(int i = 0; i < message.length(); i++)
        {
          if(buf[i] == ':')
            {
              if(loaded){
                if(buff == "0")
                  data = false;
                else
                  data = true;
                  
              relays[index] = data;
               loaded = false;
              }
              else{
                index = buff.toInt();
                loaded = true;
              }
               buff = ""; 
            }
          else
            {
              buff += buf[i];
            }
            
        }
        
      }
  }else
    relayOn();
}





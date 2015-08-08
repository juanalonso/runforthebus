#include <SPI.h>
#include <Ethernet.h>

const int VOLTPIN = 3;
const long NOBUS = 999999;

long waitingTime;

EthernetClient client;
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xCC, 0x95};

void setup() {

  pinMode(VOLTPIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial){}; 

  if (! Ethernet.begin(mac)) {
    Serial.println("Could not connect to network");
  } 

  delay(1000);
  
}



void loop() {

    hitWebPage();
    if (waitingTime != NOBUS) {
      analogWrite (VOLTPIN, waitingTime);
    } else {
      analogWrite (VOLTPIN, 0);
    }
    
    for(int f=0; f<20; f++) {
      Serial.print("*");
      delay(500);
    }
    Serial.println();

}



long hitWebPage() {
  
  if (client.connect("neuronasdq.cluster005.ovh.net", 80)) {
    
    Serial.println("Getting data...");
    
    client.println("GET /bus/getbusinfo.php HTTP/1.0");
    client.println("Host: neuronasdq.cluster005.ovh.net");
    client.println();
    
    while (client.connected()) {
      if (client.available()) {
        client.find("\"wait\":");
        waitingTime = client.parseInt();
        Serial.println(waitingTime);
        client.stop();
        return waitingTime;
      }
    }
    
    client.stop();

  }
  
  return NOBUS;

}

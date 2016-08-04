#include <SPI.h>
#include <Ethernet.h>

const int VOLTPIN = 6;
const int BUTTONPIN = 3;
const int LEDPIN = 8;

const int DATASAMPLES = 12;
const long DELAYBETWEENSAMPLES = 10000;
const int BLINK = 500;
const int NOBUSVOLTS = 150;

const long NOBUS = 999999;

long waitingTime;

EthernetClient client;
byte mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0xCC, 0x95};

void setup() {

  pinMode(BUTTONPIN, INPUT_PULLUP);

  pinMode(VOLTPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  
  Serial.begin(9600);
  while (!Serial) {};

  if (! Ethernet.begin(mac)) {
    Serial.println("Could not connect to network");
  } else {
    Serial.print("Local IP: ");
    Serial.println(Ethernet.localIP());
  }
  delay(1000);

}



void loop() {

  if (digitalRead(BUTTONPIN) == LOW) {

    digitalWrite(LEDPIN, HIGH);

    Serial.println("Getting data");
    for (int currentRep = 0; currentRep < DATASAMPLES; currentRep++) {

      hitWebPage();

      if (waitingTime != NOBUS) {
        analogWrite (VOLTPIN, waitingTime);
      } else {
        analogWrite (VOLTPIN, NOBUSVOLTS);
      }

      if (waitingTime < 35) {

        for (int f = 0; f < DELAYBETWEENSAMPLES / BLINK; f++) {
          delay(BLINK/2);
          digitalWrite(LEDPIN, LOW);
          delay(BLINK/2);
          digitalWrite(LEDPIN, HIGH);
        }
        
      } else if (waitingTime < 70) {

        for (int f = 0; f < DELAYBETWEENSAMPLES / (BLINK * 2); f++) {
          delay(BLINK);
          digitalWrite(LEDPIN, LOW);
          delay(BLINK);
          digitalWrite(LEDPIN, HIGH);
        }
        
      } else {
        
        delay (DELAYBETWEENSAMPLES);
        
      }

    }

    analogWrite (VOLTPIN, 0);
    digitalWrite(LEDPIN, LOW);

    Serial.println("End");
    Serial.println();

  } else {

    delay (100);

  }


}




long hitWebPage() {

  if (client.connect("neuronasdq.cluster005.ovh.net", 80)) {

    Serial.println("    Connecting");

    client.println("GET /bus/getbusinfo.php HTTP/1.0");
    client.println("Host: neuronasdq.cluster005.ovh.net");
    client.println();

    while (client.connected()) {
      if (client.available()) {
        client.find("\"wait\":");
        waitingTime = client.parseInt();
        Serial.print("    ");
        Serial.println(waitingTime);
        client.stop();
        return waitingTime;
      }
    }

    client.stop();

  }

  return NOBUS;

}

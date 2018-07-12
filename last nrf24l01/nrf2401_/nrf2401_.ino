#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>

#define CE_PIN   7
#define CSN_PIN 8


const uint64_t pipe = 0xE8E8F0F0E1LL; // Define the transmit pipe

SoftwareSerial mySerial(5, 6);
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

int reads = 0;
int datain[1];
byte dataout[1];
bool allow = false;
int a ;
void setup()
{
  Serial.begin(9600);
  mySerial.begin(115200);

  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  Serial.println("Nrf24L01 Receiver Starting");
  delay(1000);
  Serial.println("setup has done");
}

void loop() {
  if ( radio.available() )
  {
    radio.read( datain, sizeof(datain) );
    a = datain[0];
    Serial.print(datain[0]);
    if (a == 88) {
      Serial.println("BS order Calibration");
      allow = true;
      a = 1;
    }
  }

  if (mySerial.available())
  {
    reads = mySerial.read();
    if (reads == 22) {
      Serial.println("cc1101 send a req for calibration ");
      if (allow) {
        mySerial.write(33);
        allow = false ;
        Serial.println("calibration order sent");
      }
      else {
        Serial.println("calibration order doesn't sent");
      }
      reads = 0;
    }
    else if ( reads == 12 ||reads == 14|| reads==13 ||reads == 24|| reads == 21 || reads==23 ||  reads == 31 || reads==32 ||reads == 34 ||reads == 41 ||reads == 42 ||reads == 43  ) {
      Serial.println("cc1101 detected something !!");
      Serial.println("Path is =");
      Serial.println(reads);
      Serial.println(); 
      dataout[0] = reads;
      // making radio ready
      radio.stopListening();
      delay(30);
      radio.openWritingPipe(pipe);
      radio.write( dataout, sizeof(dataout) );
      delay(20);
      Serial.println("node ID sent to the BS");
      radio.openReadingPipe(1, pipe);
      radio.startListening();
      reads = 0;
    }

  }
}

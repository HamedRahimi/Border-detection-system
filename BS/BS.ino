#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   7
#define CSN_PIN 8



const uint64_t pipe = 0xE8E8F0F0E1LL ; // Define the transmit pipe
int calibration;
int datain[1];
byte dataout[1];

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

void setup()
{
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  delay(1000);
}


void loop()
{
  if ( radio.available() )
  {
    radio.read( datain, sizeof(datain) );
    if (0<datain[0]<100)
    {
      Serial.println(datain[0]);
    }
  }
  if (Serial.available())
  {
    if (Serial.find('8')) {
      // making radio ready
      dataout[0] = 88;
      radio.stopListening();
      delay(50);
      radio.openWritingPipe(pipe);
      radio.write( dataout, sizeof(dataout) );
      delay(50);
      radio.openReadingPipe(1, pipe);
      radio.startListening();
      delay(50);

    }
  }
}


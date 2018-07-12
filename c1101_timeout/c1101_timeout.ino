#include "cc1101.h"
#include <ELECHOUSE_CC1101.h>
#include "EEPROM.h"
#include <SoftwareSerial.h>

#define NodeID 1
#define numberOfNodes 3

#define tx_size 1  //size of tx buffer
#define rx_size 1  //size of rx buffer


SoftwareSerial mySerial(5, 6);
CC1101 cc1101;

byte TX_buffer[tx_size] = {0};
byte RX_buffer[rx_size] = {0};
byte size, i;

volatile bool packetAvailable;
bool initializer;

int Calibration[numberOfNodes] = {0};
byte Rssi[numberOfNodes] = {0};
byte t = 0;

byte rssi ;
byte lqi ;
int j = 0; //number of loop
bool cal ;
int calCount = 20;
unsigned long t1, t2 = 0;
int k, l = 1;

void setup()
{
  cal = true;
  packetAvailable = false;
  initializer = true;

  Serial.begin(9600);
  mySerial.begin(115200);
  ELECHOUSE_cc1101.Init();

  if (NodeID != 1)
  {
    ELECHOUSE_cc1101.SetReceive();
  }
  attachInterrupt(0, isPacketAvailable, FALLING);
  Serial.println("setup has done");
  delay(100);
}

void loop()
{
  // for the first time start
  if (NodeID == 1 && initializer == true) {
    sendCC();
    initializer = false;
    Serial.println("node 1 has initialized the network");
  }

  t1 = millis();
  signed long diff = t1 - t2;
  if (NodeID == t && diff >= 2000 * k) {
    Serial.print("t1 is  ");
    Serial.println(t1);
    Serial.print("t2 is  ");
    Serial.println(t2);
    Serial.print("diff is  ");
    Serial.println(diff);
    Serial.print("retrying  ");
    Serial.println(k);
    delay(1);
    sendCC();
    k++;
  }
  if (NodeID == 1 && diff >= 5000 * l) {
    Serial.print("t1 is  ");
    Serial.println(t1);
    Serial.print("t2 is  ");
    Serial.println(t2);
    Serial.print("diff is  ");
    Serial.println(diff);
    Serial.print("retrying  ");
    Serial.println(k);
    delay(1);
    sendCC();
    l++;
  }

  if (packetAvailable)
  {
    Serial.println();
    Serial.println("this node is listening :");
    size = ELECHOUSE_cc1101.ReceiveData(RX_buffer);
    byte TID = RX_buffer[0];
    cc1101.flushRxFifo();
    byte t ;
    if (TID != 0 && TID <= numberOfNodes)
    {
      k = 1;
      l = 1;
      t2 = millis();
      Serial.print("RSSI : ");
      rssi = ReadRSSI();
      Serial.println(rssi);
      Serial.print("LQI : ");
      lqi = ReadLQI();
      Serial.println(lqi);
      Serial.print("number node : ");
      Serial.println(TID);
      t = TID + 1 ;
      if (t > numberOfNodes) {
        t = 1;
      }
      Serial.print("next node is : ");
      Serial.println(t);

      if (cal && j < calCount)
      {
        if (j > 10) {
          Calibration[TID - 1] = Calibration[TID - 1] + rssi ;
        }
        if (t == 1) {
          Serial.println();
          Serial.println(j);
          Serial.println();
          j++;
        }
        if (NodeID == numberOfNodes && t == numberOfNodes) {
          Serial.println();
          Serial.println(j);
          Serial.println();
          j++;
        }
        if (j == calCount) {
          Serial.println();
          Serial.println("calibration completed");
          Serial.println();
          j = j - 10;
          for (int k = 0; k < numberOfNodes; k++) {
            Calibration[k] = Calibration[k] / j;
            Serial.println(Calibration[k]);
          }
          cal = false;
          j = 0;
        }
      }
      else if (!cal) {
        Rssi[TID - 1] = rssi;
        byte detect = abs(Rssi[TID - 1] - Calibration[TID - 1]);
        if (detect >= 10 ) {
          Serial.println("cc1101 detect something ");
          path_detect (NodeID, TID);
          Serial.println("detection positive and sent from cc1101 to nrf");
        }
        if (TID == numberOfNodes) {
          Serial.println("Asking for Calibration");
          mySerial.write(22);
          delay(10);
          if (mySerial.available()) {
            if (mySerial.read() == 33)
            {
              cal = true ;
              Serial.println(" calibration order has come from nrf to cc1101 ");

            }
          }
          else   {
            Serial.println("calibration order has not come from nrf to cc1101 ");
          }
        }
      }
    }
    if (NodeID == t ) {
      if (NodeID == numberOfNodes && !cal) {
        Serial.println("Asking for Calibration");
        mySerial.write(22);
        delay(10);
        if (mySerial.available()) {
          if (mySerial.read() == 33)
          {
            cal = true ;
            Serial.println("calibration order has come from nrf to cc1101 ");
          }
          else   {
            Serial.println(" calibration order has not come from nrf to cc1101 ");
          }
        }
      }

      sendCC();
    }
    else if (NodeID == 1 && TID == numberOfNodes )
    {
      sendCC();
    }

    else  {
      Serial.println("its not my turn");
    }
    ELECHOUSE_cc1101.SetReceive();
    packetAvailable = false ;
    interrupts();
  }
}

byte ReadLQI()
{ //returns the amount of lqi
  byte lqi = 0;
  byte value = 0;
  lqi = (cc1101.readReg(CC1101_LQI, CC1101_STATUS_REGISTER));
  value = 0x3F - (lqi & 0x3F);
  //Serial.print("CC1101_LQI ");
  //Serial.println(value);
  return value;
}

byte ReadRSSI()
{ //returns the amount of rssi
  byte rssi = 0;
  byte value = 0;

  rssi = (cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER));

  if (rssi >= 128)
  {
    value = 255 - rssi;
    value /= 2;
    value += 74;
  }
  else
  {
    value = rssi / 2;
    value += 74;
  }
  //Serial.print("CC1101_RSSI ");
  //Serial.println(value);
  return value;
}
void isPacketAvailable() {
  noInterrupts();
  packetAvailable = true;
}
//Sending Function
void sendCC() {
  Serial.println("this node is transmiting data");
  TX_buffer[0] = NodeID; //send the ID of node in order to calculate RSSI in receiver
  delay(300);
  ELECHOUSE_cc1101.SendData(TX_buffer, tx_size);
  Serial.print("node ");
  Serial.print(NodeID);
  Serial.println(" finished transmit");
}
int path_detect (int a, int b)
{
  String  A = String(a);
  String  B = String(b);
  String  out = String();
  out = A + B;
  int inter = out.toInt();
  mySerial.write(inter);
  return 0;
}


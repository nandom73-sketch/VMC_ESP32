#include <SPI.h>

#define TFT_CS   5
#define TFT_DC   4

void setup() {
  Serial.begin(115200);

  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);

  digitalWrite(TFT_CS, HIGH);
  SPI.begin();

  delay(1000);

  readID();
}

void loop() {}

void readID() {
  uint8_t id1, id2, id3;

  digitalWrite(TFT_CS, LOW);
  digitalWrite(TFT_DC, LOW); // comando

  SPI.transfer(0x04); // comando read ID

  digitalWrite(TFT_DC, HIGH); // dati

  id1 = SPI.transfer(0x00);
  id2 = SPI.transfer(0x00);
  id3 = SPI.transfer(0x00);

  digitalWrite(TFT_CS, HIGH);

  Serial.print("ID: ");
  Serial.print(id1, HEX);
  Serial.print(" ");
  Serial.print(id2, HEX);
  Serial.print(" ");
  Serial.println(id3, HEX);
}
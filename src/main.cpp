
#include <Wire.h>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <SPIFFS.h>

#define PN532_SCK 16
#define PN532_MOSI 17
#define PN532_SS 18
#define PN532_MISO 19
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup(void)
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);
  Serial.println("Hello!");

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open("/db.txt");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.println("File Content:");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
  //// ---------------------
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.print("Didn't find PN53x board");
    while (1)
      ;
  }

  Serial.print("\nFound chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("\nFirmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  nfc.setPassiveActivationRetries(0xFF);

  Serial.println("Waiting for an ISO14443A card");
}

void loop(void)
{

  Serial.println("Waiting for an ISO14443A card");
  boolean success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success)
  {
    Serial.println("Found an NFC card!");

    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX);
    }
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++)
    {
      if (uid[i] < 0x10)
        uidString += "0";
      uidString += String(uid[i], HEX);
    }

    File file = SPIFFS.open("/db.txt", "w");
    if (!file)
    {
      Serial.println("Error opening file for writing");
      return;
    }

    int bytesWritten = file.print(uidString);

    if (bytesWritten > 0)
    {
      Serial.println("\nFile was written");
      Serial.println(bytesWritten);
    }
    else
    {
      Serial.println("File write failed");
    }
    Serial.print(file.readString());

    file.close();
    Serial.println("");
  }
}

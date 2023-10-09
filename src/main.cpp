#include <Wire.h>
#include <Adafruit_PN532.h>
#include <vector>

#define PN532_SCK 16
#define PN532_MOSI 17
#define PN532_SS 18
#define PN532_MISO 19

void readMode();
void writeMode();
bool compareUID(uint8_t *uid1, uint8_t uid1Length, uint8_t *uid2, uint8_t uid2Length);

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
bool isReadModeActive;
struct Tarjeta
{
  uint8_t uid[7];
  uint8_t uidLength;
  uint32_t codigo;
  bool cargada;
};

std::vector<Tarjeta> tarjetasGimnasio;

void setup()
{
  Serial.begin(115200);
  Serial.println("Inicializando NFC...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.print("Error al detectar el PN532. Asegúrate de que esté conectado correctamente.");
    while (1)
      ;
  }

  nfc.SAMConfig();
  Serial.println("Listo para leer tarjetas NFC.");
}

void loop()
{
  char respuesta;
  Serial.print("Quieres usar el modo lectura de la tarjeta? (S/N): ");
  while (!Serial.available())
  {
    // Espera a que el usuario ingrese una respuesta
  }
  respuesta = Serial.read();
  isReadModeActive = respuesta == 'S' || respuesta == 's';
  if (isReadModeActive)
  {
    readMode();
  }
  else
  {
    writeMode();
  }
}

bool compareUID(uint8_t *uid1, uint8_t uid1Length, uint8_t *uid2, uint8_t uid2Length)
{
  if (uid1Length != uid2Length)
  {
    Serial.print("\nInvalid UID length");
    return false;
  }

  for (uint8_t i = 0; i < uid1Length; i++)
  {
    if (uid1[i] != uid2[i])
    {
      Serial.print("\nInvalid UID");
      return false;
    }
  }

  return true;
}

void readMode()
{
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success)
  {
    Serial.println("\nTarjeta detectada!");

    bool tarjetaEncontrada = false;
    Serial.print("\nTarketa size: ");
    Serial.print(tarjetasGimnasio.size());
    for (int i = 0; i < tarjetasGimnasio.size(); i++)
    {
      bool comparable = compareUID(uid, uidLength, tarjetasGimnasio[i].uid, sizeof(tarjetasGimnasio[i].uid));
      if (comparable)
      {
        Serial.print("\nTarjeta cargada - Código: ");
        Serial.println(tarjetasGimnasio[i].codigo);
        tarjetaEncontrada = true;

        break;
      }
    }

    if (!tarjetaEncontrada)
    {
      Serial.println("\nEsta tarjeta no está cargada en la lista.");

      char respuesta;
      Serial.print("\n¿Deseas agregar esta tarjeta? (S/N): ");
      while (!Serial.available())
      {
        // Espera a que el usuario ingrese una respuesta
      }
      respuesta = Serial.read();

      if (respuesta == 'S' || respuesta == 's')
      {
        uint32_t nuevoCodigo;
        Serial.print("\nIngresa el código para esta tarjeta: ");
        while (!Serial.available())
        {
          // Espera a que el usuario ingrese el código
        }
        nuevoCodigo = Serial.parseInt();

        // Agrega la nueva tarjeta a la lista
        tarjetasGimnasio.push_back({{uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]}, uidLength, nuevoCodigo, true});

        Serial.print("\nTarjeta agregada - Código: ");
        Serial.println(nuevoCodigo);
      }
    }
  }
}

void writeMode()
{
  Serial.println("Modo de escritura activado.");
  Serial.println("Tarjetas guardadas:");

  for (int i = 0; i < tarjetasGimnasio.size(); i++)
  {
    Serial.print("Tarjeta ");
    Serial.print(i + 1);
    Serial.print(": Código - ");
    Serial.println(tarjetasGimnasio[i].codigo);
  }

  Serial.print("Selecciona el número de tarjeta para escribir (1 - ");
  Serial.print(tarjetasGimnasio.size());
  Serial.print(") o 0 para salir: ");

  while (!Serial.available())
  {
    // Espera a que el usuario ingrese una respuesta
  }

  int seleccion = Serial.parseInt();

  if (seleccion >= 1 && seleccion <= tarjetasGimnasio.size())
  {
    // El usuario seleccionó una tarjeta válida
    Serial.print("Escribir en la tarjeta ");
    Serial.print(seleccion);
    Serial.print(": Código - ");
    Serial.println(tarjetasGimnasio[seleccion - 1].codigo);
    uint8_t bloqueAEscribir = 4; // Número de bloque en el que deseas escribir
    nfc.mifareclassic_WriteDataBlock(bloqueAEscribir, tarjetasGimnasio[seleccion - 1].uid);
    Serial.println("Escritura completada.");
  }
  else
  {
    Serial.println("Selección no válida. Por favor, selecciona un número de tarjeta válido o 0 para salir.");
  }
}

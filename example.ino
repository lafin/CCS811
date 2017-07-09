#include "CCS811.h"

// D0 -> WAK
// D1 -> SCL
// D2 -> SDA
// 3V -> VCC
// G -> GND
// D5 -> INT
// G -> ADD

#define ADDRESS 0x5A
#define WAKE_PIN D0
#define INT_PIN D5

CCS811 sensor;
uint8_t rawData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint16_t eCO2 = 0, TVOC = 0;
uint8_t Current = 0;
float Voltage = 0.0f;
bool newData  = true;

void intHandler()
{
  newData = true;
}

void setup()
{
  Serial.begin(115200);
  digitalWrite(WAKE_PIN, LOW);
  if (!sensor.begin(uint8_t(ADDRESS), uint8_t(WAKE_PIN), uint8_t(INT_PIN))) {
    Serial.println("Initialization failed.");
    delay(1000);
    abort();
  }
  digitalWrite(WAKE_PIN, HIGH);
  attachInterrupt(INT_PIN,  intHandler, FALLING);
}

void loop()
{
  if (newData == true) {
    newData = false;

    digitalWrite(WAKE_PIN, LOW);
    sensor.readData(rawData);
    digitalWrite(WAKE_PIN, HIGH);

    eCO2 = (uint16_t) ((uint16_t) rawData[0] << 8 | rawData[1]);
    TVOC = (uint16_t) ((uint16_t) rawData[2] << 8 | rawData[3]);
    Current = (rawData[6] & 0xFC) >> 2;
    Voltage = (float) ((uint16_t) ((((uint16_t)rawData[6] & 0x02) << 8) | rawData[7])) * (1.65f / 1023.0f), 3;
  }

  Serial.print("Eq CO2 in ppm = "); Serial.print(eCO2); Serial.print(" ");
  Serial.print("TVOC in ppb = "); Serial.print(TVOC); Serial.print(" ");
  Serial.print("Sensor current (uA) = "); Serial.print(Current); Serial.print(" ");
  Serial.print("Sensor voltage (V) = "); Serial.println(Voltage, 2);

  delay(1000);
}

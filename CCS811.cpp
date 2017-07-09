#include "CCS811.h"

CCS811::CCS811()
{
}

void CCS811::printVersion(uint8_t * FWVersion)
{
  Serial.print((FWVersion[0] & 0xF0) >> 4); Serial.print(".");
  Serial.print(FWVersion[0] & 0x04); Serial.print(".");
  Serial.println(FWVersion[1]);
}

boolean CCS811::begin(uint8_t address, uint8_t wakePin, uint8_t intPin)
{
  delay(70); // from datasheet - up to 70ms on the first Reset after new application download; up to 20ms delay after power on
  Wire.begin();

  pinMode(intPin, INPUT_PULLUP);
  pinMode(wakePin, OUTPUT);

  // initialize CCS811 and check version and status
  byte HWVersion = readByte(ADDRESS, HW_VERSION);
  Serial.print("CCS811 Hardware Version = 0x"); Serial.println(HWVersion, HEX);

  uint8_t FWBootVersion[2] = {0, 0}, FWAppVersion[2] = {0, 0};
  readBytes(ADDRESS, FW_BOOT_VERSION, 2, &FWBootVersion[0]);
  Serial.print("CCS811 Firmware Boot Version: ");
  printVersion(FWBootVersion);

  readBytes(ADDRESS, FW_APP_VERSION, 2, &FWAppVersion[0]);
  Serial.print("CCS811 Firmware App Version: ");
  printVersion(FWAppVersion);

  reset();
  byte hw_id = readByte(ADDRESS, HW_ID);
  if (hw_id != 0x81) // this is the expected hardware ID
  {
    Serial.println("Error: Incorrect Hardware ID detected.");
    return false;
  }

  checkStatus();

  Wire.beginTransmission(ADDRESS); // least significant bit indicates write (0) or read (1)
  Wire.write(APP_START);
  Wire.endTransmission();

  checkStatus();

  writeByte(ADDRESS, MEAS_MODE, dt_1sec << 4 | 0x08); // pulsed heating mode, enable interrupt
  uint8_t measmode = readByte(ADDRESS, MEAS_MODE);
  Serial.print("Confirm measurement mode = 0x"); Serial.println(measmode, HEX);

  return true;
}

void CCS811::checkStatus()
{
  // Check CCS811 status
  uint8_t status = readByte(ADDRESS, STATUS);
  Serial.print("status = 0X"); Serial.println(status, HEX);
  if (status & 0x80) {
    Serial.println("Firmware is in application mode. CCS811 is ready!");
  }
  else {
    Serial.println("Firmware is in boot mode!");
  }

  if (status & 0x10) {
    Serial.println("Valid application firmware loaded!");
  }
  else {
    Serial.println("No application firmware is loaded!");
  }

  if (status & 0x08) {
    Serial.println("New data available!");
  }
  else {
    Serial.println("No new data available!");
  }

  if (status & 0x01) {
    Serial.println("Error detected!");
    uint8_t error = readByte(ADDRESS, ERROR_ID);
    if (error & 0x01) Serial.println("CCS811 received invalid I2C write request!");
    if (error & 0x02) Serial.println("CCS811 received invalid I2C read request!");
    if (error & 0x04) Serial.println("CCS811 received unsupported mode request!");
    if (error & 0x08) Serial.println("Sensor resistance measurement at maximum range!");
    if (error & 0x10) Serial.println("Heater current is not in range!");
    if (error & 0x20) Serial.println("Heater voltage is not being applied correctly!");
  }
  else {
    Serial.println("No error detected!");
  }

  Serial.println(" ");

}

void CCS811::readData(uint8_t * destination)
{
  uint8_t rawData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t status = readByte(ADDRESS, STATUS);

  if (status & 0x01)
  {
    uint8_t error = readByte(ADDRESS, ERROR_ID);
    if (error & 0x01) Serial.println("CCS811 received invalid I2C write request!");
    if (error & 0x02) Serial.println("CCS811 received invalid I2C read request!");
    if (error & 0x04) Serial.println("CCS811 received unsupported mode request!");
    if (error & 0x08) Serial.println("Sensor resistance measurement at maximum range!");
    if (error & 0x10) Serial.println("Heater current is not in range!");
    if (error & 0x20) Serial.println("Heater voltage is not being applied correctly!");
  }

  readBytes(ADDRESS, ALG_RESULT_DATA, 8, &rawData[0]);

  for (int i = 0; i < 8; i++)
  {
    destination[i] = rawData[i];
  }
}

void CCS811::reset() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(SW_RESET);
  Wire.write(0x11);
  Wire.write(0xE5);
  Wire.write(0x72);
  Wire.write(0x8A);
  Wire.endTransmission();
}

void CCS811::writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t CCS811::readByte(uint8_t address, uint8_t subAddress)
{
  uint8_t temp[1];
  readBytes(address, subAddress, 1, &temp[0]);
  return temp[0];
}

void CCS811::readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest)
{
  delay(100);

  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.endTransmission();

  delay(100);

  Wire.requestFrom(address, (uint8_t)count);
  if (Wire.available() == count) {
    for (int i = 0; i < count; i++)
    {
      dest[i] = Wire.read();
    }
  } else {
    Serial.print("Error reading bytes ");
    Serial.print(Wire.available());
    Serial.print(" ");
    Serial.print(count);
    Serial.println();
  }
  delay(100);
}

void CCS811::sleep()
{
  Wire.beginTransmission(ADDRESS);
  Wire.write(MEAS_MODE);
  Wire.write(0x00000000);  // sets sensor to idle; measurements are disabled; lowest power mode
  Wire.endTransmission();
}

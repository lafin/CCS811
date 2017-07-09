#ifndef CCS811_h
#define CCS811_h

#include "Arduino.h"
#include "Wire.h"

#define STATUS             0x00
#define MEAS_MODE          0x01
#define ALG_RESULT_DATA    0x02
#define RAW_DATA           0x03
#define ENV_DATA           0x05
#define NTC                0x06
#define THRESHOLDS         0x10
#define BASELINE           0x11
#define HW_ID              0x20  // WHO_AM_I should be 0x81
#define ID                 0x20  // WHO_AM_I should be 0x1X
#define HW_VERSION         0x21
#define FW_BOOT_VERSION    0x23
#define FW_APP_VERSION     0x24
#define ERROR_ID           0xE0
#define APP_START          0xF4
#define SW_RESET           0xFF

#define ADDRESS            0x5A   // Address of the CCS811 Air Quality Sensor

#define  dt_idle  0x00
#define  dt_1sec  0x01
#define  dt_10sec 0x02
#define  dt_60sec 0x03

class CCS811
{
  public:
    CCS811();
    boolean begin(uint8_t address, uint8_t wakePin, uint8_t intPin);
    byte readStatus(void);
    void checkStatus(void);
    uint8_t readByte(uint8_t address, uint8_t subAddress);
    void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest);
    void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
    void readData(uint8_t * destination);
    void printVersion(uint8_t * FWVersion);
    void reset(void);
    void sleep(void);
};

#endif

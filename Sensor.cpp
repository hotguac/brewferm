// Controls the sensors
#include "Sensor.h"
#include <string>

DS18 sensor(D2);

SENSORS::SENSORS(void) {
  beerF = UNKNOWN;
  chamberF = UNKNOWN;
  ambientF = UNKNOWN;

  refresh();
}

void SENSORS::refresh(void) {
  // Read the next available 1-Wire temperature sensor
  uint8_t addr[8];
  char buffer[2048];

  sensor.read();
  while (!sensor.searchDone()) {
    //printDebugInfo();

    if (!sensor.crcError()) {
      sensor.addr(addr);
      sprintf(buffer,
        "%02X%02X%02X%02X%02X%02X%02X%02X",
        addr[0], addr[1], addr[2], addr[3],
        addr[4], addr[5], addr[6], addr[7]);

      if (strcmp(types[BEER],buffer) == 0) {
        beerF = sensor.fahrenheit() + beerF_calibrate;
      } else if (strcmp(types[CHAMBER],buffer) == 0) {
        chamberF = sensor.fahrenheit() + chamberF_calibrate;
      } else if (strcmp(types[AMBIENT],buffer) == 0) {
        ambientF = sensor.fahrenheit();
      }
    } // !crcError

    delay(250);
    sensor.read();
  }

  // Once all sensors have been read you'll get searchDone() == true
  // Next time read() is called the first sensor is read again
  if (!sensor.searchDone()) {
    //Serial.println("Hmmmmm...");
  } // !sensor.searchDone()

}

void SENSORS::printDebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (sensor.crcError()) {
    //Serial.print("CRC Error ");
  }

  // Print the sensor type
  const char *type;
  switch(sensor.type()) {
    case WIRE_DS1820: type = "DS1820"; break;
    case WIRE_DS18B20: type = "DS18B20"; break;
    case WIRE_DS1822: type = "DS1822"; break;
    case WIRE_DS2438: type = "DS2438"; break;
    default: type = "UNKNOWN"; break;
  }
  //Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  sensor.addr(addr);
  //Serial.printf(
  //  " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
  //  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  //);

  // Print the raw sensor data
  uint8_t data[9];
  sensor.data(data);
  //Serial.printf(
  //  " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
  //  data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  //);
}

double SENSORS::GetTempF(mode_t mode) {
  if (mode == BEER) {
    return beerF;
  } else if (mode == CHAMBER) {
    return chamberF;
  } else if (mode == AMBIENT) {
    return ambientF;
  }

  return UNKNOWN;
}

double SENSORS::GetTempC(mode_t mode) {
  if (mode == BEER) {
    return beerC;
  } else if (mode == CHAMBER) {
    return chamberC;
  } else if (mode == AMBIENT) {
    return ambientC;
  }

  return UNKNOWN;
}

// Controls the sensors
#include "Sensor.h"
#include <string>

DS18 sensor(D2);

SENSORS::SENSORS(void) {
  uint8_t addr[8];
  sensor.addr(addr);
  char buffer[2048];

  beerF = UNKNOWN;
  chamberF = UNKNOWN;
  ambientF = UNKNOWN;

  while (sensor.read()) {
    sprintf(buffer,
      " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);

    if (strcmp(types[BEER],buffer) == 0) {
      beerF = sensor.fahrenheit();
    } else if (strcmp(types[CHAMBER],buffer) == 0) {
      chamberF = sensor.fahrenheit();
    } else if (strcmp(types[AMBIENT],buffer) == 0) {
      ambientF = sensor.fahrenheit();
    }
  }

  if (sensor.searchDone()) {
    // Do something cool with the temperature
  }
}

void SENSORS::refresh(void) {
  // Read the next available 1-Wire temperature sensor
  if (sensor.read()) {
    while (!sensor.searchDone()) {
      // Do something cool with the temperature
    }

    // Additional info useful while debugging
    printDebugInfo();

  // If sensor.read() didn't return true you can try again later
  // This next block helps debug what's wrong.
  // It's not needed for the sensor to work properly
  } else {
    // Once all sensors have been read you'll get searchDone() == true
    // Next time read() is called the first sensor is read again
    if (sensor.searchDone()) {
      //Serial.println("No more addresses.");
    } else { // Something went wrong
      printDebugInfo();
    } // sensor.searchDone()
  } // sensor.read()

}

void SENSORS::printDebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (sensor.crcError()) {
    Serial.print("CRC Error ");
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
  Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  sensor.addr(addr);
  Serial.printf(
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  );

  // Print the raw sensor data
  uint8_t data[9];
  sensor.data(data);
  Serial.printf(
    " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  );
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

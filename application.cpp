/*
 ******************************************************************************
  Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "spark_wiring_wifi.h"
#include "stdarg.h"

PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(3);
SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

const char* SSID = "MK0924";
const char* PASSWORD = "W1Th2P34pl5";

#if Wiring_WiFi
STARTUP(System.enable(SYSTEM_FLAG_WIFITESTER_OVER_SERIAL1));
#endif

int ledPin = D7;
int def_delay;

IPAddress remoteAddress(192,168,0,11);  // laptop
int remotePort = 7777;

int localPort = 8888;
UDP Udp;
char buffer[2048];

/* Sensor pin outs

viewed from front of sensor RJ14 plug with tab UP

pin 1 - no connection
pin 2 - Red - 5V
pin 3 - Black - Ground
pin 4 - White - data

viewed from front of wall RJ25 wall jack with tab DOWN

pin 1 -
pin 2 - black
pin 3 - red
pin 4 - green
pin 5 - yellow
pin 6 -

connection mapping from sensor plug to jack

Red 5V -> Red
Black Ground -> green
White Data -> yellow

*/

#include "DS18.h"

DS18 sensor(D2);

void printDebugInfo(void);

#include "pid.h"

double Setpoint, Input, Output;

PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, PID::DIRECT);

/* This function is called once at start up ----------------------------------*/
void setup()
{
    //Setup the Tinker application here

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);

    System.disableUpdates();
    WiFi.setCredentials(SSID, PASSWORD, WPA2);
    WiFi.on();

    delay(10 * 1000);
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
    delay(20 * 1000);

    Serial.begin(9600);
    while(!Serial.available()) Particle.process();

    if (WiFi.ready()) {
        def_delay = 20 * 1000;

        Serial.print("WiFi is ready using ");
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.localIP());
        Udp.begin(8888);
        delay(def_delay);
        Particle.syncTime();
        delay(def_delay);
    } else {
        def_delay = 5 * 1000;
        Serial.println("WiFi not connected");
    }

    // Init pid fields
    Input = 74.7;
    Setpoint = 64.0;

    // Turn on pid
    myPID.SetMode(PID::AUTOMATIC);
    myPID.SetOutputLimits(34.0,84.0);
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    int len;

    //This will run in a loop
    digitalWrite(ledPin, HIGH);
    delay(15);
    digitalWrite(ledPin, LOW);

    time_t time = Time.now();
    std::string now(Time.format(time, TIME_FORMAT_ISO8601_FULL));
    now.append("|");

    //const uint8_t message[] = "Hello";
    if (WiFi.ready()) {
        // Read the next available 1-Wire temperature sensor
        if (sensor.read()) {
          // Do something cool with the temperature
          Serial.printf("Temperature %.2f C %.2f F ", sensor.celsius(), sensor.fahrenheit());
          int n = sprintf(buffer, "%4.1f", sensor.fahrenheit());
          now.append(buffer);
          now.append("F");

          Input = sensor.fahrenheit();
          myPID.Compute();
          n = sprintf(buffer, "%4.1f", Output);
          now.append("|PID Output=");
          now.append(buffer);
          now.append("F");

          const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());
          int len = Udp.sendPacket(message, now.length(), remoteAddress, remotePort);
          if (len == now.length()) {
            Serial.printf("message = %s, sizeof = %d\n\r", message, now.length());
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
      } // WiFi.ready()

    Serial.printf("\n\r");
    delay(def_delay);
}

void printDebugInfo() {
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

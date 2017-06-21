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

#include "Sensor.h"
SENSORS mySensors;

#include "pid.h"
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, PID::DIRECT);

#include "relays.h"
RELAYS myRelays;

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
        def_delay = 2 * 1000;

        Serial.print("WiFi is ready using ");
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.localIP());
        Udp.begin(8888);
        delay(def_delay);
        Particle.connect();
        delay(def_delay);
        Particle.syncTime();
        delay(def_delay);
        Particle.disconnect();
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
    //This will run in a loop
    digitalWrite(ledPin, HIGH);

    time_t time = Time.now();
    std::string now(Time.format(time, TIME_FORMAT_ISO8601_FULL));
    now.append("|");

    //const uint8_t message[] = "Hello";
    if (WiFi.ready()) {
        // Read the next available 1-Wire temperature sensor
        mySensors.refresh();

        if (mySensors.GetTempF(SENSORS::BEER) > 65) {
          myRelays.coolON();
        } else {
          myRelays.coolOFF();
        }

        // Do something cool with the temperature
        /*
        Serial.printf("Temperature %.2f C %.2f F ", sensor.celsius(), sensor.fahrenheit());
        sprintf(buffer, "%4.1f", sensor.fahrenheit());
        now.append(buffer);
        now.append("F");
        */
        Input = mySensors.GetTempF(SENSORS::BEER);
        myPID.Compute();
        sprintf(buffer, "%4.1f", Output);
        now.append("|PID Output=");
        now.append(buffer);
        now.append("F");

        const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());
        unsigned int len = Udp.sendPacket(message, now.length(), remoteAddress, remotePort);
        if (len == now.length()) {
          Serial.printf("message = %s, sizeof = %d\n\r", message, now.length());
          }

      } // WiFi.ready()

    Serial.printf("\n\r");

    digitalWrite(ledPin, LOW);

}

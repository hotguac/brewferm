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
//SYSTEM_MODE(SEMI_AUTOMATIC);
//SYSTEM_THREAD(ENABLED);

// SSID and password #define'd in WiFi_Info.h
#include "WiFi_Info.h"
const char* SSID = MY_SSID;
const char* PASSWORD = MY_PASSWORD;

#if Wiring_WiFi
STARTUP(System.enable(SYSTEM_FLAG_WIFITESTER_OVER_SERIAL1));
#endif

int ledPin = D7;

int def_delay;

#include "Sensor.h"
SENSORS mySensors;

#include "pid.h"
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, PID::DIRECT);

#include "relays.h"
RELAYS myRelays;

#include "comms.h"
COMMS myComms;

/* This function is called once at start up ----------------------------------*/
void setup()
{
    //Setup the Tinker application here

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);

    System.disableUpdates();
    //WiFi.clearCredentials();
    //WiFi.setCredentials(SSID, PASSWORD, WPA2);
    WiFi.on();

    delay(10 * 1000);
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
    delay(10 * 1000);

    Serial.begin(9600);
    while(!Serial.available()) Particle.process();

    if (WiFi.ready()) {
        def_delay = 2 * 1000;

        Serial.print("WiFi is ready using ");
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.localIP());
        Particle.connect();
        delay(def_delay);
        Particle.syncTime();
        delay(def_delay);
        Particle.disconnect();
        myComms.init();
        delay(def_delay);
} else {
        def_delay = 5 * 1000;
        Serial.println("WiFi not connected");
    }

    // Init pid fields
    Input = 76.0;
    Setpoint = 76.0;

    // Turn on pid
    myPID.SetMode(PID::AUTOMATIC);
    myPID.SetOutputLimits(64.0,84.0);
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    //This will run in a loop
    digitalWrite(ledPin, HIGH);
    mySensors.refresh();

    Input = mySensors.GetTempF(SENSORS::BEER);
    myPID.Compute();

    myRelays.controlTemp(mySensors.GetTempF(SENSORS::CHAMBER), Output);
    myComms.sendStatus(mySensors, myRelays, Output);

    digitalWrite(ledPin, LOW);
    delay(5000);
}

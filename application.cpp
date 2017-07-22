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
#include "math.h"

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
int greenPin = D0;
int redPin = D1;

int def_delay;

#include "Sensor.h"
SENSORS mySensors;

#include "pid.h"
double Setpoint, Input, Output;
// was 3, 5, 1
PID myPID(&Input, &Output, &Setpoint, 5, 2, 1, PID::DIRECT);

#include "relays.h"
RELAYS myRelays;

#include "comms.h"
COMMS myComms(&Setpoint);

struct MySP {
  char name[9];
  double value;
};

double retrieveSetPoint() {

  delay(10*1000);
  //Serial.printf("Before EEPROM get\r\n");

  MySP mySP;
  EEPROM.get(0, mySP);

  //Serial.printf("EEPROM name=%s\r\n", mySP.name);
  //Serial.printf("EEPROM value=%f\r\n", mySP.value);

  if (strcmp(mySP.name,"SetPoint") != 0) {
    if ((mySP.value < 33.0) || (mySP.value > 80)) {
      strcpy(mySP.name,"SetPoint");
      mySP.value = 74;
      EEPROM.put(0, mySP);
    }
  }

  return mySP.value;
}

/* This function is called once at start up ----------------------------------*/
void setup()
{
    //Setup the Tinker application here
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(ledPin, OUTPUT);

    digitalWrite(ledPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);

    // Init pid fields
    Input = 76.0;
    Setpoint = 74.0;
    Setpoint = retrieveSetPoint();

    // Turn on pid
    myPID.SetMode(PID::AUTOMATIC);
    if (Setpoint > 60) {
      myPID.SetOutputLimits(Setpoint - 20,Setpoint + 12);
    } else {
      myPID.SetOutputLimits(33.0,80.0);
    }

    System.disableUpdates();
    //WiFi.clearCredentials();
    //WiFi.setCredentials(SSID, PASSWORD, WPA2);
    WiFi.on();

    delay(5 * 1000);
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
    delay(5 * 1000);

    //Serial.begin(9600);
    //while(!Serial.available()) Particle.process();

    if (WiFi.ready()) {
        def_delay = 500;

        //Serial.print("WiFi is ready using ");
        //Serial.println(WiFi.SSID());
        //Serial.println(WiFi.localIP());
        Particle.connect();
        delay(def_delay);
        Particle.syncTime();
        delay(def_delay);
        Particle.disconnect();
        myComms.init();
        delay(def_delay);
    } else {
        def_delay = 5 * 1000;
        //Serial.println("WiFi not connected");
    }

}

double getAdjusted(double setp,
                   double beer,
                   double Output)
{
  double target = Output;
  double beer_setp = beer - setp; // positive = too warm

  if (beer_setp > 1.5) { // beer too warm
    target = Output;
  } else if (beer_setp > 0.5) {
      if (Output < (beer - 20.0)) {
        target = beer - 20.0;
      }
  } else if (beer_setp > 0.0) {
    if (Output < (beer - 15.0)) {
      target = beer - 15.0;
    }
  } else if (beer_setp > -0.5) { // beer to cold
    if (Output > (beer + 5.0)) {
      target = beer + 5.0;
    }
  } else if (beer_setp > -1.5) {
    if (Output > (beer + 10.0)) {
      target = beer + 10.0;
    }
  } else {
    target = Output;
  }

  return (target + Output) / 2.0;
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    //This will run in a loop
    mySensors.refresh();

    Input = mySensors.GetTempF(SENSORS::BEER);
    myPID.Compute();

    //double beer_temp = mySensors.GetTempF(SENSORS::BEER);
    double chamber_temp = mySensors.GetTempF(SENSORS::CHAMBER);
    double adj_Output = getAdjusted(Setpoint, Input, Output);
    myRelays.controlTemp(chamber_temp, adj_Output);
    digitalWrite(ledPin, HIGH);

    myComms.sendStatus(mySensors, myRelays, Output, adj_Output);
    if (myComms.setPointAvailable()) {
      Setpoint = myComms.getSetPoint();
    }

    double diff = fabs(Input - Setpoint);

    if (diff < 0.5) {
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
    } else if (diff < 1.0) {
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, HIGH);
    } else {
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
    }

    digitalWrite(ledPin, LOW);
}

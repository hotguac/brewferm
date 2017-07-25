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

double beer_temp;
double chamber_temp;

#include "pid.h"
double Setpoint, Input, Output;
// was 3, 5, 1
// then 5, 2, 1
// then 5, 1.8, 1.5
//then 4.5 1.85 1.7
PID myPID(&Input, &Output, &Setpoint, 4.5, 1.85, 1.9, PID::DIRECT);

#include "relays.h"
RELAYS myRelays;

#include "comms.h"
COMMS myComms(&Setpoint);

struct MySP {
  char name[9];
  double value;
};

void storeSetPoint(double sp) {
  MySP mySP;

  if ((sp > 32.9) && (sp < 80)) {
      strcpy(mySP.name,"SetPoint");
      mySP.value = sp;
      EEPROM.put(0, mySP);
    }
}

double retrieveSetPoint() {

  delay(10*1000);
  //Serial.printf("Before EEPROM get\r\n");

  MySP mySP;
  EEPROM.get(0, mySP);

  //Serial.printf("EEPROM name=%s\r\n", mySP.name);
  //Serial.printf("EEPROM value=%f\r\n", mySP.value);

  if (strcmp(mySP.name,"SetPoint") != 0) {
    if ((mySP.value < 33.0) || (mySP.value > 80)) {
      mySP.value = 66.0;
      storeSetPoint(mySP.value);
    }
  }

  return mySP.value;
}

void adjustLimits(double sp) {
  if (sp > 60) {
    myPID.SetOutputLimits(sp - 6, sp + 4);
  } else {
    myPID.SetOutputLimits(31.0, sp + 8);
  }
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
    Setpoint = 66.0;
    Setpoint = retrieveSetPoint();
    Input = Setpoint;

    // Turn on pid
    myPID.SetMode(PID::AUTOMATIC);
    adjustLimits(Setpoint);

    System.disableUpdates();
    WiFi.on();

    delay(4 * 1000);
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
    delay(4 * 1000);

    Serial.begin(9600);

    if (WiFi.ready()) {
        def_delay = 500;

        Particle.connect();
        delay(def_delay);
        Particle.syncTime();
        delay(def_delay);
        Particle.disconnect();
        myComms.init();
        delay(def_delay);
    } else {
        def_delay = 5 * 1000;
    }
}

// The intent of this routine is to make the PID routine
// react quicker to colder inputs that warmer
// so the chamber doesn't cool down too far resulting in
// beer temp overshoot on the cold side
double adjustInput()
{
  double offset = 0.0;
  double diff = Setpoint - beer_temp;

  if ((diff > 0.05) && (chamber_temp < beer_temp)) {
    // beer is colder than set point and
    // chamber is already colder than beer
    offset = (diff - 0.05) * 0.3;
  }

  return beer_temp - offset;
}

void setIndicatorLEDs(double Input, double Setpoint) {
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
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    //This will run in a loop
    mySensors.refresh();

    chamber_temp = mySensors.GetTempF(SENSORS::CHAMBER);
    beer_temp = mySensors.GetTempF(SENSORS::BEER);

    Input = adjustInput();
    myPID.Compute();

    myRelays.controlTemp(chamber_temp, Output);
    digitalWrite(ledPin, HIGH);

    myComms.sendStatus(mySensors, myRelays, Output, Output);

    myComms.processIncomingMessages();

    if (myComms.setPointAvailable()) {
      Setpoint = myComms.getSetPoint();
      adjustLimits(Setpoint);
      storeSetPoint(Setpoint);
    }

    setIndicatorLEDs(Input, Setpoint);
    delay(500); // temps don't change too quickly, we can wait
    digitalWrite(ledPin, LOW);
}

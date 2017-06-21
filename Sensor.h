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

#ifndef SENSORS_h
#define SENSORS_h

#include "DS18.h"

class SENSORS
{
  public:

    //Parameter types for some of the functions below
    enum mode_t { BEER = 0, CHAMBER = 1, AMBIENT = 2 };
    const char *types[3] = {"2875857F08000063",
                            "2874AD7F08000092",
                            "0000000000000000"};

    //commonly used functions **************************************************************************
    SENSORS(void);
    void refresh(void);


    mode_t GetMode(void);                 // * sets PID to either MANUAL (0) or AUTOMATIC (1)
    void printDebugInfo(void);

    //available but not commonly used functions ********************************************************
    void SetSampleTime(int);              // * sets the frequency, in Milliseconds, with which
                                      //   the PID calculation is performed.  default is 100

    //Display functions ****************************************************************
    double GetID(mode_t);						  // These functions query the pid for interal values.
    double GetTempF(mode_t);						  //  they were created mainly for the pid front-end,
    double GetTempC(mode_t);						  // where it's important to know what is actually

  private:

    void Initialize();
    float beerF;
    float beerC;
    float chamberF;
    float chamberC;
    float ambientF;
    float ambientC;

    unsigned long lastTime;

    double UNKNOWN = -999.0;

};


#endif

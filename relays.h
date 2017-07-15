#ifndef RELAYS_h
#define RELAYS_h

#include "application.h"

class RELAYS
{
  public:

  //Parameter types for some of the functions below
    enum mode_t { OFF = 0, ON = 1, PENDING = 2};
    const char *types[3] = {"2875857F08000063","2874AD7F08000092","0000000000000000"};

  //commonly used functions **************************************************************************
    RELAYS(void);
    void heatON(void);
    void heatOFF(void);
    void coolON(void);
    void coolOFF(void);

    void controlTemp(double current, double target);

    mode_t getHeatStatus();
    mode_t getCoolStatus();

  private:
    int relayCoolPin = D4;
    int relayHeatPin = D5;

    double min_heat_off_on = 60;  // seconds
    double min_cool_heat   = 60;  // seconds
    double min_cool_off_on = 300; // seconds
    double min_heat_cool   = 120; // seconds

	  //unsigned long heatOnTime;
    mode_t heatStatus;
    mode_t coolStatus;

    time_t ts_heatON;
    time_t ts_heatOFF;
    time_t ts_coolON;
    time_t ts_coolOFF;
};

#endif

#ifndef RELAYS_h
#define RELAYS_h

#include "application.h"

class RELAYS
{
  public:

  //Parameter types for some of the functions below
    enum mode_t { OFF = 0, ON = 1};
    const char *types[3] = {"2875857F08000063","2874AD7F08000092","0000000000000000"};

  //commonly used functions **************************************************************************
    RELAYS(void);
    void heatON(void);
    void heatOFF(void);
    void coolON(void);
    void coolOFF(void);

    mode_t getHeatStatus();
    mode_t getCoolStatus();

  private:
    int relayCoolPin = D4;
    int relayHeatPin = D5;

	  unsigned long heatOnTime;
    mode_t heatStatus;
    mode_t coolStatus;

};

#endif

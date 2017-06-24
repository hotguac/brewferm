#ifndef COMMS_h
#define COMMS_h

#include "application.h"

class COMMS
{
  public:

  //Parameter types for some of the functions below

  //commonly used functions **************************************************************************
    COMMS(void);
    void sendStatus(SENSORS mySensors, RELAYS myRelays, double Output);
    void init(void);

  private:
    time_t ts_lastSend;

};

#endif

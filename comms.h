#ifndef COMMS_h
#define COMMS_h

#include "application.h"
#include "relays.h"

class COMMS
{
  public:

  //Parameter types for some of the functions below
  //commonly used functions **************************************************************************
    COMMS(double *sp);
    void sendStatus(SENSORS mySensors, RELAYS myRelays, double Output);
    void init(void);
    int setPointAvailable();
    double getSetPoint();

  private:
    time_t ts_lastSend;
    double last_beerF;
    double last_chamberF;
    RELAYS::mode_t last_heatStatus;
    RELAYS::mode_t last_coolStatus;
    int min_send_time;
};

#endif

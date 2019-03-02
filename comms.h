/*
 ******************************************************************************
  Copyright (c) 2019 Joe Kokosa  All rights reserved.

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

#ifndef COMMS_H_
#define COMMS_H_

#include "application.h"
#include "brewferm.h"

class COMMS {
 public:
  // Parameter types for some of the functions below
  // commonly used functions
    explicit COMMS(void);
    void sendStatus(double set_point, double beerF,
                    double ch_target, double chamberF,
                    double chamber_pid_out,
                    mode_heat_cool system_status,
                    mode_heat_cool relay_status);

    void init(void);
    int setPointAvailable();
    int processIncomingMessages();
    double getSetPoint();
    void changeSetPoint();
 private:
    time_t ts_lastSend;
    double last_beerF;
    double last_chamberF;
    mode_heat_cool last_relay_status;
    int min_send_time;
    void setFuture(char *buffer);
};

#endif  // COMMS_H_

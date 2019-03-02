#include <string>

#include "application.h"

#include "comms.h"
#include "brewferm.h"

#define BUF_SIZE 2048

IPAddress remoteAddress1(192, 168, 0, 11);  // laptop
IPAddress remoteAddress2(192, 168, 0, 20);  // server

int remotePort = REMOTE_PORT;
int localPort = LOCAL_PORT;

UDP Udp;
char buffer[BUF_SIZE];
int setPointReceived = 0;
double setPoint;
//double *future_sp;
time_t future_time;


// TODO(Joe): remove set point storage from the comms class
/*----------------------------------------------------------------------------*/
COMMS::COMMS(void) {
  //setPoint = sp;
  ts_lastSend = Time.now();
  last_beerF = 0.0;  // default so we get first reading
  last_chamberF = 0.0;
  min_send_time = TRANSMIT_INTERVAL;  // seconds without sending something
}

void COMMS::init(void) {
  Udp.begin(localPort);
  Udp.setBuffer(BUF_SIZE);
}

void COMMS::sendStatus(double set_point, double beerF,
                      double chamber_target, double chamberF,
                      double chamber_pid_out,
                      mode_heat_cool system_status,
                      mode_heat_cool relay_status) {
  time_t current_time  = Time.now();

  if (difftime(current_time, ts_lastSend) > min_send_time) {
        last_relay_status = relay_status;
        last_beerF = beerF;
        last_chamberF = chamberF;
        ts_lastSend = current_time;

        std::string now(Time.format(current_time, TIME_FORMAT_ISO8601_FULL));
        now.append("|");

        snprintf(buffer, BUF_SIZE,
          "SP %.1fF|Br %.1fF|CT %.1fF|Ch %.1fF|X %5.1f|St %d|Rl %d\r\n",
                      set_point,
                      beerF,
                      chamber_target,
                      chamberF,
                      chamber_pid_out,
                      system_status,
                      relay_status);

        now.append(buffer);

        const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());

        if (WiFi.ready()) {
          unsigned int len = Udp.sendPacket(message,
                                            now.length(),
                                            remoteAddress1,
                                            remotePort);

          // delay allows system background processes to run, so let's
          // give it some time to get caught up
          delay(DELAY_TIME);

          len =
            Udp.sendPacket(message, now.length(), remoteAddress2, remotePort);

         delay(DELAY_TIME);

          if (len == now.length()) {
            // Serial.printf("%s", message);
          }
        } else {
           Serial.printf("not sent = %s", message);
        }  // WiFi.ready()

        delay(DELAY_TIME);
      }
}

struct MySP {
  char name[9];
  double value;
};

/*----------------------------------------------------------------------------*/
int COMMS::processIncomingMessages() {
  int available = 0;

  Udp.parsePacket();
  if (Udp.available()) {
    int count = Udp.read(buffer, BUF_SIZE);

    if (count < (BUF_SIZE - 2)) {
      buffer[count] = ' ';  // terminate the received string
      buffer[count+1] = 0;
    }

    switch (buffer[0]) {
      case 'd' :  System.dfu();  // dfu or reset
                  break;
      case 's' :  setPoint = atof(buffer + 1);
                  setPointReceived = 1;
                  break;
      //case 'n' :  setFuture(buffer + 1);
      //            break;
    }
  }

  return available;
}

/*----------------------------------------------------------------------------*/
int COMMS::setPointAvailable() {
  return setPointReceived;
}

/*----------------------------------------------------------------------------*/
double COMMS::getSetPoint() {
  return setPoint;
}

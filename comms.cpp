#include "Sensor.h"
#include "comms.h"

#include "application.h"

#define BUF_SIZE 2048

IPAddress remoteAddress1(192,168,0,11);  // laptop
IPAddress remoteAddress2(192,168,0,20);  // server
int remotePort = 7777;

int localPort = 8888;
UDP Udp;
char buffer[BUF_SIZE];
int setPointReceived = 0;
double *setPoint;

COMMS::COMMS(double *sp) {
  setPoint = sp;
  ts_lastSend = Time.now();
  last_beerF = 0.0; //default so we get first reading
  last_chamberF = 0.0;
  min_send_time = 20; // seconds without sending something
}

void COMMS::init(void) {
  Udp.begin(localPort);
  if (Udp.setBuffer(BUF_SIZE)) {
    //Serial.println("Udp buffer set");
  } else {
    //Serial.println("Udp buffer set failed");
  }
}

void COMMS::sendStatus(SENSORS mySensors,
                       RELAYS myRelays,
                       double Output,
                       double adj_Output) {

  time_t current_time  = Time.now();

  double beerF = mySensors.GetTempF(SENSORS::BEER);
  double chamberF = mySensors.GetTempF(SENSORS::CHAMBER);

  RELAYS::mode_t heatStatus = myRelays.getHeatStatus();
  RELAYS::mode_t coolStatus = myRelays.getCoolStatus();

  if ((difftime(current_time, ts_lastSend) > min_send_time) ||
      (abs(last_beerF - beerF) > 0.1) ||
      (abs(last_chamberF - chamberF) > 0.1) ||
      (heatStatus != last_heatStatus) ||
      (coolStatus != last_coolStatus)) {

        last_heatStatus = heatStatus;
        last_coolStatus = coolStatus;
        last_beerF = beerF;
        last_chamberF = chamberF;
        ts_lastSend = current_time;

        std::string now(Time.format(current_time, TIME_FORMAT_ISO8601_FULL));
        now.append("|");

        sprintf(buffer, "SP %.1fF|Br %.1fF|Ch %.1fF|OP %.1fF|AO %.1fF|Ht %d|Cl %d\r\n",
                      *setPoint,
                      beerF,
                      chamberF,
                      Output,
                      adj_Output,
                      heatStatus,
                      coolStatus);

        now.append(buffer);

        const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());
        if (WiFi.ready()) {
          unsigned int len = Udp.sendPacket(message,
                                            now.length(),
                                            remoteAddress1,
                                            remotePort);
          len = Udp.sendPacket(message,
                                            now.length(),
                                            remoteAddress2,
                                            remotePort);
          if (len == now.length()) {
            //Serial.printf("%s", message);
          }
        } else {
          //Serial.printf("not sent = %s", message);
        } // WiFi.ready()
      }
}

struct MySP {
  char name[9];
  double value;
};

int COMMS::processIncomingMessages() {
  int available = 0;
  //float temp;

  Udp.parsePacket();
  if (Udp.available()) {
    int count = Udp.read(buffer, BUF_SIZE);
    buffer[count] = ' '; // terminate the received string
    buffer[count+1] = 0;

    // this is a kludge for now
    if (buffer[0] == 'd') {
      System.dfu(); // look for dfu or reset
    }

    *setPoint = atof(buffer);
    setPointReceived = 1;
  }

  return available;
}

int COMMS::setPointAvailable() {
  return setPointReceived;
}

double COMMS::getSetPoint() {
  return *setPoint;
}

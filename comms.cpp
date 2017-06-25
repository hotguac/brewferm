#include "Sensor.h"
#include "comms.h"

#include "application.h"

IPAddress remoteAddress(192,168,0,11);  // laptop
int remotePort = 7777;

int localPort = 8888;
UDP Udp;
char buffer[2048];

COMMS::COMMS(void) {
  ts_lastSend = Time.now();
  last_beerF = 0.0; //default so we get first reading
  last_chamberF = 0.0;
  min_send_time = 120; // seconds without sending something
}

void COMMS::init(void) {
  Udp.begin(8888);
}

void COMMS::sendStatus(SENSORS mySensors, RELAYS myRelays, double Output) {
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

        sprintf(buffer, "Br %.1fF|Ch%.1fF|Out%.1fF|H%d|C%d\r\n",
                      beerF,
                      chamberF,
                      Output,
                      heatStatus,
                      coolStatus);

        now.append(buffer);

        const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());
        if (WiFi.ready()) {
          unsigned int len = Udp.sendPacket(message,
                                            now.length(),
                                            remoteAddress,
                                            remotePort);
          if (len == now.length()) {
            Serial.printf("%s", message);
          }
        } else {
          Serial.printf("not sent = %s", message);
        } // WiFi.ready()
      }
}

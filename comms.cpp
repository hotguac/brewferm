#include "relays.h"
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
}

void COMMS::init(void) {
  Udp.begin(8888);
}

void COMMS::sendStatus(SENSORS mySensors, RELAYS myRelays, double Output) {
  time_t time = Time.now();
  std::string now(Time.format(time, TIME_FORMAT_ISO8601_FULL));
  now.append("|");

  Serial.printf("Beer %.2f F , Chamber%.2f F , Output%.2f F\r\n",
                mySensors.GetTempF(SENSORS::BEER),
                mySensors.GetTempF(SENSORS::CHAMBER),
                Output);

  sprintf(buffer, "Beer %.2f F | Chamber%.2f F ",
                mySensors.GetTempF(SENSORS::BEER),
                mySensors.GetTempF(SENSORS::CHAMBER));

  now.append(buffer);

  sprintf(buffer, "%4.1f", Output);
  now.append("|PID Output=");
  now.append(buffer);
  now.append("F\n\r");

  const uint8_t* message = reinterpret_cast<const uint8_t*>(now.c_str());
  if (WiFi.ready()) {
    unsigned int len = Udp.sendPacket(message,
                                      now.length(),
                                      remoteAddress,
                                      remotePort);
    if (len == now.length()) {
      Serial.printf("message = %s", message);
    }
  } else {
    Serial.printf("not sent = %s", message);
  } // WiFi.ready()
}

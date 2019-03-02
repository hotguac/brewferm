#!/usr/bin/env python

import socket
import sys

UDP_IP = "192.168.0.12"
UDP_PORT = 8888
MESSAGE = 's' + sys.argv[1]

print >>sys.stderr, MESSAGE

sock = socket.socket(socket.AF_INET,
                  socket.SOCK_DGRAM)

sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

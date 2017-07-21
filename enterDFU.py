import socket
import sys

UDP_IP = "192.168.0.18"
UDP_PORT = 8888
MESSAGE = "dfu";

sock = socket.socket(socket.AF_INET,
                  socket.SOCK_DGRAM)

sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

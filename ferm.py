import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('192.168.0.11', 7777)
print >>sys.stderr, 'starting'
sock.bind(server_address)

while True:
    print >>sys.stderr, 'waiting'
    data, address = sock.recvfrom(4096)

    #print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
    #print >>sys.stderr, data

    if (address == ('192.168.0.18', 8888)):
        print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
        print >>sys.stderr, data

import socket
import sys
import sqlite3

conn = sqlite3.connect('readings.db')

c = conn.cursor()

#create table
try:
    #c.execute('''DROP TABLE readings''')
    c.execute('''CREATE TABLE readings
             (ts date, beer real, setpoint real,
             chamber real, output real, heat integer, cool integer)''')
    conn.commit()
except: # catch all errors
    print >>sys.stderr, 'table already exists'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('192.168.0.11', 7777)
print >>sys.stderr, 'starting'
sock.bind(server_address)

print >>sys.stderr, 'waiting'

while True:
    data, address = sock.recvfrom(4096)

    #print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
    #print >>sys.stderr, data

    if (address == ('192.168.0.18', 8888)):
        #print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
        #print >>sys.stderr, data
        #print >>sys.stderr, '12345678901234567890123456789012345678901234567890123456789012345678901234567890'
        ts = data[:20]
        sp = data[24:28]
        beer = data[33:37]
        chamber = data[42:46]
        output = data[51:55]
        heat = data[60:61]
        cool = data[65:66]

        parms = (ts, sp, beer, chamber, output, heat, cool)

        print >>sys.stderr, 'TS %s  SP %s  Beer %s  Ch %s  Out %s  Ht %s  Cl %s' % (ts,sp,beer,chamber,output,heat,cool)
        #print >>sys.stderr, 'set point %s beer %s' % (sp,beer)
        #print >>sys.stderr, 'chamber %s output %s' % (chamber, output)
        #print >>sys.stderr, 'heat %s cool %s' % (heat, cool)

        try:
            c.execute('INSERT INTO readings VALUES (?, ?, ?, ?, ?, ?, ?)', parms)
            conn.commit()
        except Exception as e:
            print >>sys.stderr, 'insert failed %s' % (e,)

        #print >>sys.stderr, ''

#!/usr/bin/env python

import socket
import sys
import sqlite3

conn = sqlite3.connect('readings.db')

c = conn.cursor()

#create table
try:
    # c.execute('''DROP TABLE readings3''')
    c.execute('''CREATE TABLE readings3
             (addr varchar(25), ts date, setpoint real, beer real,
             ch_target real, chamber real, c_out real,
             system_status integer,
             relays_status integer)''')

    conn.commit()
except: # catch all errors
    print >>sys.stderr, 'table already exists'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('192.168.0.11', 7777)
print >>sys.stderr, 'starting'
sock.bind(server_address)

print >>sys.stderr, 'waiting'

while True:
    data, (address, socknum) = sock.recvfrom(4096)

    #if (address == ('192.168.0.18', 8888)):

    if (1 == 1):
        #print >>sys.stderr, 'received %s bytes from %s' % (len(data), address)
        #print >>sys.stderr, data
        # example             2019-01-29T22:15:54Z|SP 65.0F|Br 61.3F|CT 71.0F|Ch 74.5F|X   1.0|St 0|Rl 1
        #                      ('192.168.0.13', 8888)
        #print >>sys.stderr, '12345678901234567890123456789012345678901234567890123456789012345678901234567890'
        #print >>sys.stderr, '0         1         2         3         4         5         6         7'

        split_up = data.split('|SP ')
        ts = split_up[0]

        s2 = split_up[1].split('F|')
        sp = s2[0]

        split_up = data.split('|Br ')
        s2 = split_up[1].split('F|')
        beer = s2[0]

        split_up = data.split('|CT ')
        s2 = split_up[1].split('F|')
        target = s2[0]

        split_up = data.split('|Ch ')
        s2 = split_up[1].split('F|')
        chamber = s2[0]

        split_up = data.split('|X ')
        s2 = split_up[1].split('|St ')
        c_out = s2[0]

        split_up = data.split('|St ')
        s2 = split_up[1].split('|Rl ')
        sys_stat = s2[0]
        relays = s2[1]

        if (relays[:1] == '1'):
            relays = 'Cool'

        if (relays[:1] == '2'):
            relays = 'Idle'

        if (relays[:1] == '3'):
            relays = 'Heat'

        if (sys_stat[:1] == '1'):
            sys_stat = 'Cool'

        if (sys_stat[:1] == '2'):
            sys_stat = 'Idle'

        if (sys_stat[:1] == '3'):
            sys_stat = 'Heat'

        parms = (address, ts, sp, beer, target, chamber, c_out, relays, sys_stat)

        if (sys_stat == relays):
            print >>sys.stderr, '%s - %s Beer (%s : %s) Chamber (%s : %s) Control (%s) System (%s : %s)' % parms
        else:
            print >>sys.stderr, '%s - %s Beer (%s : %s) Chamber (%s : %s) Control (%s) System (%s : %s)*' % parms

        #print >>sys.stderr, 'TS %s  SP %s  BeerF %s  Chamber Target %s ChamberF %s  Relays %s ;' % (ts,sp,beer,target,chamber,relays)

        try:
            c.execute('INSERT INTO readings3 VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)', parms)
            conn.commit()
        except Exception as e:
            print >>sys.stderr, 'insert failed %s' % (e,)

        #print >>sys.stderr, ''

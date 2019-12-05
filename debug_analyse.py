#!/usr/bin/python2
#coding:utf8

END = chr(0)
END_change = chr(0xFD)
END_change_change = chr(0xFC)

import serial, sys
port = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=0.1)

if len(sys.argv)>1:
    cmd = sys.argv[1]
    print cmd
    if cmd[:3]=='ST:':
        time = int(cmd[3:])
        print time
        cmd = 'ST' + chr(time>>24) + chr((time>>16)&0xFF) + chr((time>>8)&0xFF) + chr(time&0xFF)
    cmd = cmd.replace(END_change,END_change+END_change_change)
    cmd = cmd.replace(END,END_change+END_change)
    cmd += END
    print cmd
    port.write(cmd)
#    exit(0)

import re

packet=''
s=''
while(1):
    s += port.readall()
    pack,s=s.split(END,1) if END in s else (None,s)
    if pack:
        pack = pack.replace(END_change+END_change,END)
        pack = pack.replace(END_change+END_change_change,END_change)
        pack = re.search('(\w+:)(.*)',pack,re.DOTALL)
        if pack==None: continue
        num=0
        for i in range(len(pack.group(2))):
            num += ord(pack.group(2)[~i])<<(i*8) 
        print pack.group(1), num
#    while (END in s):
#        packet += s.split(END,1)[0]
#        packet=packet.replace(END_change+END_change,END)
#        packet=packet.replace(END_change+END_change_change,END_change)
#        #print len(packet)
#        #for i in packet:
#        #    print hex(ord(i)),
#        #print hex(ord(END))
#        while ('ALR:' in packet):
#            beg,end=packet.split('ALR:',1)
#            beg+='ALARM:'
#            msbyte=((ord(end[0])<<8)+ord(end[1]))
#            lsbyte=((ord(end[2])<<8)+ord(end[3]))
#            time=(msbyte<<16)+lsbyte
#            end=str(time)+' '+end[4:]
#            packet=beg+end
#        while ('ACT:' in packet):
#            beg,end=packet.split('ACT:',1)
#            beg+='ACTION:'
#            number=ord(end[0])
#            end=str(number)+' '+end[1:]
#            packet=beg+end
#        print packet
#        packet = ''
#        s = s.split(END,1)[1]
##        for i in packet:
##            print hex(ord(i)),
##        print hex(ord(END))
##        packet = s.split(END)[1]
#    packet += s
            

        

#!/usr/bin/python2
#coding:utf8

END = chr(0)
END_change = chr(0xFD)
END_change_change = chr(0xFC)

import serial, sys, time
port = serial.Serial('/dev/ttyS0', baudrate=9600, timeout=0.2)

if "-bt05" in sys.argv:
    port.write("AT+ROLE\r\n");
    ROLE=port.readall()
    if ROLE.strip()=="+ROLE=0":
        port.write("AT+ROLE1\r\n");
        ROLE=port.readall()
        if ROLE.strip()!="+ROLE=1":
            print "Error master mode selected"
            exit(-1)
    port.write("AT+BAND\r\n");
    CONN=port.readall()
    if not ("Connected" in CONN):
        ind=sys.argv.index("-bt05")
        if len(sys.argv) > ind+1:
            bt_addr=sys.argv[ind+1]
            try:
                int(bt_addr,16)
            except:
                print "Address of connecting device must be hex!"
                exit(-3)
            port.write("AT+BAND%s\r\n" %(bt_addr));
            port.write("AT+BAND\r\n");
            time.sleep(1)
            CONN=port.readall()
            if "Connected" not in CONN:
                print "Error connection device"
                exit(-2)
        else:
            print "Scanning devices: ...."
            port.write("AT+INQ\r\n");time.sleep(1);
            SCAN=port.readall()
            print SCAN
            exit(1)
        
if len(sys.argv)>1:
    cmd = sys.argv[1]
    print cmd
    if cmd[:3]=='ST:':
        time = int(cmd[3:])
        print time
        cmd = 'ST' + chr(time>>24) + chr((time>>16)&0xFF) + chr((time>>8)&0xFF) + chr(time&0xFF)
    elif cmd[:3] in ("Rb:","Rh:","Rw:","Rs:"):
        addr=int(cmd[3:],16)
        cmd = cmd[:2] + chr(addr&0xFF) + chr((addr>>8)&0xFF) + chr((addr>>16)&0xFF) + chr(addr>>24) 
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
        if pack.group(1)=='s:':
            print pack.group(1), pack.group(2)
        else:
        #if pack.group(1) in ('ALR:','ACT:','b:','h:','w:')
            for i in range(len(pack.group(2))):
                num += ord(pack.group(2)[~i])<<(i*8) 
            print pack.group(1), num
        if "-i" not in sys.argv:
            exit(0)
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
            

        

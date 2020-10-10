#!/usr/bin/python2
#coding:utf8

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

from interact import *
interact=interact()

if "-g" in sys.argv:
    interact.debug=1;
    sys.argv.remove("-g")
else:
    debug=None;
        
# Example commands: ./debug_analyse.py  Rw:2000080c
if len(sys.argv)>1:
    print interact.interact(sys.argv[1])
#    cmd = interact.cmd_gen(sys.argv[1])
#    interact.transmit_cmd(cmd)
#
#if not interact.R: # Command without read
#    exit(0)
#
#print interact.recieve_ans()

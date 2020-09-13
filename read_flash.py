#!/usr/bin/python2
#coding:utf8

packet_size=250 # Count of byte readable from flash at one time

import sys,os,time

if '+' in sys.argv:
    sys.argv.remove('+')
    f=open('flash','a')
else:
    f=open('flash','w')

if len(sys.argv)<3:
    count=16*4096*64
else:
    count = int(sys.argv[2])

if len(sys.argv)<2:
    addr=0
else:
    addr = int(sys.argv[1])
    count-=addr

from interact import *
interact=interact()

def interrupt():
    f.close()

print interact.interact("RWb:422181b0=1") # NSS pin high
print interact.interact("RWb:422181b0=0") # NSS pin low
print interact.interact("RWb:4000380C=3") # READ command
print interact.interact("RWb:4000380C=%s" %(hex((addr>>16)&0xFF)[2:])) # MSB of addr
print interact.interact("RWb:4000380C=%s" %(hex((addr>>8)&0xFF)[2:])) # Second byte of addr
print interact.interact("RWb:4000380C=%s" %(hex(addr&0xFF)[2:])) # LSB byte of addr
while(count>0):
    print 'Lost bytes reading - ',count, ', Addr current is ',addr
    if count>packet_size:
        data = interact.interact("RWb:4000380C=0*%s" %(hex(packet_size)))
        data_len=len(data.split())
        if data_len!=packet_size:continue
        count-=data_len
        addr+=data_len
    else:
        data = interact.interact("RWb:4000380C=0*%s" %(hex(count)))
        data_len=len(data.split())
        if data_len!=count:continue
        count-=data_len
        addr+=data_len
    if data!='':
        f.write(data)
        f.flush()
print interact.interact("RWb:422181b0=1") # NSS pin high

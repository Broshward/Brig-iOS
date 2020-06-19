#!/usr/bin/python2
#coding:utf8

import sys,os,time

if '+' in sys.argv:
    sys.argv.remove('+')
    f=open('flash','a')
else:
    f=open('flash','w')

addr = int(sys.argv[1])
count = int(sys.argv[2])


from interact import *
interact=interact()

def interrupt():
    f.close()

print interact.interact("RWb:422181b0=0") # NSS pin low
print interact.interact("RWb:4000380C=3") # READ command
print interact.interact("RWb:4000380C=%s" %(hex((addr>>16)&0xFF)[2:])) # MSB of addr
print interact.interact("RWb:4000380C=%s" %(hex((addr>>8)&0xFF)[2:])) # Second byte of addr
print interact.interact("RWb:4000380C=%s" %(hex(addr&0xFF)[2:])) # LSB byte of addr
while(count>0):
    if count>4:
        data = interact.interact("RWb:4000380C=0*4")
        count-=4
        addr+=4
    else:
        data = interact.interact("RWb:4000380C=0*%s" %(count))
        addr+=count
        count=0
    print 'Lost bytes reading - ',count, ', Addr current is ',addr
    f.write(data)
    f.flush()
print interact.interact("RWb:422181b0=1") # NSS pin high

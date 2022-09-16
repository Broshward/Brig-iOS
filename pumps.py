#!/usr/bin/python2
#coding:utf8

usage = 'usage: pumps.py [left|right|power|all on|off]'
import sys, os, time
from interact import *
interact=interact()

GPIOB_BSRR =  0x40010C10
GPIOB_BRR  =  0x40010C14
GPIOB_ODR =  0x40010C0C
left_pump_bit=4
right_pump_bit=2
power_bit=8

if len(sys.argv)==1:
    value=interact.interact("Rw:%x" %(GPIOB_ODR))

    print 'Pump left is ','ON' if (int(value)&(1<<left_pump_bit))!=0 else 'OFF'
    print 'Pump right is ','ON' if (int(value)&(1<<right_pump_bit))!=0 else 'OFF'
    print 'Pumps power is ','ON'  if (int(value)&(1<<power_bit))!=0 else 'OFF'
    print 'GPIOB_ODR = 0b%s' %(bin(int(value)))
    exit(0)

elif sys.argv[1] == 'left':
    value = 1<<left_pump_bit
elif sys.argv[1] == 'right':
    value = 1<<right_pump_bit
elif sys.argv[1] == 'all':
    value = (1<<left_pump_bit) | (1<<right_pump_bit)
elif sys.argv[1] == 'power':
    value = 0
else:
    print "Error pump detect !!!!"
    print usage
    exit(1)

if sys.argv[2] == 'on':
    port = GPIOB_BSRR
elif sys.argv[2] == 'off':
    port = GPIOB_BRR
else:
    print 'Error action detect !!!!'
    print usage
    exit(1)

    
interact.interact("Ww:%x=%x" %(port,1<<power_bit)) #For relay power UP
time.sleep(0.5)
interact.interact("Ww:%x=%x" %(port,value))

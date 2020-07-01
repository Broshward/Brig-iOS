#!/usr/bin/python2
#coding:utf8

usage = 'usage: pump.py left|right|all on|off'
import sys, os

GPIOB_BSRR =  0x40010C10
GPIOB_BRR  =  0x40010C14
left_pump_bit=2
right_pump_bit=4
if sys.argv[1] == 'left':
    value = 1<<left_pump_bit
elif sys.argv[1] == 'right':
    value = 1<<right_pump_bit
elif sys.argv[1] == 'all':
    value = (1<<left_pump_bit) | (1<<right_pump_bit)
    value |= 0x100 #For relay power UP
else:
    print "Error pump detect !!!!"
    print usage
    exit(1)

if sys.argv[2] == 'on':
    value |= 0x100 #For relay power UP
    port = GPIOB_BSRR
elif sys.argv[2] == 'off':
    port = GPIOB_BRR
else:
    print 'Error action detect !!!!'
    print usage
    exit(1)

from interact import *
interact=interact()
interact.interact("Ww:%x=%x" %(port,value))


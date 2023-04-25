#!/usr/bin/python2
#coding:utf8

ADC_channels=[
"(ADC_IN0):",
"(ADC_IN1):",
"(ADC_IN2):",
"(ADC_IN3):",
"(ADC_IN4):",
"(ADC_IN5):",
"(ADC_IN6):",
"(ADC_IN7):",
"(ADC_IN8):",
"(ADC_IN9):",
"(ADC_IN16):",
"(ADC_IN17):"
]

ADC_max=4095
ADC_min=0

def flags_parce(flags):
    return    

def ADC_illustrate(channels, ADCs):
    for i in range(len(channels)):
        print channels[i],'\t',ADCs[i]
    return
    
import os
from interact import *
interact=interact()

jdata_base = int(os.popen('readelf -s main.elf |grep jdata').read().strip().split()[1], 16)

time_stamp = interact.interact("Rw:%x" %(jdata_base))

ADC_values = interact.interact("Rh:%x*%x" %(jdata_base+4,12))
print 'ADC channels 0...11: ', ADC_values.split()
ADC_illustrate(ADC_channels, ADC_values.split(','))
print

flags_addr = int(os.popen('readelf -s main.elf |grep flags').read().strip().split()[1], 16)
print 'Flags addr: 0x%x' %(flags_addr)
flags =  interact.interact("R4:%x" %(flags_addr))
flags = '{:032b}'.format(int(flags,16))
print 'Flags: ',
for i in range(4):
    print flags[i*8:(i+1)*8],' ',

#interact.interact("Ww:%x=0" %(flags_addr))

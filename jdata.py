#!/usr/bin/python2
#coding:utf8

ADC_channels=[
{0:"(ADC_IN0)   Luminosity", "T":0.525, 'E1':237, "Re1":2370},
{0:"(ADC_IN1)   Nutrien solution temperature, Bottom, INPUT",  "B":3337, "R":9550, "T":25},
{0:"(ADC_IN2)   Nutrien solution temperature, Bottom, OUTPUT",  "B":3353.5, "R":9609, "T":25},
{0:"(ADC_IN3)   Nutrien solution temperature, Top, OUTPUT",  "B":3357.6, "R":9596.8, "T":25},
{0:"(ADC_IN4)   Nutrien solution temperature, Top, INPUT",  "B":3364, "R":9469, "T":25},
{0:"(ADC_IN5)   Center  center  bottom temperature", "B":3377.5, "R":9502.7, "T":25},
{0:"(ADC_IN6)   Center  center  Top temperature", "B":3362, "R":9405, "T":25},
{0:"(ADC_IN7)   Front   right   bottom temperature", "B":3356, "R":9530, "T":25},
{0:"(ADC_IN8)   Front   center  bottom temperature", "B":3355.67, "R":9459.67, "T":25},
{0:"(ADC_IN9)   Outdoor temperature", "B":3348, "R":9554, "T":25},
{0:"(ADC_IN16)  Analog voltage"      },
{0:"(ADC_IN17)  Internal temperature" }
]

ADC_max=4095
ADC_min=0
R_pullup=2e3
R_NTC=10e3
B_NTC=3950

def flags_parce(flags):
    return    

from NTC import Temp
from LUX import Lux

def ADC_illustrate(channels, ADCs):
    for i in range(len(channels)):
        ADC=int(ADCs[i])
        if 'reserved' in channels[i][0]: continue
        elif i==10:
            print '%-50s' %(channels[i][0])+': ', 1.2*4095/ADC
        elif i==11 :
            print '%-50s' %(channels[i][0])+': ', ADC
        elif 'Luminosity' in channels[i][0]:
            current_resistance = ADC*R_pullup/(ADC_max-ADC_min-ADC)
            current_luminosity = Lux(current_resistance,channels[i]['T'],channels[i]['E1'],channels[i]["Re1"])
            print '%-50s' %(channels[i][0])+': ',current_luminosity
        elif 'temperature' in channels[i][0]:
            current_resistance = ADC*R_pullup/(ADC_max-ADC_min-ADC)
            current_temperature = Temp(current_resistance,channels[i]['B'],channels[i]['R'],channels[i]["T"]+273.15)
            print '%-60s' %(channels[i][0])+': ',current_temperature-273.15
            
    return
    
import os

jdata_base = int(os.popen('readelf -s main.elf |grep jdata').read().strip().split()[1], 16)

time_stamp = int(os.popen('./debug_analyse.py Rw:%x' %(jdata_base)).read())
date = os.popen('date -d @%s' %(time_stamp)).read()
print 'Date: %s' %(date)

ADC_values = os.popen('./debug_analyse.py Rh:%x*%x' %(jdata_base+4,12)).read()
print 'ADC channels 0...11: ', ADC_values.split()
ADC_illustrate(ADC_channels, ADC_values.split()[:12])
print
average = int(os.popen('readelf -s main.elf |grep ADC_average').read().strip().split()[1], 16)
ADC_values = os.popen('./debug_analyse.py Rh:%x*%x' %(average,12)).read()
print 'Average values 0...11: ', ADC_values.split()
ADC_illustrate(ADC_channels, ADC_values.split()[:12])


flags =  int(os.popen('./debug_analyse.py Rw:%x' %(jdata_base+4+12)).read())
print 'Flags: {:032b}'.format(flags)


draw ="""

"""

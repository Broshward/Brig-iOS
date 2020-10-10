#!/usr/bin/python2
#coding:utf8

TIM=0x40000800
ARR=0x2c
CCR3=0x3c
CCR4=0x40

import sys
pwm=int(sys.argv[1])
period=int(sys.argv[2])
arr=period
ch1=pwm

from interact import *
interact=interact()
interact.debug=1

#print interact.interact("") 
cmd="CR%x=%x,%x=%x" %(TIM+ARR,arr,TIM+CCR3,ch1)
print cmd
#sleep(1)
interact.interact(cmd)
cmd="CR*%x,*%x,*%x" %(TIM+ARR,TIM+CCR3,TIM+CCR4)
print cmd
print interact.interact(cmd)
#print interact.interact("RWh:%x=%x" %(TIM+ARR,arr)) # Timer4 ARR register (Period of timer/counter)
#print interact.interact("RWh:%x=%x" %(TIM+CCR3,ch1)) # Timer4 CCR3 register
#print interact.interact("RWh:%x=%x" %(TIM+CCR4,ch2)) # Timer4 CCR4 register

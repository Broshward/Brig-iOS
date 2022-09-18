#!/usr/bin/python2

import sys,os
from interact import *

interact = interact()
ODR=0xC
GPIOA = 0x40010800
GPIOB = 0x40010C00
GPIOC = 0x40011000
GPIOD = 0x40011400
GPIOE = 0x40011800

print 'GPIOA output drive state is:'
print '{:032b}'.format((int(interact.interact('Rh:%x' %(GPIOA + ODR))))),'\n'

print 'GPIOB output drive state is:'
print '{:032b}'.format((int(interact.interact('Rh:%x' %(GPIOB + ODR))))),'\n'

print 'GPIOC output drive state is:'
print '{:032b}'.format((int(interact.interact('Rh:%x' %(GPIOC + ODR))))),'\n'

print 'GPIOD output drive state is:'
print '{:032b}'.format((int(interact.interact('Rh:%x' %(GPIOD + ODR))))),'\n'

print 'GPIOE output drive state is:'
print '{:032b}'.format((int(interact.interact('Rh:%x' %(GPIOE + ODR))))),'\n'

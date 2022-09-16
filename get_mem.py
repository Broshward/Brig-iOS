#!/usr/bin/python2

usage = """
    usage: ./get_mem.py addr count
"""

import sys
addr=eval(sys.argv[1])
count=eval(sys.argv[2])

cinterpreter_path='/home/leha/STM32/Cinterpreter/'
sys.path.append(cinterpreter_path)

from cinterpreter import *

print 'Begin address: ', hex(addr) 
print 'Count of bytes: ', count

var={'type': 'char', 'addr': addr }
out=[]

for i in range(count):
    out.append(value(var))
    var['addr']+=1

print out
print out.count(0)

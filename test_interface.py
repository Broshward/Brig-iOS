#!/usr/bin/python2

test_addr=0x20000000+530 # ~200 byte free in RAM

import sys,os,time
CINTERPRETER_PATH='/home/leha/STM32/Cinterpreter/'
sys.path.append(CINTERPRETER_PATH)
from cinterpreter import *
from interact import *
interact=interact()

class bcolors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    WHITE = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def read_and_write(test_addr,cmd_opts,obj,pattern):
    global num
    if type(pattern)!=str:
        cmd = 'R%s%s:%x' %(cmd_opts,obj,test_addr)
        print ' Read command R%s%s:%x: %s' %(cmd_opts,obj,test_addr,interact.interact(cmd))
        cmd = 'W%s%s:%x=%x' %(cmd_opts,obj,test_addr,pattern)
        print ' Write command W%s%s:%x=%x' %(cmd_opts,obj,test_addr,pattern)
        interact.interact(cmd)
        cmd = 'R%s%s:%x' %(cmd_opts,obj,test_addr)
        answ=int(interact.interact(cmd),16)
        print ' Read command R%s%s:%x: %x' %(cmd_opts,obj,test_addr,answ)
    else:
        if ',' in pattern:
            count=pattern.count(',')+1
        else:
            count=0
        cmd = 'R%s%s:%x' %(cmd_opts,obj,test_addr)
        if count!=0:
            cmd+='*%x' %(count)
        print ' Read command %s: %s' %(cmd,interact.interact(cmd))
        cmd = 'W%s%s:%x=%s' %(cmd_opts,obj,test_addr,pattern)
        print ' Write command %s ' %(cmd)
        interact.interact(cmd)
        cmd = 'R%s%s:%x' %(cmd_opts,obj,test_addr)
        if count!=0:
            cmd+='*%x' %(count)
        answ=interact.interact(cmd)
        print ' Read command %s: %s' %(cmd,answ)
        if count and 'i' not in cmd_opts:
            pattern = ((pattern.rsplit(',',1)[1]+',')*count)[:-1]
    if answ==pattern:
        print bcolors.GREEN + '     Test is successful' + bcolors.WHITE
    else:
        print bcolors.RED + '     Test is wrong' + bcolors.WHITE
        print 'pattern = \'%s\'' %(pattern)
        print 'Answer  = \'%s\'' %(answ)
    num+=1

num=1
print '\nWRIPSISB test:\n'
print '%d. Read byte & Write byte test.' %(num)
read_and_write(test_addr,'','b',0xA5)

print '%d. Read halfword & Write halfword test.' %(num)
read_and_write(test_addr,'','h',0xA5c3)

print '%d. Read word & Write word test.' %(num)
read_and_write(test_addr,'','w',0xA5c3F081)

#print '%d. Read string & Write string test.' %(num)
read_and_write(test_addr,'i','s','Hello worlds!')

#interact.debug=1
print '%d. Increment Read byte & Increment Write byte test. (Array of byte)' %(num)
read_and_write(test_addr,'i','b','a5,c3,c3,f0,f0,f0,81,81,81,81')

print '%d. Increment Read half word & Increment Write test. (Array of half word(16bit))' %(num)
read_and_write(test_addr,'i','h','a5,c3,c3,f0,f0f0,8181,8181')

print '%d. Increment Read half word & Increment Write test. (Array of half word(16bit))' %(num)
read_and_write(test_addr,'i','w','a5c3c3f0,f0f0,81818181,aaaaaaaa,55555555')

print '%d. Read array of byte without increment memory. (For example from port or another peripherial)'
read_and_write(test_addr,'','b','a5,c3,c3,f0,f0,f0,81,81,81,81,aa,aa,aa,aa,55,55,55,55')

print '%d. Read string without increment memory. (For example read string from port)'
read_and_write(test_addr,'','s','Hello world!')


#!/usr/bin/python2

address_of_new_crontab=0x20000000+530 # ~247 byte free
usage= "usage: crontab [-w] [num]"

new_crontab=[]
new_crontab.append('*/15 * * * * * 40003000=AAAA' ) # Watchdog timer set to 0xAAAA
new_crontab.append('0 0 */5 * * * 40010C10=100')  # Relay power on
new_crontab.append('1 0 */5 * * * 40010C10=10' )# Left pump on
new_crontab.append('0 10 */5 * * * 40010C14=10') # Left pump off
new_crontab.append('1 10 */5 * * * 40010C10=4' )# Right pump on 
new_crontab.append('0 20 */5 * * * 40010C14=4' )# Right pump off
new_crontab.append('1 20 */5 * * * 40010C14=100' )# Relay power off

import sys,os,time
CINTERPRETER_PATH='/home/leha/STM32/Cinterpreter/'
sys.path.append(CINTERPRETER_PATH)
from cinterpreter import *
from interact import *

crontab_recieve_cmd='readelf -s main.elf |grep crontab |grep OBJECT'
crontab = os.popen(crontab_recieve_cmd).read().strip().split()
crontab_addr = int(crontab[1],16)
crontab_len = int(crontab[2])/4 # string number
interact=interact()

def string_read(addr):
    s=''
    while(1):
        temp_str = ''.join([chr(int(i)) for i in interact.interact("Rib:%x*64" %(addr)).split()])
        if chr(0) in temp_str:
            s+=temp_str.split(chr(0),1)[0]
            return s
        else:
            s+=temp_str
        if len(s)>1000:
            print 'String too long: "%s"' %(s)
            exit(-1)

if '-w' in sys.argv:
    write=True
    sys.argv.remove('-w')
else:
    write = False

if len(sys.argv)==1:
    crontab_items=range(crontab_len)
else: 
    num = sys.argv[1]
    crontab_items=[]
    for i in num.split(','):
        if '-' in i:
            beg,end=i.split('-')
            beg=eval(beg)
            end=eval(end)+1 
            end=end if end < crontab_len else crontab_len
            crontab_items+=(range(beg,end))
        else:
            crontab_items.append(eval(i) if eval(i) <= crontab_len else crontab_len)

if write:
    parse('char *crontab[32]')
    crontab_items.sort()
    crontab_addrs=[address_of_new_crontab]
    for i in new_crontab:
        crontab_addrs.append(crontab_addrs[-1]+len(i)+1)
    crontab_addrs.append(0)
    crontab_items=range(len(new_crontab))
    for i in crontab_items:
        parse('strcpy((char*)0x%x,"%s");' %(crontab_addrs[i],new_crontab[i]))
        print  len(new_crontab[i]), ' bytes writed'
        time.sleep(1)
    for i in crontab_items:
        print 'New value of crontab[%d] = %d' %(i,crontab_addrs[i])
        parse('crontab[%d]=%d;' %(i,crontab_addrs[i])) 
    parse('crontab[%d]=0;' %(i+1)) #Disable all alarms strings after last item
    for i in range(len(crontab_addrs)):
        print 'crontab[%d] = 0x%x' %(i,crontab_addrs[i])
        if crontab_addrs[i]:
            crontab_string = string_read(crontab_addrs[i])
#            import pdb; pdb.set_trace()
            print crontab_string
else:
    for i in crontab_items:
        crontab_addr_i = int(interact.interact("Rw:%x" %(crontab_addr+i*4)))
        print '&crontab[%d] = 0x%x' %(i,crontab_addr+i*4)
        print 'crontab[%d] = 0x%x' %(i,crontab_addr_i)
        if crontab_addr_i != 0:
            crontab_string = string_read(crontab_addr_i)
            print crontab_string
            print len(crontab_string),'\n'

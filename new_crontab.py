#!/usr/bin/python2
#coding:utf8

address_of_new_crontab=0x20000000+500 # ~247 byte free
new_crontab=[]
new_crontab.append('*/15 * * * * * 40003000=AAAA' ) # Watchdog timer set to 0xAAAA
new_crontab.append('0 0 */5 * * * 40010C10=100')  # Relay power on
new_crontab.append('1 0 */5 * * * 40010C10=10' )# Left pump on
new_crontab.append('0 13 */5 * * * 40010C14=10') # Left pump off
new_crontab.append('1 13 */5 * * * 40010C10=4' )# Right pump on 
new_crontab.append('0 26 */5 * * * 40010C14=4' )# Right pump off
new_crontab.append('1 26 */5 * * * 40010C14=100' )# Relay power off


import sys,os,time

CINTERPRETER_PATH='/home/leha/STM32/Cinterpreter/'

sys.path.append(CINTERPRETER_PATH)
from cinterpreter import *

#Watchdog replaced to crontab[0]
parse("char *crontab[32];crontab[0]=0x8001eb0") #At 0x8001eb0 True wotchdog reset string 
print 'This program is not freezing!'
timeout = 15
while timeout>0:#if timeout will be < 15 then next alarm of crontab[8] string will occur and reset microcontroller due to the '0' value of crontab[8]
    print '\rWaiting %d seconds' %(timeout),
    sys.stdout.flush()
    timeout -= 1
    time.sleep(1)
print
parse("crontab[1]=0;crontab[2]=0;crontab[3]=0;crontab[5]=0;crontab[6]=0;crontab[7]=0;crontab[8]=0;crontab[9]=0") # Disable all alarms strings except crontab[0] (Watchdog reset)
exit(0)

# New cron table write to microcontroller
i=0
#beg_time=time.time()
def timeout():
    print 'This program is not freezing!'
    timeout = 15
    while timeout>0:#if timeout will be < 15 then next alarm of crontab[8] string will occur and reset microcontroller due to the '0' value of crontab[8]
        print '\rWaiting %d seconds' %(timeout),
        sys.stdout.flush()
        timeout -= 1
        time.sleep(1)
    print

parse('char *crontab[32]')
while i<len(new_crontab):
    print 'Address of new_crontab[%d] = %d' %(i,address_of_new_crontab)
    parse('crontab[%d]=%d;strcpy(crontab[%d],"%s");' %(i,address_of_new_crontab,i,new_crontab[i]))
    address_of_new_crontab += len(new_crontab[i])+1
    print  len(new_crontab[i]), ' bytes writed'
    timeout()
    i+=1
parse("crontab[%s]=0" %(i)) # Disable all alarms strings except crontab[0] (Watchdog reset)





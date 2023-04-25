#!/usr/bin/python2

import sys,os
from interact import *

interact = interact()
RTC_CNTH = int(interact.interact('Rh:40002818'),16)
RTC_CNTL = int(interact.interact('Rh:4000281C'),16)
UTC = (RTC_CNTH<<16)+RTC_CNTL

if '-v' in sys.argv:
    print 'RTC->CNTH = ',hex(RTC_CNTH)
    print 'RTC->CNTL = ',hex(RTC_CNTL)
    print 'UTC timestamp: ', UTC
    print '\nCurrent time: '
os.system('date -d @%d' %(UTC))

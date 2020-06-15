#!/usr/bin/python2
#coding:utf8

import sys,os

addr = int(sys.argv[1])
count = int(sys.argv[2])

os.system("./debug_analyse.py RWb:422181b0=0 // NSS pin low        ")
os.system("./debug_analyse.py RWb:4000380C=3 // READ command       ")
os.system("./debug_analyse.py RWb:4000380C=hex((addr>>16)&0xFF)[2:]")
os.system("./debug_analyse.py RWb:4000380C=hex((addr>>8)&0xFF)[2:] ")
os.system("./debug_analyse.py RWb:4000380C=hex(addr&0xFF)[2:]      ")
os.system("./debug_analyse.py RWb:4000380C=0,FF                    ")
os.system("./debug_analyse.py RWb:422181b0=1 // NSS pin high       ")

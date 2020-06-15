#!/usr/bin/python2
#coding:utf8

beta=3950.0 #Default beta coefficient
R0=10e3  # Default nominal of current termistor
T0=298.15 # Default nominal measure termistor temperature

import sys,math
def Temp(R,beta,R0,T0=298.15):
#    R_inf = R0*math.e**(-beta/T0)
#    T = beta/(math.log(R/R_inf))
    R=float(R)
    R0=float(R0)
    beta=float(beta)
    T0=float(T0)
    T = 1/(1/T0+math.log(R/R0)/beta)
    return T

def B_calc(R1,R2,T1,T2):
    B = math.log(R1/R2)/(1/T1-1/T2)
    return B

def R_calc(T,beta,R0,T0=298.15):
    R = R0*math.e**(beta*(1/T-1/T0))
    return R
       
def calc(R1,T1,R2,T2):
    beta=B_calc(R1,R2,T1,T2)
    print 'Beta = ',beta
    print 'R25 = ',R_calc(25+273.15,beta,R1,T1)
    return

if __name__ == '__main__':
    usage = """usage: %s calc_function argumens
                calc_function may be the "T","B" or "R" """

    if sys.argv[1] == 'calc':
        R1,T1=sys.argv[2].split('/')
        R2,T2=sys.argv[3].split('/')
        R1=float(R1)
        R2=float(R2)
        T1=float(T1)
        T2=float(T2)
        calc(R1,T1,R2,T2)
    else:
        if len(sys.argv)>3:
            beta = float(sys.argv[3])
        if len(sys.argv)>2:
            R0 = float(sys.argv[2])
        R = float(sys.argv[1])
        T = Temp(R,beta,R0)
        print T-273.15

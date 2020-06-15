#!/usr/bin/python2
#coding:utf8

import sys,math
def Lux(R,R0,E0,T):
#    R_inf = R0*math.e**(-beta/T0)
#    T = beta/(math.log(R/R_inf))
    R=float(R)
    R0=float(R0)
    T=float(T)
    E0=float(E0)
    E = E0*math.e**(-math.log(R/R0)/T)
    return E

def lux_approx(p,R):
    return p[0]*e**(-log(R/p[1])/p[2])
def err_approx(p,R,E):
    return E-lux_approx(p,R)

def calc(R1,E1,R2,E2):
    print 'R = ',R1, '@ E=',E1,'R = ',R2, '@ E=',E2
    T = math.log(R1/R2)/math.log(E2/E1)
    print 'T = ',T
    return T

if __name__ == '__main__':
    usage = """usage: %s calc_function argumens
                calc_function may be the "T","B" or "R" """

    if sys.argv[1] == 'calc':
        R=[]
        E=[]
        for pair in sys.argv[2:]:
            r,e=pair.split('/')
            R.append(float(r))
            E.append(float(e))
        print R,E
        if len(R)==len(E)==2:
            calc(R[0],E[0],R[1],E[1])
        else:
            T=calc(R[0],E[0],R[1],E[1])
            from numpy import *
            p0 = array([T, R[0], E[0]])
            print p0  
            R=array(R)
            E=array(E)
            from scipy.optimize import leastsq  
            plsq = leastsq(err_approx, p0, args=(R, E))  
            print plsq[0]  
    else:
        R = sys.argv[1]
        R0 = sys.argv[2]
        E0 = sys.argv[3]
        T =  sys.argv[4]
        print Lux(R,R0,E0,T)

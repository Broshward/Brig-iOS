#!/usr/bin/python2
#coding:utf8

device="/dev/ttyUSB0"

cmds={  "R1":0b01000001,        # Example: R1:20000010   R1:20000010*16
        "R2":0b01000010,        # Example: 
        "R4":0b01000100,        # Example: 
        "W1":0b10000001,        # Example: W1:20000010=15   W1:20000010=22*16   W1:20000010=22,16,152,3,4
        "W2":0b10000010,        # Example: 
        "W4":0b10000100,        # Example: 
        "Rib":0b01100001,       # Example: 
        "Rih":0b01100010,       # Example: 
        "Riw":0b01100100,       # Example: 
        "Ris":0b01100000,       # Example: 
        "Rb":0b01000001,        # Example: 
        "Rh":0b01000010,        # Example: 
        "Rw":0b01000100,        # Example: 
        "Rs":0b01000000,        # Example: 
        "Wib":0b10100001,       # Example: 
        "Wih":0b10100010,       # Example: 
        "Wiw":0b10100100,       # Example: 
        "Wis":0b10100000,       # Example: 
        "Wb":0b10000001,        # Example: 
        "Wh":0b10000010,        # Example: 
        "Ww":0b10000100,        # Example: 
        "Ws":0b10000000,        # Example: 
        "WRib":0b11100001,       # Example: 
        "WRih":0b11100010,       # Example: 
        "WRiw":0b11100100,       # Example: 
        "WRis":0b11100000,       # Example: 
        "WRb":0b11000001,       # Example: 
        "WRh":0b11000010,       # Example: 
        "WRw":0b11000100,       # Example: 
        "WRs":0b11000000        # Example: 
    }

END = chr(0)
END_change = chr(0xFD)
END_change_change = chr(0xFC)

import serial,time

class interact:

    def __init__(self,raw=False):
        self.port = serial.Serial(device, baudrate=9600, timeout=0.2)
        self.debug=0
        self.FORCE_REMOVE_ECHO=0
        if raw:
            self.raw=True
        else:
            self.raw=False
        #return self
    
    def cmd_gen(self,cmd):
        if cmd=='0': # For reset command bus
            cmd='' 
            self.R = 0
        elif cmd[:3]=='ST:':
# ST is deprecated, needed for replace =>
#"CR42050090=1;4000281c=AAAA;40002818=AAAA;42050090=0;"
            time = int(cmd[3:])
            print time
            cmd = 'ST' + "".join([chr((time>>(i*8))&0xFF) for i in range(4)])
        elif cmd.split(':')[0] in cmds.keys(): 
            cmd = cmd.split(':',1)
            if '=' in cmd[1]:
                addr,data = cmd[1].split('=',1)
            else:
                addr,data = cmd[1],None
            cmd = cmds[cmd[0]]
            self.W = cmd & (1<<7) # Control command flags
            self.R = cmd & (1<<6) 
            self.I = cmd & (1<<5)
            self.P = cmd & (1<<4)
            self.data_width = cmd & 0b111
            if '*' in addr:
                addr,count = addr.split('*',1)
                count=int(count,16)
            else:
                count = None
            addr=int(addr,16)
            addr="".join([chr((addr>>(i*8))&0xFF) for i in range(4)])
            if (self.data_width)!=0: # data is not string
                if data:
                    if '*' in data:
                        data,count = data.split('*',1)
                        count=int(count)
                    temp=''
                    for j in data.split(','):
                        temp+=''.join([chr((int(j,16)>>(i*8))&0xFF) for i in range(self.data_width)])
                    data=temp        
                if count:
                    if count > 255:
                        print "Count of data cannot be more than 255!"
                        exit(1)
                    if data:
                        data*=count
                    else:
                        data=chr(int(count))
                    self.count=count
            cmd = chr(cmd) + addr + (data if data else '')                
        cmd = cmd.replace(END_change,END_change+END_change_change)
        cmd = cmd.replace(END,END_change+END_change)
        cmd += END
        if self.debug:
            print "Command:" 
            print ' ', [hex(ord(i)) for i in cmd]
            print ' ', [chr(ord(i)) for i in cmd ]
        if len(cmd) > 255:
            print "Count of packet cannot be more than 255!"
            exit(1)
        return cmd
        
    def transmit_cmd(self,cmd):
        if self.port.write(cmd) != len(cmd):
            print 'UART write error'
        self.cmd = cmd[:-1]

    def recieve_ans(self):
        tm=time.time()
        s=''
        data=''
        while(1):
            if time.time() - tm > 1:
                return #'Port timeout occured'
            s += self.port.readall()
            pack,s=s.split(END,1) if END in s else (None,s)
            if pack:
                if self.FORCE_REMOVE_ECHO==1:
                    if pack.find(self.cmd) == 0:
                        pack = pack[len(self.cmd):]
                if self.debug:
                    print "Answer:" 
                    print ' ', [hex(ord(i)) for i in pack] #Raw hex data
                    print ' ', [chr(ord(i)) for i in pack] #Raw char data
                pack = pack.replace(END_change+END_change,END)
                pack = pack.replace(END_change+END_change_change,END_change)
                if self.raw:
                    return pack
                if self.data_width==0: # String data
                    data=pack
                else:
                    res=0
                    for i in [pack[i:i+self.data_width] for i in range(0,len(pack),self.data_width)]:
                        data += '%x' %(sum([ord(i[j])<<8*j for j in range(self.data_width)]))+','
                    data=data[:-1]
                return data

    def interact(self,cmd):
        self.port.flush() 
        self.port.readall()
        cmd=self.cmd_gen(cmd)
        self.transmit_cmd(cmd)
        #if self.R:
        ans=self.recieve_ans()
        if ans == None:
            None
        return ans

if __name__ == '__main__':
    interact=interact()
    import sys
    print interact.interact(sys.argv[1])


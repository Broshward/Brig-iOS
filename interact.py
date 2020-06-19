#!/usr/bin/python2
#coding:utf8

cmds={  "Rb":0b01100001,
        "Rh":0b01100010,
        "Rw":0b01100100,
        "Rs":0b01100000,
        "Wb":0b10000001,
        "Wh":0b10000010,
        "Ww":0b10000100,
        "Ws":0b10100000,
        "RWb":0b11010001,
        "RWh":0b11000010,
        "RWw":0b11000100,
        "RWs":0b11100000
    }

END = chr(0)
END_change = chr(0xFD)
END_change_change = chr(0xFC)

import serial 

class interact:

    def __init__(self):
        self.port = serial.Serial('/dev/ttyS0', baudrate=9600, timeout=0.2)
        self.debug=0
        #return self
    
    def cmd_gen(self,cmd):
        if cmd=='0': # For reset command bus
            cmd='' 
        elif cmd[:3]=='ST:':
# ST is deprecated, needed for replace =>
#"CR42050090=1;4000281c=AAAA;40002818=AAAA;42050090=0;"
            time = int(cmd[3:])
            print time
            cmd = 'ST' + "".join([chr((time>>(i*8))&0xFF) for i in range(4)])
        elif cmd.split(':')[0] in cmds.keys(): 
            try:
                cmd,count = cmd.split('*')
            except:
                cmd,count = cmd,None
            cmd = cmd.split(':')
            try:
                addr,data = cmd[1].split('=')
            except:
                addr,data = cmd[1],None
            cmd = cmds[cmd[0]]
            self.W = cmd & (1<<7) # Control command flags
            self.R = cmd & (1<<6) 
            self.I = cmd & (1<<5)
            self.P = cmd & (1<<4)
            self.data_width = cmd & 0b111
            addr=int(addr,16)
            addr="".join([chr((addr>>(i*8))&0xFF) for i in range(4)])
            if data:
                temp=''
                for j in data.split(','):
                    if (self.data_width)!=0:
                        temp+=''.join([chr((int(j,16)>>(i*8))&0xFF) for i in range(self.data_width)])
                data=temp        
                if count:
                    data*=int(count,16)
            else:
                if count:
                    data=chr(int(count,16))
                else:
                    data=''
            cmd = chr(cmd) + addr + data
                
        cmd = cmd.replace(END_change,END_change+END_change_change)
        cmd = cmd.replace(END,END_change+END_change)
        cmd += END
        if self.debug:
            print "Command:" 
            print ' ', [hex(ord(i)) for i in cmd]
            print ' ', [chr(ord(i)) for i in cmd ]
        return cmd
        
    def transmit_cmd(self,cmd):
        self.port.write(cmd)
    def recieve_ans(self):
        s=''
        data=''
        while(1):
            s += self.port.readall()
            pack,s=s.split(END,1) if END in s else (None,s)
            if pack:
                pack = pack.replace(END_change+END_change,END)
                pack = pack.replace(END_change+END_change_change,END_change)
                if self.debug:
                    print "Answer:" 
                    print ' ', [hex(ord(i)) for i in pack] #Raw hex data
                    print ' ', [chr(ord(i)) for i in pack] #Raw char data
                if self.data_width==0: # String data
                    data=pack
                else:
                    res=0
                    for i in [pack[i:i+self.data_width] for i in range(0,len(pack),self.data_width)]:
                        data += str(sum([ord(i[j])<<8*j for j in range(self.data_width)]))+' '
                return data

    def interact(self,cmd):
        cmd=self.cmd_gen(cmd)
        self.transmit_cmd(cmd)
        if self.R:
            return self.recieve_ans()

#if __name__ == '__main__':
#    interact=interact()
#    interact.interact()


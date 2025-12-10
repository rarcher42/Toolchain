import random
import time
import sys
import serial
''' 
01 READBYTES:       01 SAL SAH SAB BCL BCH NLO N    (read N+1 bytes 1-256 for N=0..255)
    returns n+1 bytes
    
02 WRITEBYTES       02 SAL SAH SAB B0 B1 B2..... BN  (use CMD_IX to find last byte to write) - Max 256 bytes
    returns ACK
    
03 JUMPTO           03 SAL SAH SAB  
    returns ACK; anything the target program outputs to FIFO
    
04 SETBKPT          04 SAL SAH SAB
    returns replaced byte.  Store this away and restore it with a WRITEBYTES

05 GETCONTEXT       05
    returns         [E-flag][Flags][B][A][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
'''

SER_PORT="COM6"

class Frame:
    ''' Encapsulate a frame '''
    def __init__(self):
        self.SOF = 0x02
        self.ESC = 0x10
        self.EOF = 0x03
        self.SIZE_CMD_BUF = 512
    ''' 
        Encapsulate the payload to exclude SOF & EOF with ESC encoding 
        A wire frame begins with SOF and ends with EOF for ease of decoding 
    '''
    def wire_encode(self, rawpay):
        outbytes = bytes()
        outbytes += self.SOF.to_bytes(1)  # Start the wire frame
        for b in rawpay:
            if b == self.SOF: 
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x11.to_bytes(1)
            elif b == self.ESC:
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x12.to_bytes(1)
            elif b == self.EOF:
                outbytes += self.ESC.to_bytes(1)
                outbytes += 0x13.to_bytes(1) 
            else:
                outbytes += b.to_bytes(1)
        outbytes += self.EOF.to_bytes(1)  # End the wire frame
        return outbytes  

    ''' 
        Reverse payload encapsulation to yield original payload.  This
        involves discarding the intial SOF and terminal EOF, and 
        reversing all escaped data inside the payload to restore their values.
    '''
    def wire_decode(self, wire):
        state = 0
        inp = 0
        cmd_ptr = 0

        while True:
            if (cmd_ptr >= self.SIZE_CMD_BUF):
                print("OVERFLOW!!!!!!!!!!!!!!!!!!!!")
                state = 0
                exit(-1)
                continue

            # print("State = ", state)
            # INIT state
            if state == 0:
                state = 1
                cmd_ptr = 0
                outbytes = bytes()
                inp = 0
                error = False
                continue

            # AWAIT state
            elif state == 1:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    state = 2
                continue
                
            # COLLECT state 
            elif state == 2:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    cmd_ptr = 0
                    continue
                elif b == self.EOF:
                    state = 4
                    continue
                elif b == self.ESC:
                    state = 3
                    continue
                else:
                    outbytes += b.to_bytes(1)
                    cmd_ptr += 1
                    continue    
    
            # TRANSLATE state
            elif state == 3:
                b = wire[inp]
                inp += 1
                if b == self.SOF:
                    cmd_ptr = 0
                    outbytes = bytes()
                    continue
                elif b == self.EOF:
                    state = 0
                    continue
                elif b == 0x11:
                    outbytes += self.SOF.to_bytes(1)
                    cmd_ptr += 1
                    state = 2 
                    continue
                elif b == 0x12:
                    outbytes += self.ESC.to_bytes(1)
                    cmd_ptr += 1
                    state = 2
                    continue
                elif b == 0x13:
                    outbytes += self.EOF.to_bytes(1)
                    cmd_ptr += 1
                    state = 2
                    continue
                continue

            # PROCESS state
            elif state == 4:
                state = 0
                return outbytes
            # INVALID state
            else:
                print("INVALID STATE")
                sys.exit(-2)
                state = 0
                continue
               

class FIFO:
    def __init__(self, port='COM17', br=921600, parity=serial.PARITY_NONE, size=serial.EIGHTBITS,
                 stops=serial.STOPBITS_ONE, to=3.0):
        try:
            self.ser = serial.Serial(port=port, baudrate=br, parity=parity,
                            bytesize=size, stopbits=stops, timeout=to)
            self.open=True
        except:
            print("Error opening port")
            self.open = False

    def write(self, s):
        self.ser.write(s)
        return
        
    def read(self):
        outb = b''
        outb = self.ser.read_until(b'\x03')
        return outb
        
    def close(self):
        self.ser.close()
        return

# Create a bidirectional pipeline to the 65c816 CPU
class CPU_Pipe:
    def __init__ (self, port, rate):
        self.SERIAL_PORT = port
        self.fifo = FIFO(port, rate, serial.PARITY_NONE, serial.EIGHTBITS, serial.STOPBITS_ONE, 0.1)
        self.v = Frame()
        self.bp_list = list()
        self.fifo.read()     # Ditch any power on messages or noise bursts
    
    def cmd_dialog(self, cmd):
        outf = self.v.wire_encode(cmd)
        self.fifo.write(outf)
        reinf = self.fifo.read()
        return self.v.wire_decode(reinf)
        
    def read_mem(self, sa_24, nbytes):
        nbytes -= 1             # 0-255 --> 1-256
        assert(nbytes < 256)
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        nbl = (nbytes & 0xFF).to_bytes(1, 'little')
        cmd = b'\x01'+sal+sah+sab+nbl
        return self.cmd_dialog(cmd)
        
    def write_mem(self, sa_24, data):
        assert(len(data) < 257)
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x02' + sal + sah + sab 
        for b in data:
            cmd += b.to_bytes(1, "little")
        return  self.cmd_dialog(cmd)

    def jump_long(self, sa_24):
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x03' + sal + sah + sab
        return self.cmd_dialog(cmd)
        
    def set_breakpoint(self, sa_24):
        sab = ((sa_24 >> 16) & 0xFF).to_bytes(1, 'little')
        sah = ((sa_24 >> 8) & 0xFF).to_bytes(1, 'little')
        sal = ((sa_24) & 0xFF).to_bytes(1, 'little')
        cmd = b'\x04' + sal + sah + sab
        value = self.cmd_dialog(cmd)       
        self.bp_list.append((sa_24, value))
        return value
       
    def replace_breaks(self):
        for bp in self.bp_list:
            print(bp)
            adr24 = bp[0]
            val8 = int.from_bytes(bp[1], 'little')
            print("Writing %06X: %02X" % (adr24, val8))
            self.write_mem(adr24, bp[1])
            self.bp_list.remove(bp)
        print("Exit bp_list=", self.bp_list)
    
    def get_registers(self):
        cmd = b'\x05'
        return self.cmd_dialog(cmd)
        
        
    def decode_flags(self, f, emode):
        s = "FLAGS(%02X)=" % f
        if f & 0x80:
            s+= "N"
        else:
            s+= "-"
            
        if f & 0x40:
            s += "V"
        else:
            s += "-"
            
        if f & 0x20:
            if emode == 0:
                s += "M"
            else:
                s += '1'
        else:
            s += "-"
        
        if f & 0x10:
            if emode == 0:
                s += "X"
            else:
                s += 'B'       # Break flag
        else:
            s += "-"
            
        if f & 0x8:
            s += "D"
        else:
            s += "-"
            
        if f & 0x4:
            s += "I"
        else:
            s += "-"
            
        if f & 0x2:
            s += "Z"
        else:
            s += "-"
        if f & 0x1:
            s += "C"
        else:
            s += "-"
        return s
        
    def print_registers(self):
    # Refactor this, eventually
        # [E-flag][Flags][B][A][XL][XH][YL][YH][SPL][SPH][DPRL][DPRH][PCL][PCH][PBR][DBR]
        # 0:  E-Flag:  00= native FF=emulation
        # 1:  Flags      
        # 2:  A
        # 3:  B
        # 4:  XL, XH
        # 6:  YL, YH
        # 8:  SPL, SPH
        # 10: DPRL, DPRH
        # 12: PCL, PCH
        # 14: PBR
        # 15: DBR
        
        regs = self.get_registers()
        if regs[0] < 4:
            m_flag = (regs[0] & 0b10)
            x_flag = (regs[0] & 0b01)
            # Print native mode registers
            s = "E(XM=%02X)=0 " % regs[0]
            print(s, end="")
            if m_flag:
                s = "A=%02X," % int.from_bytes(regs[2:3], "little")
                print(s, end="")
                s = "B=%02X," % int.from_bytes(regs[3:4], "little")
                print(s, end="")
            else:
                s = "C=%04X," % int.from_bytes(regs[2:4], "little")
                print(s, end="")
             # Print X and Y
            if x_flag:
                s = "X=%02X," % int.from_bytes(regs[4:5], "little")
                print(s, end="")
                s = "Y=%02X," % int.from_bytes(regs[6:7], "little")
                print(s, end="")
            else:
                s = "X=%04X," % int.from_bytes(regs[4:6], "little")
                print(s, end="")
                s = "Y=%04X," % int.from_bytes(regs[6:8], "little")
                print(s, end="")
            # Print SP
            s = "SP=%04X," % int.from_bytes(regs[8:10], "little")
            print(s, end="")
            s = "DPR=%04X," % int.from_bytes(regs[10:12], "little")
            print(s, end="")
            s = "PC=%04X," % int.from_bytes(regs[12:14], "little")
            print(s, end="")
            s = "PBR=%02X," % int.from_bytes(regs[14:15], "little")
            print(s, end="")
            s = "DBR=%02X," % int.from_bytes(regs[15:16], "little")
            print(s, end="")
            print(self.decode_flags(regs[1], False))
        else:
            # Print emulation mode registers
            print("E=1: ", end="")
            s = "A=%02X," % int.from_bytes(regs[2:3], "little")
            print(s, end="")
            s = "B=%02X," % int.from_bytes(regs[3:4], "little")
            print(s, end="")
            s = "X=%02X," % int.from_bytes(regs[4:5], "little")
            print(s, end="")
            s = "Y=%02X," % int.from_bytes(regs[6:7], "little")
            print(s, end="")
            s = "SP=%04X," % (int.from_bytes(regs[8:9], "little") | 0x0100)
            print(s, end="")
            s = "PC=%04X," % int.from_bytes(regs[12:14], "little")
            print(s, end="")
            print(self.decode_flags(regs[1], True))
            
    def str_to_bytes(self, s):
        outb = b''
        for i in range(0, len(s), 2):
            x = int(s[i:i+2], 16)
            outb += x.to_bytes(1, 'little')
        return outb
        
    def send_srec(self, srec_fn):
        fh = open(srec_fn, "r")
        lines = fh.readlines()
        for li in lines:
            li = li.strip()
            rectype = li[0:2]
            if rectype == 'S0':
                # print("Rec(S0) - DISCARD")
                pass
            elif rectype == 'S1':
                ads = li[4:8]
                nstr = li[2:4]
                n = int(nstr, 16) - 3   # Subtract out checksum and 2 address bytes
                # print("Rec(S1) - 16 bits @$", ads) 
                addr = int(ads, 16)
                #print("ADDR = $%04X" % addr)
                datastr = li[8:-2]
                #print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                self.write_mem(addr, barry)
            elif rectype == 'S2':
                ads = li[4:10]
                nstr = li[2:4]
                n = int(nstr, 16) - 4   # Subtract checksum and 3 address bytes
                # print("Rec(S2) - 24 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%06X" % addr)
                datastr = li[10:-2]
                # print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                self.write_mem(addr, barry)
            elif rectype == 'S3':
                ads = li[4:12]
                nstr = li[2:4]
                n = int(nstr, 16) - 5   # Subtract checksum and 4 address bytes
                # print("Rec(S3) - 32 bits @$", ads)
                addr = int(ads, 16)
                # print("n = ", n)
                # print("ADDR = $%08X" % addr)
                datastr = li[12:-2]
                #print(len(datastr), datastr)
                barry = self.str_to_bytes(datastr)
                self.write_mem(addr, barry)
            elif rectype == 'S5':
                pass
            elif rectype == 'S7':
                ads = li[4:12]
                # print("Rec(S7) - 32 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%08X" % addr)
            elif rectype == 'S8':
                ads = li[4:10]
                # print("Rec(S8 - 24 bits @$", ads)
                addr = int(ads, 16)
                # print("ADDR = $%06X" % addr)
            elif rectype == 'S9':
                ads = li[4:8]
                #print("Rec(S9) - 16 bits @$", ads)
                addr = int(ads, 16)
                #print("ADDR = $%04X" % addr)
            else:
                print("Unknown record type %s" % rectype)
        fh.close()
        return
        
        
def dump_hex(sa_24, data):
    count = 0
    for b in data:
        if (count == 0):
            s = "\n%06X:" % sa_24
            print(s, end="")
        s = " %02X" % b
        print(s, end="")
        count += 1
        sa_24 += 1
        if count > 15:
            count = 0
    return

def test_page_read_write(address):
    # Create a page of test data to write: Fill up a page with FF FE FD FC ... 01 00
    outb = b''
    for b in range(255, -1, -1):
        outb += b.to_bytes(1, "little")
    # OPEN a bidirectional CPU pipeline
    pipe = CPU_Pipe(SER_PORT, 921600)
   
    print("Writing data directly to memory")
    resp = pipe.write_mem(address, outb)
    print("Write returns: ", resp)
    
    print("Reading back the data")
    mem = pipe.read_mem(address, 256)
    dump_hex(address, mem)
    return 
 
def test_go(address):
    pipe = CPU_Pipe(SER_PORT, 921600)
    reply = pipe.jump_long(address)
    print(reply)
    return


if __name__ == "__main__":
    pipe = CPU_Pipe(SER_PORT, 921600)
    srec_fn = "rammon.hex"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    bp_replaced_val = pipe.set_breakpoint(0x200d)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    pipe.replace_breaks()
    exit(0)
   
    start_t = time.time()
    for sa in range(0, 0x010000, 256):
        mem = pipe.read_mem(sa, 256)
        dump_hex(sa, mem)
    end_t = time.time()
    print("\n\nDumped 64K byte in %10.1f seconds" % (end_t - start_t))
    print("Rate = %10.1f bytes/second: " % (65536.0 / (end_t - start_t)))
    
    exit(1)
    while True:
        n = random.randrange(1, 256)
        inf = b'E'
        for i in range(n):
            inf += random.randrange(0, 255).to_bytes(1)
        outf = v.wire_encode(inf)
        # print("Writing ", outf)
        fifo.write(outf)
        #time.sleep(0.1)
        reinf = fifo.read()
        #print("Reading ", reinf)
        rereinf = v.wire_decode(reinf)
        print(len(rereinf), "\t", rereinf == inf)
        if rereinf != inf:
            print("ERROR: raw string:")
            print(inf)
            print("ERROR: wire string:")
            print(outf)
            break
    
    

   
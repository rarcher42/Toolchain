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

SER_PORT="COM4"
opcode_table = (
# 00
("BRK", 2, 2, 2, 2),             # BRK implied
("ORA", 2, 2, 2, 2),             # ORA (dp, X)
("COP", 2, 2, 2, 2),             # COP
("ORA", 2, 2, 2, 2),             # ORA dp
("TSB", 2, 2, 2, 2),             # TSB dp
("ORA", 2, 2, 2, 2),             # ORA dp
("ASL", 2, 2, 2, 2),             # ASL dp
("ORA", 2, 2, 2, 2),             # ORA (dp)
# 08
("PHP", 1, 1, 1, 1),             # PHP
("ORA", 2, 3, 3, 2),             # ORA #imm (M-dependent)
("ASL", 1, 1, 1, 1),             # ASL A
("PHD", 1, 1, 1, 1),             # PHD
("TSB", 3, 3, 3, 3),             # TSB abs
("ORA", 3, 3, 3, 3),             # ORA abs
("ASL", 3, 3, 3, 3),             # ASL abs
("ORA", 4, 4, 4, 4),             # ORA long
# 10
("BPL", 2, 2, 2, 2),             # BPL
("ORA", 2, 2, 2, 2),             # ORA (dp),Y
("ORA", 2, 2, 2, 2),             # ORA (dp)
("ORA", 2, 2, 2, 2),             # ORA (dp,S),Y
("TRB", 2, 2, 2, 2),             # TRB dp
("ORA", 3, 3, 3, 3),             # ORA dp,X
("ASL", 2, 2, 2, 2),             # ASL dp,X
("ORA", 4, 4, 4, 4),             # ORA (dp),Y
# 18
("CLC", 1, 1, 1, 1),             # CLC
("ORA", 3, 4, 4, 4),             # ORA abs,Y
("INC", 1, 1, 1, 1),             # INC A
("TCS", 1, 1, 1, 1),             # TCS
("TRB", 3, 3, 3, 3),             # TRB abs
("ORA", 4, 4, 4, 4),             # ORA abs,X
("ASL", 3, 3, 3, 3),             # ASL abs,X
("ORA", 5, 5, 5, 5),             # ORA long,X
# 20
("JSR", 3, 3, 3, 3),             # JSR abs
("AND", 2, 2, 2, 2),             # AND (dp,X)
("JSL", 4, 4, 4, 4),             # JSL long
("AND", 2, 2, 2, 2),             # AND dp
("BIT", 2, 2, 2, 2),             # BIT dp
("AND", 2, 2, 2, 2),             # AND dp
("ROL", 2, 2, 2, 2),             # ROL dp
("AND", 2, 2, 2, 2),             # AND (dp)
# 28
("PLP", 1, 1, 1, 1),             # PLP
("AND", 2, 3, 3, 2),             # AND #imm (M-dependent)
("ROL", 1, 1, 1, 1),             # ROL A
("PLD", 1, 1, 1, 1),             # PLD
("BIT", 3, 3, 3, 3),             # BIT abs
("AND", 3, 3, 3, 3),             # AND abs
("ROL", 3, 3, 3, 3),             # ROL abs
("AND", 4, 4, 4, 4),             # AND long# 30
("BMI", 2, 2, 2, 2),             # BMI
("AND", 2, 2, 2, 2),             # AND (dp),Y
("AND", 2, 2, 2, 2),             # AND (dp)
("AND", 2, 2, 2, 2),             # AND (dp,S),Y
("BIT", 2, 2, 2, 2),             # BIT dp,X
("AND", 3, 3, 3, 3),             # AND dp,X
("ROL", 2, 2, 2, 2),             # ROL dp,X
("AND", 4, 4, 4, 4),             # AND (dp),Y
# 38
("SEC", 1, 1, 1, 1),             # SEC
("AND", 3, 4, 4, 4),             # AND abs,Y
("DEC", 1, 1, 1, 1),             # DEC A
("TSC", 1, 1, 1, 1),             # TSC
("BIT", 3, 3, 3, 3),             # BIT abs
("AND", 4, 4, 4, 4),             # AND abs,X
("ROL", 3, 3, 3, 3),             # ROL abs,X
("AND", 5, 5, 5, 5),             # AND long,X
# 40
("RTI", 1, 1, 1, 1),             # RTI
("EOR", 2, 2, 2, 2),             # EOR (dp,X)
("WDM", 2, 2, 2, 2),             # WDM
("EOR", 2, 2, 2, 2),             # EOR dp
("MVN", 3, 3, 3, 3),             # MVN src,dst
("EOR", 2, 2, 2, 2),             # EOR dp
("LSR", 2, 2, 2, 2),             # LSR dp
("EOR", 2, 2, 2, 2),             # EOR (dp)
# 48
("PHA", 1, 1, 1, 1),             # PHA
("EOR", 2, 3, 3, 2),             # EOR #imm (M-dependent)
("LSR", 1, 1, 1, 1),             # LSR A
("PHK", 1, 1, 1, 1),             # PHK
("JMP", 3, 3, 3, 3),             # JMP abs
("EOR", 3, 3, 3, 3),             # EOR abs
("LSR", 3, 3, 3, 3),             # LSR abs
("EOR", 4, 4, 4, 4),             # EOR long
# 50
("BVC", 2, 2, 2, 2),             # BVC
("EOR", 2, 2, 2, 2),             # EOR (dp),Y
("EOR", 2, 2, 2, 2),             # EOR (dp)
("EOR", 2, 2, 2, 2),             # EOR (dp,S),Y
("MVN", 3, 3, 3, 3),             # MVN (mirrors 44; same encoding rules)
("EOR", 3, 3, 3, 3),             # EOR dp,X
("LSR", 2, 2, 2, 2),             # LSR dp,X
("EOR", 4, 4, 4, 4),             # EOR (dp),Y
# 58
("CLI", 1, 1, 1, 1),             # CLI
("EOR", 3, 4, 4, 4),             # EOR abs,Y
("PHY", 1, 1, 1, 1),             # PHY
("TCD", 1, 1, 1, 1),             # TCD
("JMP", 3, 3, 3, 3),             # JMP (abs)
("EOR", 4, 4, 4, 4),             # EOR abs,X
("LSR", 3, 3, 3, 3),             # LSR abs,X
("EOR", 5, 5, 5, 5),             # EOR long,X
# 60
("RTS", 1, 1, 1, 1),             # RTS
("ADC", 2, 2, 2, 2),             # ADC (dp,X)
("PER", 3, 3, 3, 3),             # PER relative long
("ADC", 2, 2, 2, 2),             # ADC dp
("STZ", 2, 2, 2, 2),             # STZ dp
("ADC", 2, 2, 2, 2),             # ADC dp
("ROR", 2, 2, 2, 2),             # ROR dp
("ADC", 2, 2, 2, 2),             # ADC (dp)
# 68
("PLA", 1, 1, 1, 1),             # PLA
("ADC", 2, 3, 3, 2),             # ADC #imm (M-dependent)
("ROR", 1, 1, 1, 1),             # ROR A
("RTL", 1, 1, 1, 1),             # RTL
("JMP", 3, 3, 3, 3),             # JMP abs long indirect: JMP (abs)
("ADC", 3, 3, 3, 3),             # ADC abs
("ROR", 3, 3, 3, 3),             # ROR abs
("ADC", 4, 4, 4, 4),             # ADC long
# 70
("BVS", 2, 2, 2, 2),             # BVS
("ADC", 2, 2, 2, 2),             # ADC (dp),Y
("ADC", 2, 2, 2, 2),             # ADC (dp)
("ADC", 2, 2, 2, 2),             # ADC (dp,S),Y
("STZ", 2, 2, 2, 2),             # STZ dp,X
("ADC", 3, 3, 3, 3),             # ADC dp,X
("ROR", 2, 2, 2, 2),             # ROR dp,X
("ADC", 4, 4, 4, 4),             # ADC (dp),Y
# 78
("SEI", 1, 1, 1, 1),             # SEI
("ADC", 3, 4, 4, 4),             # ADC abs,Y
("PLY", 1, 1, 1, 1),             # PLY
("TDC", 1, 1, 1, 1),             # TDC
("JMP", 3, 3, 3, 3),             # JMP (abs,X)
("ADC", 4, 4, 4, 4),             # ADC abs,X
("ROR", 3, 3, 3, 3),             # ROR abs,X
("ADC", 5, 5, 5, 5),             # ADC long,X
# 80
("BRA", 2, 2, 2, 2),             # BRA
("STA", 2, 2, 2, 2),             # STA (dp,X)
("BRL", 3, 3, 3, 3),             # BRL
("STA", 2, 2, 2, 2),             # STA dp
("STY", 2, 2, 2, 2),             # STY dp
("STA", 2, 2, 2, 2),             # STA dp
("STX", 2, 2, 2, 2),             # STX dp
("STA", 2, 2, 2, 2),             # STA (dp)
# 88
("DEY", 1, 1, 1, 1),             # DEY
("BIT", 2, 3, 3, 2),             # BIT #imm (M-dependent)
("TXA", 1, 1, 1, 1),             # TXA
("PHB", 1, 1, 1, 1),             # PHB
("STY", 3, 3, 3, 3),             # STY abs
("STA", 3, 3, 3, 3),             # STA abs
("STX", 3, 3, 3, 3),             # STX abs
("STA", 4, 4, 4, 4),             # STA long
# 90
("BCC", 2, 2, 2, 2),             # BCC
("STA", 2, 2, 2, 2),             # STA (dp),Y
("STA", 2, 2, 2, 2),             # STA (dp)
("STA", 2, 2, 2, 2),             # STA (dp,S),Y
("STY", 2, 2, 2, 2),             # STY dp,X
("STA", 3, 3, 3, 3),             # STA dp,X
("STX", 2, 2, 2, 2),             # STX dp,Y
("STA", 4, 4, 4, 4),             # STA (dp),Y
# 98
("TYA", 1, 1, 1, 1),             # TYA
("STA", 3, 4, 4, 4),             # STA abs,Y
("TXS", 1, 1, 1, 1),             # TXS
("TXY", 1, 1, 1, 1),             # TXY
("STZ", 3, 3, 3, 3),             # STZ abs
("STA", 4, 4, 4, 4),             # STA abs,X
("STZ", 3, 3, 3, 3),             # STZ abs,X
("STA", 5, 5, 5, 5),             # STA long,X
# A0
("LDY", 2, 3, 2, 3),             # LDY #imm (X-dependent)
("LDA", 2, 3, 3, 2),             # LDA #imm (M-dependent)
("LDX", 2, 3, 2, 3),             # LDX #imm (X-dependent)
("LDA", 2, 2, 2, 2),             # LDA dp
("LDY", 2, 2, 2, 2),             # LDY dp
("LDA", 2, 2, 2, 2),             # LDA dp
("LDX", 2, 2, 2, 2),             # LDX dp
("LDA", 2, 2, 2, 2),             # LDA (dp)
# A8
("TAY", 1, 1, 1, 1),             # TAY
("LDA", 2, 3, 3, 2),             # LDA #imm (duplicate immediate forms only differ by mnemonic)
("TAX", 1, 1, 1, 1),             # TAX
("PLB", 1, 1, 1, 1),             # PLB
("LDY", 3, 3, 3, 3),             # LDY abs
("LDA", 3, 3, 3, 3),             # LDA abs
("LDX", 3, 3, 3, 3),             # LDX abs
("LDA", 4, 4, 4, 4),             # LDA long
# B0
("BCS", 2, 2, 2, 2),             # BCS
("LDA", 2, 2, 2, 2),             # LDA (dp),Y
("LDA", 2, 2, 2, 2),             # LDA (dp)
("LDA", 2, 2, 2, 2),             # LDA (dp,S),Y
("LDY", 2, 2, 2, 2),             # LDY dp,X
("LDA", 3, 3, 3, 3),             # LDA dp,X
("LDX", 2, 2, 2, 2),             # LDX dp,Y
("LDA", 4, 4, 4, 4),             # LDA (dp),Y
# B8
("CLV", 1, 1, 1, 1),             # CLV
("LDA", 3, 4, 4, 4),             # LDA abs,Y
("TSX", 1, 1, 1, 1),             # TSX
("TYX", 1, 1, 1, 1),             # TYX
("LDY", 3, 3, 3, 3),             # LDY abs
("LDA", 4, 4, 4, 4),             # LDA abs,X
("LDX", 3, 3, 3, 3),             # LDX abs,Y
("LDA", 5, 5, 5, 5),             # LDA long,X
# C0
("CPY", 2, 3, 2, 3),             # CPY #imm (X-dependent)
("CMP", 2, 2, 2, 2),             # CMP (dp,X)
("REP", 2, 2, 2, 2),             # REP #imm (always 2-byte immediate)
("CMP", 2, 2, 2, 2),             # CMP dp
("CPY", 2, 2, 2, 2),             # CPY dp
("CMP", 2, 2, 2, 2),             # CMP dp
("DEC", 2, 2, 2, 2),             # DEC dp
("CMP", 2, 2, 2, 2),             # CMP (dp)
# C8
("INY", 1, 1, 1, 1),             # INY
("CMP", 2, 3, 3, 2),             # CMP #imm (M-dependent)
("DEX", 1, 1, 1, 1),             # DEX
("WAI", 1, 1, 1, 1),             # WAI
("CPY", 3, 3, 3, 3),             # CPY abs
("CMP", 3, 3, 3, 3),             # CMP abs
("DEC", 3, 3, 3, 3),             # DEC abs
("CMP", 4, 4, 4, 4),             # CMP long
# D0
("BNE", 2, 2, 2, 2),             # BNE
("CMP", 2, 2, 2, 2),             # CMP (dp),Y
("CMP", 2, 2, 2, 2),             # CMP (dp)
("CMP", 2, 2, 2, 2),             # CMP (dp,S),Y
("PEI", 2, 2, 2, 2),             # PEI dp
("CMP", 3, 3, 3, 3),             # CMP dp,X
("DEC", 2, 2, 2, 2),             # DEC dp,X
("CMP", 4, 4, 4, 4),             # CMP (dp),Y
# D8
("CLD", 1, 1, 1, 1),             # CLD
("CMP", 3, 4, 4, 4),             # CMP abs,Y
("PHX", 1, 1, 1, 1),             # PHX
("STP", 1, 1, 1, 1),             # STP
("JML", 3, 3, 3, 3),             # JML abs long indirect
("CMP", 4, 4, 4, 4),             # CMP abs,X
("DEC", 3, 3, 3, 3),             # DEC abs,X
("CMP", 5, 5, 5, 5),             # CMP long,X
# E0
("CPX", 2, 3, 2, 3),             # CPX #imm (X-dependent)
("SBC", 2, 2, 2, 2),             # SBC (dp,X)
("SEP", 2, 2, 2, 2),             # SEP #imm (always 2 bytes)
("SBC", 2, 2, 2, 2),             # SBC dp
("CPX", 2, 2, 2, 2),             # CPX dp
("SBC", 2, 2, 2, 2),             # SBC dp
("INC", 2, 2, 2, 2),             # INC dp
("SBC", 2, 2, 2, 2),             # SBC (dp)
# E8
("INX", 1, 1, 1, 1),             # INX
("SBC", 2, 3, 3, 2),             # SBC #imm (M-dependent)
("NOP", 1, 1, 1, 1),             # NOP
("XBA", 1, 1, 1, 1),             # XBA
("INC", 3, 3, 3, 3),             # INC abs
("SBC", 3, 3, 3, 3),             # SBC abs
("INC", 3, 3, 3, 3),             # INC abs
("SBC", 4, 4, 4, 4),             # SBC long
# F0
("BEQ", 2, 2, 2, 2),             # BEQ
("SBC", 2, 2, 2, 2),             # SBC (dp),Y
("SBC", 2, 2, 2, 2),             # SBC (dp)
("SBC", 2, 2, 2, 2),             # SBC (dp,S),Y
("PEA", 3, 3, 3, 3),             # PEA abs
("SBC", 3, 3, 3, 3),             # SBC dp,X
("INC", 2, 2, 2, 2),             # INC dp,X
("SBC", 4, 4, 4, 4),             # SBC (dp),Y
# F8
("SED", 1, 1, 1, 1),             # SED
("SBC", 3, 4, 4, 4),             # SBC abs,Y
("PLX", 1, 1, 1, 1),             # PLX
("XCE", 1, 1, 1, 1),             # XCE
("JSR", 3, 3, 3, 3),             # JSR (abs,X)
("SBC", 4, 4, 4, 4),             # SBC abs,X
("INC", 3, 3, 3, 3),             # INC abs,X
("SBC", 5, 5, 5, 5)              # SBC long,X
)

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
        if (emode):
            s += " [E]"
        else:
            s += " [N]"
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
                dump_hex(addr, barry)
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
                dump_hex(addr, barry)
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
                dump_hex(addr, barry)
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
    print(len(opcode_table))
    pipe = CPU_Pipe(SER_PORT, 921600)
    srec_fn = "rammon.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    pipe.replace_breaks()
    exit(0)
   
   
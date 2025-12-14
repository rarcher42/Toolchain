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

IMM=0
opcode_table = (
# 00
# {"MNEMONIC", M0X0-bytes, M0X1 bytes, M1X0 byres, M1X1 bytes, E-bytes, 65c02 bytes, 6502 bytes}
("BRK", IMM,  1,1,1,1,1,1),              # BRK implied, actually 2 bytes but debugger makes it 1 byte!
("ORA", IMM,  2,2,2,2,2,2),             # ORA (dp, X)
("COP", IMM,  2,2,2,2,0,0),             # COP
("ORA", IMM,  2,2,2,2,0,0),             # ORA dp
("TSB", IMM,  2,2,2,2,2,0),             # TSB dp
("ORA", IMM,  2,2,2,2,2,2),             # ORA dp
("ASL", IMM,  2,2,2,2,2,2),             # ASL dp
("ORA", IMM,  2,2,2,2,0,0),             # ORA (dp)
# 08
("PHP", IMM,  1,1,1,1,1,1),             # PHP
("ORA", IMM,  3,3,2,3,3,3),             # ORA #imm (M-dependent)
("ASL", IMM,  1,1,1,1,1,1),             # ASL A
("PHD", IMM,  1,1,1,1,0,0),             # PHD
("TSB", IMM,  3,3,3,3,3,0),             # TSB abs
("ORA", IMM,  3,3,3,3,3,3),             # ORA abs
("ASL", IMM,  3,3,3,3,3,3),             # ASL abs
("ORA", IMM,  4,4,4,4,0,0),             # ORA long
# 10
("BPL", IMM,  2,2,2,2,2,2),             # BPL
("ORA", IMM,  2,2,2,2,2,2),             # ORA (dp),Y
("ORA", IMM,  2,2,2,2,2,0),             # ORA (dp)
("ORA", IMM,  2,2,2,2,0,0),             # ORA (dp,S),Y
("TRB", IMM,  2,2,2,2,2,0),             # TRB dp
("ORA", IMM,  2,2,2,2,2,2),             # ORA dp,X
("ASL", IMM,  2,2,2,2,2,2),             # ASL dp,X
("ORA", IMM,  2,2,2,2,0,0),             # ORA (dp),Y
# 18
("CLC", IMM,  1,1,1,1,1,1),             # CLC
("ORA", IMM,  3,3,3,3,3,3),             # ORA abs,Y
("INC", IMM,  1,1,1,1,1,0),             # INC A
("TCS", IMM,  1,1,1,1,0,0),             # TCS
("TRB", IMM,  3,3,3,3,3,0),             # TRB abs
("ORA", IMM,  3,3,3,3,3,3),             # ORA abs,X
("ASL", IMM,  3,3,3,3,3,3),             # ASL abs,X
("ORA", IMM,  4,4,4,4,0,0),             # ORA long,X
# 20
("JSR", IMM,  3,3,3,3,3,3),             # JSR abs
("AND", IMM,  2,2,2,2,2,2),             # AND (dp,X)
("JSL", IMM,  4,4,4,4,0,0),             # JSL long
("AND", IMM,  2,2,2,2,0,0),             # AND dp
("BIT", IMM,  2,2,2,2,2,2),             # BIT dp
("AND", IMM,  2,2,2,2,2,2),             # AND dp
("ROL", IMM,  2,2,2,2,2,2),             # ROL dp
("AND", IMM,  2,2,2,2,0,0),             # AND (dp)
# 28
("PLP", IMM,  1,1,1,1,1,1),             # PLP
("AND", IMM,  3,3,2,3,3,3),             # AND #imm (M-dependent)
("ROL", IMM,  1,1,1,1,1,1),             # ROL A
("PLD", IMM,  1,1,1,1,0,0),             # PLD
("BIT", IMM,  3,3,3,3,3,3),             # BIT abs
("AND", IMM,  3,3,3,3,3,3),             # AND abs
("ROL", IMM,  3,3,3,3,3,3),             # ROL abs
("AND", IMM,  4,4,4,4,0,0),             # AND long# 30
#30               
("BMI", IMM,  2,2,2,2,2,2),             # BMI 
("AND", IMM,  2,2,2,2,2,2),             # AND (dp),Y
("AND", IMM,  2,2,2,2,2,0),             # AND (dp)
("AND", IMM,  2,2,2,2,0,0),             # AND (dp,S),Y
("BIT", IMM,  2,2,2,2,2,0),             # BIT dp,X
("AND", IMM,  2,2,2,2,2,2),             # AND dp,X
("ROL", IMM,  2,2,2,2,2,2),             # ROL dp,X
("AND", IMM,  2,2,2,2,0,0),             # AND (dp),Y
# 38
("SEC", IMM,  1,1,1,1,1,1),             # SEC
("AND", IMM,  3,3,3,3,3,3),             # AND abs,Y
("DEC", IMM,  1,1,1,1,1,0),             # DEC A
("TSC", IMM,  1,1,1,1,0,0),             # TSC
("BIT", IMM,  3,3,3,3,3,0),             # BIT
("AND", IMM,  3,3,3,3,3,3),             # AND abs,X
("ROL", IMM,  3,3,3,3,3,3),             # ROL abs,X
("AND", IMM,  4,4,4,4,0,0),             # AND long,X
# 40
("RTI", IMM,  1,1,1,1,1,1),             # RTI
("EOR", IMM,  2,2,2,2,0,0),             # EOR (dp,X)
("WDM", IMM,  2,2,2,2,0,0),             # WDM
("EOR", IMM,  2,2,2,2,0,0),             # EOR dp
("MVN", IMM,  3,3,3,3,0,0),             # MVN src,dst
("EOR", IMM,  2,2,2,2,2,2),             # EOR dp
("LSR", IMM,  2,2,2,2,2,2),             # LSR dp
("EOR", IMM,  2,2,2,2,0,0),             # EOR (dp)
# 48
("PHA", IMM,  1,1,1,1,1,1),             # PHA
("EOR", IMM,  3,3,2,3,3,3),             # EOR #imm (M-dependent)
("LSR", IMM,  1,1,1,1,1,1),             # LSR A
("PHK", IMM,  1,1,1,1,0,0),             # PHK
("JMP", IMM,  3,3,3,3,3,3),             # JMP abs
("EOR", IMM,  3,3,3,3,3,3),             # EOR abs
("LSR", IMM,  3,3,3,3,3,3),             # LSR abs
("EOR", IMM,  4,4,4,4,0,0),             # EOR long
# 50
("BVC", IMM,  2,2,2,2,2,2),             # BVC
("EOR", IMM,  2,2,2,2,2,2),             # EOR (dp),Y
("EOR", IMM,  2,2,2,2,2,0),             # EOR (dp)
("EOR", IMM,  2,2,2,2,0,0),             # EOR (dp,S),Y
("MVN", IMM,  3,3,3,3,0,0),             # MVN (mirrors 44; same encoding rules)
("EOR", IMM,  2,2,2,2,2,2),             # EOR dp,X
("LSR", IMM,  2,2,2,2,2,2),             # LSR dp,X
("EOR", IMM,  2,2,2,2,0,0),             # EOR (dp),Y
# 58
("CLI", IMM,  1,1,1,1,1,1),             # CLI
("EOR", IMM,  3,3,3,3,3,3),             # EOR abs,Y
("PHY", IMM,  1,1,1,1,1,0),             # PHY
("TCD", IMM,  1,1,1,1,0,0),             # TCD
("JML", IMM,  4,4,4,4,0,0),             # JML (abs)
("EOR", IMM,  3,3,3,3,3,3),             # EOR abs,X
("LSR", IMM,  3,3,3,3,3,3),             # LSR abs,X
("EOR", IMM,  4,4,4,4,0,0),             # EOR long,X
# 60
("RTS", IMM,  1,1,1,1,1,1),             # RTS
("ADC", IMM,  2,2,2,2,2,2),             # ADC (dp,X)
("PER", IMM,  3,3,3,3,0,0),             # PER relative long
("ADC", IMM,  2,2,2,2,0,0),             # ADC dp
("STZ", IMM,  2,2,2,2,2,0),             # STZ dp
("ADC", IMM,  2,2,2,2,2,2),             # ADC dp
("ROR", IMM,  2,2,2,2,2,2),             # ROR dp
("ADC", IMM,  2,2,2,2,0,0),             # ADC (dp)
# 68
("PLA", IMM,  1,1,1,1,1,1),             # PLA
("ADC", IMM,  3,3,2,3,3,3),             # ADC #imm (M-dependent)
("ROR", IMM,  1,1,1,1,1,1),             # ROR A
("RTL", IMM,  1,1,1,1,0,0),             # RTL
("JMP", IMM,  3,3,3,3,3,3),             # JMP abs long indirect: JMP (abs)
("ADC", IMM,  3,3,3,3,3,3),             # ADC abs
("ROR", IMM,  3,3,3,3,3,3),             # ROR abs
("ADC", IMM,  4,4,4,4,0,0),             # ADC long
# 70
("BVS", IMM,  2,2,2,2,2,2),             # BVS
("ADC", IMM,  2,2,2,2,2,2),             # ADC (dp),Y
("ADC", IMM,  2,2,2,2,2,2),             # ADC (dp)
("ADC", IMM,  2,2,2,2,0,0),             # ADC (dp,S),Y
("STZ", IMM,  2,2,2,2,2,0),             # STZ dp,X
("ADC", IMM,  2,2,2,2,2,2),             # ADC dp,X
("ROR", IMM,  2,2,2,2,2,2),             # ROR dp,X
("ADC", IMM,  2,2,2,2,0,0),             # ADC (dp),Y
# 78
("SEI", IMM,  1,1,1,1,1,1),             # SEI
("ADC", IMM,  3,3,3,3,3,3),             # ADC abs,Y
("PLY", IMM,  1,1,1,1,1,0),             # PLY
("TDC", IMM,  1,1,1,1,0,0),             # TDC
("JMP", IMM,  3,3,3,3,3,0),             # JMP (abs,X)
("ADC", IMM,  3,3,3,3,3,3),             # ADC abs,X
("ROR", IMM,  3,3,3,3,3,3),             # ROR abs,X
("ADC", IMM,  4,4,4,4,0,0),             # ADC long,X
# 80
("BRA", IMM,  2,2,2,2,2,0),             # BRA
("STA", IMM,  2,2,2,2,2,2),             # STA (dp,X)
("BRL", IMM,  3,3,3,3,0,0),             # BRL
("STA", IMM,  2,2,2,2,0,0),             # STA 1,S
("STY", IMM,  2,2,2,2,2,2),             # STY dp
("STA", IMM,  2,2,2,2,2,2),             # STA dp
("STX", IMM,  2,2,2,2,2,2),             # STX dp
("STA", IMM,  2,2,2,2,0,0),             # STA SR
# 88
("DEY", IMM,  1,1,1,1,1,1),             # DEY
("BIT", IMM,  3,3,2,3,3,0),             # BIT #imm (M-dependent)
("TXA", IMM,  1,1,1,1,1,1),             # TXA
("PHB", IMM,  1,1,1,1,0,0),             # PHB
("STY", IMM,  3,3,3,3,3,3),             # STY abs
("STA", IMM,  3,3,3,3,3,3),             # STA abs
("STX", IMM,  3,3,3,3,3,3),             # STX abs
("STA", IMM,  4,4,4,4,0,0),             # STA long
# 90
("BCC", IMM,  2,2,2,2,2,2),             # BCC
("STA", IMM,  2,2,2,2,2,2),             # STA (dp),Y
("STA", IMM,  2,2,2,2,2,0),             # STA (dp)
("STA", IMM,  2,2,2,2,0,0),             # STA (dp,S),Y
("STY", IMM,  2,2,2,2,2,2),             # STY dp,X
("STA", IMM,  2,2,2,2,2,2),             # STA dp,X
("STX", IMM,  2,2,2,2,2,2),             # STX dp,Y
("STA", IMM,  2,2,2,2,0,0),             # STA (dp),Y
# 98
("TYA", IMM,  1,1,1,1,1,1),             # TYA
("STA", IMM,  3,3,3,3,3,3),             # STA abs,Y
("TXS", IMM,  1,1,1,1,1,1),             # TXS
("TXY", IMM,  1,1,1,1,0,0),             # TXY
("STZ", IMM,  3,3,3,3,3,0),             # STZ abs
("STA", IMM,  3,3,3,3,3,3),             # STA abs,X
("STZ", IMM,  3,3,3,3,3,0),             # STZ abs,X
("STA", IMM,  4,4,4,4,0,0),             # STA long,X
# A0
("LDY", IMM,  3,2,3,2,3,3),             # LDY #imm (X-dependent)
("LDA", IMM,  2,2,2,2,2,2),             # LDA #imm (M-dependent)
("LDX", IMM,  3,2,3,2,3,3),             # LDX #imm (X-dependent)
("LDA", IMM,  2,2,2,2,0,0),             # LDA SR
("LDY", IMM,  2,2,2,2,2,2),             # LDY dp
("LDA", IMM,  2,2,2,2,2,2),             # LDA dp
("LDX", IMM,  2,2,2,2,2,2),             # LDX dp
("LDA", IMM,  2,2,2,2,0,0),             # LDA (dp)
# A8
("TAY", IMM,  1,1,1,1,1,1),             # TAY
("LDA", IMM,  3,3,2,3,3,3),             # LDA #imm (duplicate immediate forms only differ by mnemonic)
("TAX", IMM,  1,1,1,1,1,1),             # TAX
("PLB", IMM,  1,1,1,1,0,0),             # PLB
("LDY", IMM,  3,3,3,3,3,3),             # LDY abs
("LDA", IMM,  3,3,3,3,3,3),             # LDA abs
("LDX", IMM,  3,3,3,3,3,3),             # LDX abs
("LDA", IMM,  4,4,4,4,0,0),             # LDA long
# B0
("BCS", IMM,  2,2,2,2,2,2),             # BCS
("LDA", IMM,  2,2,2,2,2,2),             # LDA (dp),Y
("LDA", IMM,  2,2,2,2,2,0),             # LDA (dp)
("LDA", IMM,  2,2,2,2,0,0),             # LDA (dp,S),Y
("LDY", IMM,  2,2,2,2,2,2),             # LDY dp,X
("LDA", IMM,  2,2,2,2,2,2),             # LDA dp,X
("LDX", IMM,  2,2,2,2,2,2),             # LDX dp,Y
("LDA", IMM,  2,2,2,2,0,0),             # LDA (dp),Y
# B8
("CLV", IMM,  1,1,1,1,1,1),             # CLV
("LDA", IMM,  3,3,3,3,3,3),             # LDA abs,Y
("TSX", IMM,  1,1,1,1,1,1),             # TSX
("TYX", IMM,  1,1,1,1,0,0),             # TYX
("LDY", IMM,  3,3,3,3,3,3),             # LDY abs
("LDA", IMM,  3,3,3,3,3,3),             # LDA abs,X
("LDX", IMM,  3,3,3,3,3,3),             # LDX abs,Y
("LDA", IMM,  4,4,4,4,0,0),             # LDA long,X
# C0
("CPY", IMM,  3,2,3,2,3,3),             # CPY #imm (X-dependent)
("CMP", IMM,  2,2,2,2,2,2),             # CMP (dp,X)
("REP", IMM,  2,2,2,2,0,0),             # REP #imm (always 2-byte immediate)
("CMP", IMM,  2,2,2,2,0,0),             # CMP 1,S
("CPY", IMM,  2,2,2,2,2,2),             # CPY dp
("CMP", IMM,  2,2,2,2,2,2),             # CMP dp
("DEC", IMM,  2,2,2,2,2,2),             # DEC dp
("CMP", IMM,  2,2,2,2,0,0),             # CMP (dp)
# C8
("INY", IMM,  1,1,1,1,1,1),             # INY
("CMP", IMM,  3,3,2,3,3,3),             # CMP #imm (M-dependent)
("DEX", IMM,  1,1,1,1,1,1),             # DEX
("WAI", IMM,  1,1,1,1,1,0),             # WAI
("CPY", IMM,  3,3,3,3,3,3),             # CPY abs
("CMP", IMM,  3,3,3,3,3,3),             # CMP abs
("DEC", IMM,  3,3,3,3,3,3),             # DEC abs
("CMP", IMM,  4,4,4,4,0,0),             # CMP long
# D0
("BNE", IMM,  2,2,2,2,2,2),             # BNE
("CMP", IMM,  2,2,2,2,2,2),             # CMP (dp),Y
("CMP", IMM,  2,2,2,2,2,0),             # CMP (dp)
("CMP", IMM,  2,2,2,2,0,0),             # CMP (dp,S),Y
("PEI", IMM,  2,2,2,2,0,0),             # PEI dp
("CMP", IMM,  2,2,2,2,2,2),             # CMP dp,X
("DEC", IMM,  2,2,2,2,2,2),             # DEC dp,X
("CMP", IMM,  2,2,2,2,0,0),             # CMP (dp),Y
# D8
("CLD", IMM,  1,1,1,1,1,1),             # CLD
("CMP", IMM,  3,3,3,3,3,3),             # CMP abs,Y
("PHX", IMM,  1,1,1,1,1,0),             # PHX
("STP", IMM,  1,1,1,1,1,0),             # STP
("JML", IMM,  3,3,3,3,0,0),             # JML abs long indirect
("CMP", IMM,  3,3,3,3,3,3),             # CMP abs,X
("DEC", IMM,  3,3,3,3,3,3),             # DEC abs,X
("CMP", IMM,  4,4,4,4,0,0),             # CMP long,X
# E0
("CPX", IMM,  3,2,3,2,3,3),             # CPX #imm (X-dependent)
("SBC", IMM,  2,2,2,2,2,2),             # SBC (dp,X)
("SEP", IMM,  2,2,2,2,0,0),             # SEP #imm (always 2 bytes)
("SBC", IMM,  2,2,2,2,0,0),             # SBC dp
("CPX", IMM,  2,2,2,2,2,2),             # CPX dp
("SBC", IMM,  2,2,2,2,2,2),             # SBC dp
("INC", IMM,  2,2,2,2,2,2),             # INC dp
("SBC", IMM,  2,2,2,2,0,0),             # SBC (dp)
# E8
("INX", IMM,  1,1,1,1,1,1),             # INX
("SBC", IMM,  3,3,2,3,3,3),             # SBC #imm (M-dependent)
("NOP", IMM,  1,1,1,1,1,1),             # NOP
("XBA", IMM,  1,1,1,1,0,0),             # XBA
("INC", IMM,  3,3,3,3,3,3),             # INC abs
("SBC", IMM,  3,3,3,3,3,3),             # SBC abs
("INC", IMM,  3,3,3,3,3,3),             # INC abs
("SBC", IMM,  4,4,4,4,0,0),             # SBC long
# F0
("BEQ", IMM,  2,2,2,2,2,2),             # BEQ
("SBC", IMM,  2,2,2,2,2,2),             # SBC (dp),Y
("SBC", IMM,  2,2,2,2,2,0),             # SBC (dp)
("SBC", IMM,  2,2,2,2,0,0),             # SBC (dp,S),Y
("PEA", IMM,  3,3,3,3,0,0),             # PEA abs
("SBC", IMM,  2,2,2,2,2,2),             # SBC dp,X
("INC", IMM,  2,2,2,2,2,2),             # INC dp,X
("SBC", IMM,  2,2,2,2,0,0),             # SBC (dp),Y
# F8
("SED", IMM,  1,1,1,1,1,1),             # SED
("SBC", IMM,  3,3,3,3,3,3),             # SBC abs,Y
("PLX", IMM,  1,1,1,1,1,0),             # PLX
("XCE", IMM,  1,1,1,1,0,0),             # XCE
("JSR", IMM,  3,3,3,3,0,0),             # JSR (abs,X)
("SBC", IMM,  3,3,3,3,3,3),             # SBC abs,X
("INC", IMM,  3,3,3,3,3,3),             # INC abs,X
("SBC", IMM,  4,4,4,4,0,0)              # SBC long,X
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
        self.e_flag = True   # Assume something.  Correct it upon first status read
        self.m_flag = True
        self.x_flag = True

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
            
    def get_offset(self):
        # Get table offset according to settings of E=4+2, if E=0 2*M+X+2
        if (self.e_flag):
            return 3+2      # For now, use M1X1 entry
        if (self.m_flag):
            val = 2
        else:
            val = 0
        if (self.x_flag):
            val += 1
        return val+2    # Location of instruction length

    def get_instruction_at(self, addr):
        # Get the instruction byte
        op = self.read_mem(addr, 1)
        opv = int.from_bytes(op, "little")
        offset = self.get_offset()
        oplen = opcode_table[opv][offset]
        if (oplen > 1):
            instr = self.read_mem(addr, oplen)
        else:
            instr = op
        return instr


    def print_instruction_at(self, addr):
        instr = self.get_instruction_at(addr)
        opv = int.from_bytes(instr[0:1], "little")
        ns = self.mnemonic = opcode_table[opv][0]
        s = "%06X: " % addr
        s += ns
        s += dump_hex_str(instr)
        return s, instr


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
    # Note: if sa_24 = -1 then don't print leading address
    count = 0
    for b in data:
        if (count == 0) and sa_24 != -1:
            s = "\n%06X:" % sa_24
            print(s, end="")
        s = " %02X" % b
        print(s, end="")
        count += 1
        sa_24 += 1
        if count > 15:
            count = 0
    print("\n")
    return

def dump_hex_str(data):
    count = 0
    s = ""
    for b in data:
        s += " %02X" % b
        count += 1
        if count > 15:
            count = 0
            s += "\n"
    s += "\n"
    return s


        
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
    srec_fn = "allops_m1x1.s19"
    print("Loading %s" % srec_fn)
    pipe.send_srec(srec_fn)
    done = False
    addr = 0x2000
    while not done:
        s, res = pipe.print_instruction_at(addr)
        print(s)
        nexti = int.from_bytes(res[0:1], 'little')
        if nexti == 0:
            done = True
        addr += len(res)
    
    exit(0)
    bp_replaced_val = pipe.set_breakpoint(0x2012)   # Set a breakpoint
    print("\nJumping to program")
    res = pipe.jump_long(0x002000)
    pipe.print_registers()
    pipe.replace_breaks()
    exit(0)
   
   
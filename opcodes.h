void nop(void);
void sec(void);
void clc(void);
void sed(void);
void cld(void);
void sei(void);
void cli(void);
void clv(void);

void sep(void);
void rep(void);

void inx(void);
void dex(void);
void iny(void);
void dey(void);

void stp(void);
void brk(void);

void xce(void);


// branch and jumps
void bra(void);
void brl(void);
void bcs(void);
void bcc(void);
void bne(void);
void beq(void);
void bmi(void);
void bpl(void);
void bvs(void);
void bvc(void);
void jmp(void);
void jml(void);
void jsr(void);
void rts(void);
void jsl(void);
void rtl(void);

// loads
void lda(void);
void ldx(void);
void ldy(void);
void sta(void);
void stz(void);
void stx(void);
void sty(void);

void tax(void);
void tay(void);
void tcd(void);
void tcs(void);
void tdc(void);
void tsc(void);
void tsx(void);
void txa(void);
void txs(void);
void tsx(void);
void txa(void);
void tya(void);
void txy(void);
void tyx(void);
void xba(void);

// Simple stack ops
void pha(void);
void phb(void);
void phd(void);
void phk(void);
void php(void);
void phx(void);
void phy(void);

void pla(void);
void plb(void);
void pld(void);
void plp(void);
void plx(void);
void ply(void);
void plk(void);

void anl(void);
void eor(void);
void ora(void);


void cmp(void);
void cpx(void);
void cpy(void);
void adc(void);
void sbc(void);
void unimp(void);

void inc(void);
void dec(void);
void lsr(void);
void asl(void);
void rol(void);
void ror(void);
void bit(void);
void tsb(void);
void trb(void);

void rti(void);
uint8_t bcd_add8(uint8_t x1, uint8_t x2, uint8_t carry_in);
uint8_t bcd_sub4(uint8_t x1, uint8_t y1, uint8_t b_in);
uint8_t bcd_sub8 (uint8_t x, uint8_t y, uint8_t c_in);

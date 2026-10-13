void nop(cpu_context_t *);
void sec(cpu_context_t *);
void clc(cpu_context_t *);
void sed(cpu_context_t *);
void cld(cpu_context_t *);
void sei(cpu_context_t *);
void cli(cpu_context_t *);
void clv(cpu_context_t *);

void sep(cpu_context_t *);
void rep(cpu_context_t *);

void inx(cpu_context_t *);
void dex(cpu_context_t *);
void iny(cpu_context_t *);
void dey(cpu_context_t *);

void stp(cpu_context_t *);
void brak(cpu_context_t *);

void xce(cpu_context_t *);


// branch and jumps
void bra(cpu_context_t *);
void brl(cpu_context_t *);
void bcs(cpu_context_t *);
void bcc(cpu_context_t *);
void bne(cpu_context_t *);
void beq(cpu_context_t *);
void bmi(cpu_context_t *);
void bpl(cpu_context_t *);
void bvs(cpu_context_t *);
void bvc(cpu_context_t *);
void jmp(cpu_context_t *);
void jml(cpu_context_t *);
void jsr(cpu_context_t *);
void rts(cpu_context_t *);
void jsl(cpu_context_t *);
void rtl(cpu_context_t *);

// loads
void lda(cpu_context_t *);
void ldx(cpu_context_t *);
void ldy(cpu_context_t *);
void sta(cpu_context_t *);
void stz(cpu_context_t *);
void stx(cpu_context_t *);
void sty(cpu_context_t *);

void tax(cpu_context_t *);
void tay(cpu_context_t *);
void tcd(cpu_context_t *);
void tcs(cpu_context_t *);
void tdc(cpu_context_t *);
void tsc(cpu_context_t *);
void tsx(cpu_context_t *);
void txa(cpu_context_t *);
void txs(cpu_context_t *);
void tsx(cpu_context_t *);
void txa(cpu_context_t *);
void tya(cpu_context_t *);
void txy(cpu_context_t *);
void tyx(cpu_context_t *);
void xba(cpu_context_t *);

// Simple stack ops
void pha(cpu_context_t *);
void phb(cpu_context_t *);
void phd(cpu_context_t *);
void phk(cpu_context_t *);
void php(cpu_context_t *);
void phx(cpu_context_t *);
void phy(cpu_context_t *);

void pla(cpu_context_t *);
void plb(cpu_context_t *);
void pld(cpu_context_t *);
void plp(cpu_context_t *);
void plx(cpu_context_t *);
void ply(cpu_context_t *);
void plk(cpu_context_t *);

void anl(cpu_context_t *);
void eor(cpu_context_t *);
void ora(cpu_context_t *);


void cmp(cpu_context_t *);
void cpx(cpu_context_t *);
void cpy(cpu_context_t *);
void adc(cpu_context_t *);
void sbc(cpu_context_t *);
void unimp(cpu_context_t *);

void inc(cpu_context_t *);
void dec(cpu_context_t *);
void lsr(cpu_context_t *);
void asl(cpu_context_t *);
void rol(cpu_context_t *);
void ror(cpu_context_t *);
void bit(cpu_context_t *);
void tsb(cpu_context_t *);
void trb(cpu_context_t *);

void rti(cpu_context_t *);
uint8_t bcd_add8(uint8_t x1, uint8_t x2, uint8_t carry_in);
uint8_t bcd_sub4(uint8_t x1, uint8_t y1, uint8_t b_in);
uint8_t bcd_sub8 (uint8_t x, uint8_t y, uint8_t c_in);

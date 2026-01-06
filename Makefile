CC=gcc
CFLAGS=-Wall -I.
DEPS = sim.h optbl_65816.h disasm.h calc_ea.h vm.h srec.h opcodes.h
OBJ = sim.o optbl_65816.o disasm.o calc_ea.o vm.o srec.o opcodes.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sim: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o 

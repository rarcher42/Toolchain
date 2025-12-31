gcc -c -Wall sim.c
gcc -c -Wall disasm.c
gcc -c -Wall vm.c
gcc -c -Wall srec.c
gcc -c -Wall optbl_65816.c
cc sim.o optbl_65816.o disasm.o vm.o srec.o -o sim



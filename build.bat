gcc -c -Wall disasm.c
gcc -c -Wall vm.c
gcc -c -Wall srec.c
cc disasm.o vm.o srec.o -o disasm


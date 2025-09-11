Bottom Up Allocator
------------------------------

Bottom up allocator for local register allocation. Takes as input a file containing ILOC 
operations and produces an equivalent sequence with no more than (k) registers used.

This program takes three command line arguments
1. The number, k, of registers for the target machine
2. The name of the input file
3. The name of the output file
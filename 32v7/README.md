# ARM Calling Convention

## Register Usage
Registers | Function            | preserved during call
----------|---------------------|----------------------
R0-R3     | args / return value | No
R4-R10    | local variables     | Yes
R11 (FP)  | frame pointer       | Yes
R12 (IP)  | intra-proc/scratch  | No
R13 (SP)  | stack pointer       | Yes
R14 (LR)  | link register       | No
R15 (PC)  | program counter     | No

- if a routine has more than 4 arguments R0-R3 are used for the first 4
  and the rest are placed on the stack
- local variable can also be stored in R0-R3 and even LR (in leaf functions)
- typical subroutine prologue `stmfd sp!,{r4-r6,lr}`
- typical subroutine epilogue `ldmfd sp!,{r4-r6,pc}`

## Condition Codes
cond | code | flags        | description
-----|------|--------------|------------------------------------------
eq   | 0000 | Z==1         | equals / equals zero
ne   | 0001 | Z==0         | not equal
cs/hs| 0010 | C==1         | carry set / unsigned higher or same
cc/lo| 0011 | C==0         |
mi   | 0100 | N==1         | negative
pl   | 0101 | N==0         | positive or zero
vs   | 0110 | V==1         | overflow
vc   | 0111 | V==0         | no overflow
hi   | 1000 | C==1 && Z==0 | unsigned higher
ls   | 1001 | C==0 && Z==1 | unsigned lower or same
ge   | 1010 | N==V         | signed greater or same
lt   | 1011 | N!=V         | signed less than
gt   | 1100 | Z==0 && N==V | signed greater than
le   | 1101 | Z=-1 && N!=V | signed less than or equal
al   | 1110 |              | always executed
--   | 1111 |              | unused (advanced SIMD and floating point)

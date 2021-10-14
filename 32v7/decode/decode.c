#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * R11 FP
 * R12 scratch pad
 * R13 SP
 * R14 LR
 * R15 PC
 */

const char *condition_code[] = {
    "eq", // 0000 equals / equals zero
    "ne", // 0001 not equal
    "cs", // 0010 carry set / unsigned higher or same
    "cc", // 0011 carry clear / unsigned lower
    "mi", // 0100 0bnegative
    "pl", // 0101 positive or zero
    "vs", // 0110 overflow
    "vc", // 0111 no overflow
    "hi", // 1000 unsigned higher
    "ls", // 1001 unsigned lower or same
    "ge", // 1010 signed greater or same
    "lt", // 1011 signed less than
    "gt", // 1100 signed greater than
    "le", // 1101 signed less than or equal
    "al", // 1110 always
    "--", // 1111 unused (advanced SIMD and floating point)

};

typedef union instruction {
    uint32_t raw;
    // basic instruction encoding
    struct {
        uint32_t f3_0:4;
        uint32_t op:1;
        uint32_t f24_5:20;
        uint32_t op1:3;
        uint32_t cond:4;
    } e;
    // branch and branch with link (B, BL)
    struct {
        uint32_t offset:24; // offset
        uint32_t L:1;       // link bit
        uint32_t bits:3;    // should be 101
        uint32_t cond:4;
    } b;
    // data processing
    struct {
        uint32_t opd2:12;   // operand 2
        uint32_t Rd:4;      // second operand reg
        uint32_t Rn:4;      // first operand reg
        uint32_t S:1;       // set condition code
        uint32_t opcode:4;  // opcode
        uint32_t I:1;       // immediate operand
        uint32_t bits:2;    // should be 00
        uint32_t cond:4;
    } d;
    // load / store
    struct {
        uint32_t offset:12; // offset
        uint32_t Rd:4;      // src/dst reg
        uint32_t Rn:4;      // base reg
        uint32_t L:1;       // load/store bit
        uint32_t W:1;       // write-back bit
        uint32_t B:1;       // byte/word bit
        uint32_t U:1;       // up/down bit
        uint32_t P:1;       // pre/post indexing
        uint32_t I:1;       // immediate operand
        uint32_t bits:2;    // should be 01
        uint32_t cond:4;
    } t;
} instruction;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: decode count file\n");
        exit(2);
    }

    int count = atoi(argv[1]);
    FILE *fp = fopen(argv[2], "rb");
    instruction *code;

    code = malloc(4 * count);
    if (fread(code, 4, count, fp) != count) {
        fprintf(stderr, "read error\n");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        instruction in = code[i];
        const char *cc = condition_code[in.e.cond];
        if (in.b.bits == 5) {
            // branch and branch with link
            printf("%08x branch %s %d %06x\n", in.raw, cc, in.b.L, in.b.offset);
        } else if (in.d.bits == 0) {
            // data processing
            printf("%08x data %s %d %xh %d R%d R%d %03xh\n", in.raw, cc, in.d.I, in.d.opcode, in.d.S, in.d.Rn, in.d.Rd, in.d.opd2);
        } else if (in.t.bits == 1) {
            // data processing
            printf("%08x ld/st %s %d %d %d %d %d %d R%d R%d %06xh\n", in.raw, cc, in.t.I, in.t.P, in.t.U, in.t.B, in.t.W, in.t.L, in.t.Rn, in.t.Rd, in.t.offset);
        } else {
            printf("%08x %s %xh %d\n", in.raw, cc, in.e.op1, in.e.op);
        }
    }

    return 0;
}

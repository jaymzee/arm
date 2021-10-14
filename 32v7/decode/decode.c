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

const char *opcode[] = {
    "AND", // 0000
    "EOR", // 0001
    "SUB", // 0010 operand1 - operand2
    "RSB", // 0011 operand2 - operand1
    "ADD", // 0100
    "ADC", // 0101
    "SBC", // 0110 operand1 - operand2 + carry - 1
    "RSC", // 0111 operand2 - operand1 + carry - 1
    "TST", // 1000 same as AND, but result is not written
    "TEQ", // 1001 same as EOR, but result is not written
    "CMP", // 1010 same as SUB, but result is not written
    "CMN", // 1011 same as SUB, but result is not written
    "ORR", // 1100 operand 1 OR operand 2
    "MOV", // 1101 operand 2 (operand 1 is ignored)
    "BIC", // 1110 operand 1 AND NOT operand 2 (bit clear)
    "MVN", // 1111 NOT operand2 (operand 1 is ignored)
};

const char *ldr_str[] = {
    "STR",
    "LDR"
};

const char *shift_type[] = {
    "LSL",
    "LSR",
    "ASR",
    "ROR"
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

union operand2 {
    uint16_t raw;
    struct {
        // operand 2 is a register
        uint16_t Rm:4;  // 2nd operand register
        uint16_t shft_unused:1;
        uint16_t shft_typ:2; // shift applied to Rm
        uint16_t shft_len:5; // shift applied to Rm
    };
    struct {
        // operand 2 is an immediate value
        uint16_t imm:8;
        uint16_t rot:4;
    };
};


int shift_len(union operand2 op2)
{
    if (op2.shft_len == 0) {
        if (op2.shft_typ == 1 || op2.shft_typ == 2) {
            return 32;
        }
    }
    return op2.shft_len;
}

union operand2 op2;
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
            printf("%08x cond=%s branch L=%d offset=0x%06x\n",
                   in.raw, cc, in.b.L, in.b.offset);
        } else if (in.d.bits == 0) {
            // data processing
            union operand2 opd2;
            opd2.raw = in.d.opd2;
            if (in.d.I) {
                // operand 2 is an immediate
                printf("%08x cond=%s %s S=%d R%d, R%d, opd2.rot=%d opd2.imm=0x%02x\n",
                       in.raw, cc, opcode[in.d.opcode],
                       in.d.S, in.d.Rd, in.d.Rn, opd2.rot, opd2.imm);
            } else {
                // operand 2 is a register
                printf("%08x cond=%s %s S=%d R%d, R%d, R%d %s #%d\n",
                       in.raw, cc, opcode[in.d.opcode],
                       in.d.S, in.d.Rd, in.d.Rn, opd2.Rm,
                       shift_type[opd2.shft_typ], shift_len(opd2));
            }
        } else if (in.t.bits == 1) {
            // load / store
            printf("%08x cond=%s %s I=%d P=%d U=%d B=%d W=%d R%d R%d offset=0x%06x\n",
                   in.raw, cc, ldr_str[in.t.L], in.t.I, in.t.P, in.t.U,
                   in.t.B, in.t.W, in.t.Rd, in.t.Rn, in.t.offset);
        } else {
            printf("%08x cond=%s op1=0x%x op=%d\n",
                   in.raw, cc, in.e.op1, in.e.op);
        }
    }

    return 0;
}

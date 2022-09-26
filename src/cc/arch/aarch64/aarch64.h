#pragma once

#include <stdbool.h>

#include "emit.h"

#ifndef EMIT_LABEL
#define EMIT_LABEL(label)  emit_label(label)
#endif
#ifndef EMIT_ASM0
#define EMIT_ASM0(op)  EMIT_ASM2(op, NULL, NULL)
#endif
#ifndef EMIT_ASM1
#define EMIT_ASM1(op, operand1)  EMIT_ASM2(op, operand1, NULL)
#endif
#ifndef EMIT_ASM2
#define EMIT_ASM2(op, operand1, operand2)  EMIT_ASM3(op, operand1, operand2, NULL)
#endif
#ifndef EMIT_ASM3
#define EMIT_ASM3(op, operand1, operand2, operand3)  emit_asm3(op, operand1, operand2, operand3)
#endif
#ifndef EMIT_ASM4
#define EMIT_ASM4(op, operand1, operand2, operand3, operand4)  emit_asm4(op, operand1, operand2, operand3, operand4)
#endif

#ifndef IM
#define IM(x)  im(x)
#endif
#ifndef IMMEDIATE_OFFSET
#define IMMEDIATE_OFFSET(reg, ofs)  immediate_offset(reg, ofs)
#endif
#ifndef PRE_INDEX
#define PRE_INDEX(reg, ofs)  pre_index(reg, ofs)
#endif
#ifndef POST_INDEX
#define POST_INDEX(reg, ofs)  post_index(reg, ofs)
#endif
#ifndef REG_OFFSET
#define REG_OFFSET(base, reg, scale)  reg_offset(base, reg, scale)
#endif
#ifndef LABEL_AT_PAGE
#define LABEL_AT_PAGE(label)  label_at_page(label)
#endif
#ifndef LABEL_AT_PAGEOFF
#define LABEL_AT_PAGEOFF(label)  label_at_pageoff(label)
#endif
#ifndef NUM
#define NUM(x)  num(x)
#endif
#ifndef HEXNUM
#define HEXNUM(x)  hexnum(x)
#endif
#ifndef FLONUM
#define FLONUM(x)  flonum(x)
#endif
#ifndef MANGLE
#define MANGLE(label)  mangle(label)
#endif

#define _UXTW(shift)  fmt("uxtw #%d", shift)
#define _LSL(shift)   fmt("lsl #%d", shift)

#define W0    "w0"
#define W1    "w1"
#define W2    "w2"
#define W3    "w3"
#define W4    "w4"
#define W5    "w5"
#define W6    "w6"
#define W7    "w7"
#define W8    "w8"
#define W9    "w9"
#define W10   "w10"
#define W11   "w11"
#define W12   "w12"
#define W13   "w13"
#define W14   "w14"
#define W15   "w15"
#define W16   "w16"
#define W17   "w17"
#define W18   "w18"
#define W19   "w19"
#define W20   "w20"
#define W21   "w21"
#define W22   "w22"
#define W23   "w23"
#define W24   "w24"
#define W25   "w25"
#define W26   "w26"
#define W27   "w27"
#define W28   "w28"
// #define W29   "w29"
// #define W30   "w30"
// #define W31   "w31"

#define X0    "x0"
#define X1    "x1"
#define X2    "x2"
#define X3    "x3"
#define X4    "x4"
#define X5    "x5"
#define X6    "x6"
#define X7    "x7"
#define X8    "x8"
#define X9    "x9"
#define X10   "x10"
#define X11   "x11"
#define X12   "x12"
#define X13   "x13"
#define X14   "x14"
#define X15   "x15"
#define X16   "x16"
#define X17   "x17"
#define X18   "x18"
#define X19   "x19"
#define X20   "x20"
#define X21   "x21"
#define X22   "x22"
#define X23   "x23"
#define X24   "x24"
#define X25   "x25"
#define X26   "x26"
#define X27   "x27"
#define X28   "x28"
// #define X29   "x29"
// #define X30   "x30"
// #define X31   "x31"
#define FP    "fp"  // x29
#define LR    "lr"  // x30
#define SP    "sp"  // x31
#define ZR    "zr"  // x31: zero register
#define WZR   "wzr"  // x31: zero register(32bit)

#ifndef __NO_FLONUM
#define D0    "d0"
#define D1    "d1"
#define D2    "d2"
#define D3    "d3"
#define D4    "d4"
#define D5    "d5"
#define D6    "d6"
#define D7    "d7"
#define D8    "d8"
#define D9    "d9"
#define D10   "d10"
#define D11   "d11"
#define D12   "d12"
#define D13   "d13"
#define D14   "d14"
#define D15   "d15"
#define D16   "d16"
#define D17   "d17"
#define D18   "d18"
#define D19   "d19"
#define D20   "d20"
#define D21   "d21"
#define D22   "d22"
#define D23   "d23"
#define D24   "d24"
#define D25   "d25"
#define D26   "d26"
#define D27   "d27"
#define D28   "d28"
#define D29   "d29"
#define D30   "d30"
#define D31   "d31"

#define S0    "s0"
#define S1    "s1"
#define S2    "s2"
#define S3    "s3"
#define S4    "s4"
#define S5    "s5"
#define S6    "s6"
#define S7    "s7"
#define S8    "s8"
#define S9    "s9"
#define S10   "s10"
#define S11   "s11"
#define S12   "s12"
#define S13   "s13"
#define S14   "s14"
#define S15   "s15"
#define S16   "s16"
#define S17   "s17"
#define S18   "s18"
#define S19   "s19"
#define S20   "s20"
#define S21   "s21"
#define S22   "s22"
#define S23   "s23"
#define S24   "s24"
#define S25   "s25"
#define S26   "s26"
#define S27   "s27"
#define S28   "s28"
#define S29   "s29"
#define S30   "s30"
#define S31   "s31"
#endif

// Condition
#define CEQ  "eq"
#define CNE  "ne"
#define CLT  "lt"
#define CGT  "gt"
#define CLE  "le"
#define CGE  "ge"
#define CLO  "lo"
#define CHI  "hi"
#define CLS  "ls"
#define CHS  "hs"

#define MOV(o1, o2)           EMIT_ASM2("mov", o1, o2)
#define MOVK(o1, o2, o3)      EMIT_ASM3("movk", o1, o2, o3)
#define LDP(o1, o2, o3)       EMIT_ASM3("ldp", o1, o2, o3)
#define LDR(o1, o2)           EMIT_ASM2("ldr", o1, o2)
#define LDRB(o1, o2)          EMIT_ASM2("ldrb", o1, o2)
#define LDRSB(o1, o2)         EMIT_ASM2("ldrsb", o1, o2)
#define LDRH(o1, o2)          EMIT_ASM2("ldrh", o1, o2)
#define LDRSH(o1, o2)         EMIT_ASM2("ldrsh", o1, o2)
#define STP(o1, o2, o3)       EMIT_ASM3("stp", o1, o2, o3)
#define STR(o1, o2)           EMIT_ASM2("str", o1, o2)
#define STRB(o1, o2)          EMIT_ASM2("strb", o1, o2)
#define STRH(o1, o2)          EMIT_ASM2("strh", o1, o2)
#define UXTB(o1, o2)          EMIT_ASM2("uxtb", o1, o2)
#define UXTH(o1, o2)          EMIT_ASM2("uxth", o1, o2)
#define UXTW(o1, o2)          EMIT_ASM2("uxtw", o1, o2)
#define SXTB(o1, o2)          EMIT_ASM2("sxtb", o1, o2)
#define SXTH(o1, o2)          EMIT_ASM2("sxth", o1, o2)
#define SXTW(o1, o2)          EMIT_ASM2("sxtw", o1, o2)
#define ADD(o1, o2, o3)       EMIT_ASM3("add", o1, o2, o3)
#define SUB(o1, o2, o3)       EMIT_ASM3("sub", o1, o2, o3)
#define SUBS(o1, o2, o3)      EMIT_ASM3("subs", o1, o2, o3)
#define MUL(o1, o2, o3)       EMIT_ASM3("mul", o1, o2, o3)
#define SDIV(o1, o2, o3)      EMIT_ASM3("sdiv", o1, o2, o3)
#define UDIV(o1, o2, o3)      EMIT_ASM3("udiv", o1, o2, o3)
#define MSUB(o1, o2, o3, o4)  EMIT_ASM4("msub", o1, o2, o3, o4)
#define AND(o1, o2, o3)       EMIT_ASM3("and", o1, o2, o3)
#define ORR(o1, o2, o3)       EMIT_ASM3("orr", o1, o2, o3)
#define EOR(o1, o2, o3)       EMIT_ASM3("eor", o1, o2, o3)
#define CMP(o1, o2)           EMIT_ASM2("cmp", o1, o2)
#define CMN(o1, o2)           EMIT_ASM2("cmn", o1, o2)
#define LSL(o1, o2, o3)       EMIT_ASM3("lsl", o1, o2, o3)
#define ASR(o1, o2, o3)       EMIT_ASM3("asr", o1, o2, o3)
#define NEG(o1, o2)           EMIT_ASM2("neg", o1, o2)
#define BRANCH(o1)            EMIT_ASM1("b", o1)
#define Bcc(c, o1)            EMIT_ASM1("b" c, o1)
#define BR(o1)                EMIT_ASM1("br", o1)
#define BL(o1)                EMIT_ASM1("bl", o1)
#define BLR(o1)               EMIT_ASM1("blr", o1)
#define RET()                 EMIT_ASM0("ret")
#define CSET(o1, c)           EMIT_ASM2("cset", o1, c)

#define ADRP(o1, o2)       EMIT_ASM2("adrp", o1, o2)

#define _BYTE(x)       EMIT_ASM1(".byte", x)
#define _WORD(x)       EMIT_ASM1(".word", x)
#define _LONG(x)       EMIT_ASM1(".long", x)
#define _QUAD(x)       EMIT_ASM1(".quad", x)
#define _FLOAT(x)      EMIT_ASM1(".float", x)
#define _DOUBLE(x)     EMIT_ASM1(".double", x)
#define _GLOBL(x)      EMIT_ASM1(".globl", x)
#define _COMM(x, y)    EMIT_ASM2(".comm", x, y)
#define _ASCII(x)      EMIT_ASM1(".ascii", x)
#define _SECTION(x)    EMIT_ASM1(".section", x)
#define _TEXT()        EMIT_ASM0(".text")
#define _DATA()        EMIT_ASM0(".data")

#ifdef __APPLE__
#define _RODATA()      _SECTION("__DATA,__const")
#define EMIT_ALIGN(x)  emit_align_p2(x)
#define _LOCAL(x)      (0)
#else
#define _RODATA()      _SECTION(".rodata")
#define EMIT_ALIGN(x)  emit_align(x)
#define _LOCAL(x)      EMIT_ASM1(".local", x)
#endif


#ifndef __NO_FLONUM
#define FMOV(o1, o2)        EMIT_ASM2("fmov", o1, o2)  // dst <- src
#define FADD(o1, o2, o3)    EMIT_ASM3("fadd", o1, o2, o3)
#define FSUB(o1, o2, o3)    EMIT_ASM3("fsub", o1, o2, o3)
#define FMUL(o1, o2, o3)    EMIT_ASM3("fmul", o1, o2, o3)
#define FDIV(o1, o2, o3)    EMIT_ASM3("fdiv", o1, o2, o3)
#define FCMP(o1, o2)        EMIT_ASM2("fcmp", o1, o2)

#define SCVTF(o1, o2)   EMIT_ASM2("scvtf", o1, o2)  // float <- int
#define UCVTF(o1, o2)   EMIT_ASM2("ucvtf", o1, o2)  // float <- unsigned int
#define FCVT(o1, o2)    EMIT_ASM2("fcvt", o1, o2)  // float <- float
#define FCVTZS(o1, o2)  EMIT_ASM2("fcvtzs", o1, o2)  // int <- float
#endif

void mov_immediate(const char *dst, intptr_t value, bool b64);
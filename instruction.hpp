#pragma once
#pragma ide diagnostic ignored "bugprone-branch-clone"

#include "processor.hpp"
#include "utils.hpp"

#define LD_EXE_CYCLES           3
#define JUMP_EXE_CYCLES         1
#define ADD_EXE_CYCLES          3
#define SUB_EXE_CYCLES          3
#define MUL_EXE_CYCLES          3
#define DIV_EXE_CYCLES          4
#define DIV_ZERO_EXE_CYCLES     1

static int counter = 0;

static int getCycles(char type) {
    switch (type) {
        case 'L': return LD_EXE_CYCLES;
        case 'J': return JUMP_EXE_CYCLES;
        case 'A': return ADD_EXE_CYCLES;
        case 'S': return SUB_EXE_CYCLES;
        case 'M': return MUL_EXE_CYCLES;
        case 'D': return DIV_EXE_CYCLES;
        default:
            error("No such instruction type %c", type);
    }
    unreachable();
}

struct Instruction {
    int id, cycles; bool is_div;
    int dest, src1, src2, addr;
    int condition, src, offset;
    FunctionalUnit fu;

    static Instruction createArithmetic(char type, int dest, int src1, int src2) {
        Instruction instruction{};
        instruction.id = counter ++;
        instruction.cycles = getCycles(type);
        instruction.is_div = (type == 'D');
        instruction.dest = dest;
        instruction.src1 = src1;
        instruction.src2 = src2;
        instruction.fu = (type == 'A' or type == 'S') ? ADD_SUB_UNIT: MUL_DIV_UNIT;

        return instruction;
    }

    static Instruction createLoad(int dest, int addr) {
        Instruction instruction{};
        instruction.id = counter++;
        instruction.cycles = LD_EXE_CYCLES;
        instruction.dest = dest;
        instruction.addr = addr;
        instruction.fu = LOAD_UNIT;

        return instruction;
    }

    static Instruction createJump(int condition, int src, int offset) {
        Instruction instruction{};
        instruction.id = counter ++;
        instruction.cycles = JUMP_EXE_CYCLES;
        instruction.condition = condition;
        instruction.src = src;
        instruction.offset = offset;
        // TODO
        instruction.fu = ADD_SUB_UNIT;

        return instruction;
    }
};
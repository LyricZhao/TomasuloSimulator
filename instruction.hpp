#pragma once
#pragma ide diagnostic ignored "bugprone-branch-clone"

#include "utils.hpp"

#define LOAD_EXE_CYCLES         3
#define JUMP_EXE_CYCLES         1
#define ADD_EXE_CYCLES          3
#define SUB_EXE_CYCLES          3
#define MUL_EXE_CYCLES          3
#define DIV_EXE_CYCLES          4
#define DIV_ZERO_EXE_CYCLES     1

enum InstructionType {
    JUMP, LOAD, ADD, SUB, MUL, DIV
};

static InstructionType getType(char type) {
    switch (type) {
        case 'L': return LOAD;
        case 'J': return JUMP;
        case 'A': return ADD;
        case 'S': return SUB;
        case 'M': return MUL;
        case 'D': return DIV;
        default:
            error("No such instruction type %c", type);
    }
    unreachable();
}

static int getCycles(char type) {
    switch (type) {
        case 'L': return LOAD_EXE_CYCLES;
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
    // Structural information
    InstructionType type;
    int id, dest, src1, src2, addr;
    int condition, src, offset;

    // Results
    int issued_cycle, executed_cycle, written_cycle;

    [[nodiscard]] std::string getName() const {
        switch (type) {
            case ADD: return "ADD";
            case SUB: return "SUB";
            case MUL: return "MUL";
            case DIV: return "DIV";
            case LOAD: return "LOAD";
            case JUMP: return "JUMP";
            default:
                error("No such instruction type %d", type);
        }
        unreachable();
    }

    static Instruction createArithmetic(int id, char type, int dest, int src1, int src2) {
        Instruction instruction{};
        instruction.type = getType(type);
        instruction.id = id;
        instruction.dest = dest;
        instruction.src1 = src1;
        instruction.src2 = src2;

        return instruction;
    }

    static Instruction createLoad(int id, int dest, int addr) {
        Instruction instruction{};
        instruction.type = LOAD;
        instruction.id = id;
        instruction.dest = dest;
        instruction.addr = addr;

        return instruction;
    }

    static Instruction createJump(int id, int condition, int src, int offset) {
        Instruction instruction{};
        instruction.type = JUMP;
        instruction.id = id;
        instruction.condition = condition;
        instruction.src = src;
        instruction.offset = offset;

        return instruction;
    }

    [[nodiscard]] bool isArithmetic() const {
        return type == MUL or type == DIV or type == ADD or type == SUB;
    }

    void issued(int cycle) {
        if (not issued_cycle)
            issued_cycle = cycle;
    }

    void executed(int cycle) {
        if (not executed_cycle)
            executed_cycle = cycle;
    }

    void written(int cycle) {
        if (not written_cycle)
            written_cycle = cycle;
    }
};
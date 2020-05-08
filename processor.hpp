#pragma once

#include "instruction.hpp"
#include "utils.hpp"

#define NUM_REGISTERS 32

typedef uint32_t Register;

enum FunctionalUnitType {
    ADD_SUB_UNIT,
    MUL_DIV_UNIT,
    LOAD_UNIT
};

struct ReservationStation {
    bool busy;
    int op, vj, vk, qj, qk, a;
};

enum TickAction {
    STALL,
    PC_INCREASE,
    SHUTDOWN
};

// In-order issue and out-of-order execution
// Pipelines:
//  => IS (Issue): unit conflicts, WAW
//  => RO (Read Operands): RAW
//  => EX (Execution)
//  => WB (Write Back): WAR
class Processor {
    int cycle;
    Register registers[NUM_REGISTERS];

public:
    TickAction tick(Instruction *instruction) {
        return SHUTDOWN;
    }
};
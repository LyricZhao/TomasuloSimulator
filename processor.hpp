#pragma once

#include "instruction.hpp"
#include "utils.hpp"

#define NUM_REGISTERS   32

#define NUM_ADD_SUB_RS  6
#define NUM_MUL_DIV_RS  3
#define NUM_LOAD_RS     3

#define NUM_ADD_SUB_FU  3
#define NUM_MUL_DIV_FU  2
#define NUM_LOAD_FU     2

struct Register {
    int value;
};

struct FunctionalUnit {
    int remaining_cycles;
    int dest, result;

    // Return whether to write back
    bool tick() {
        if (remaining_cycles) {
            -- remaining_cycles;
            return remaining_cycles == 0;
        }
        return false;
    }
};

// Reservation station for both FU and Loader
struct ReservationStation {
    bool busy;
    FunctionalUnit *fu; // Refer to functional units
    Instruction *instruction; // To be executed
    int op, vj, vk, qj, qk, addr;

    // Return whether to execute
    bool tick() {
        if (busy) {
            if (fu != nullptr) {
                busy = (fu->remaining_cycles == 0);
                return false;
            } else {
                return true;
            }
        }
        return false;
    }
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

    ReservationStation add_sub_rs[NUM_ADD_SUB_RS];
    ReservationStation mul_div_rs[NUM_MUL_DIV_RS];
    ReservationStation load_rs[NUM_LOAD_RS];

    FunctionalUnit add_sub_fu[NUM_ADD_SUB_FU];
    FunctionalUnit mul_div_fu[NUM_MUL_DIV_FU];
    FunctionalUnit load_fu[NUM_LOAD_FU];

public:
    TickAction tick(Instruction *instruction) {
        cycle ++;
        bool running = false;

        // Functional units
        std::vector<FunctionalUnit*> to_write_back;
        for (auto &fu: join3(add_sub_fu, mul_div_fu, load_fu)) {
            if (fu.tick()) {
                to_write_back.push_back(&fu);
            }
        }

        // Reservation Station
        std::vector<ReservationStation*> to_execute;
        for (auto &rs: join3(add_sub_rs, mul_div_rs, load_rs)) {
            if (rs.tick()) {
                to_execute.push_back(&rs);
            }
        }
        // Sort by instruction id
        auto comp = [](ReservationStation* x, ReservationStation* y) -> bool {
            return x->instruction->id < y->instruction->id;
        };
        std::sort(to_execute.begin(), to_execute.end(), comp);

        // TODO: Check whether finish
        if ((not running) and instruction == nullptr)
            return SHUTDOWN;

        // Issue
        auto [rs_ptr, num] = getRelatedRS(instruction->type);
        for (int i = 0; i < num; ++ i) {
            if (not rs_ptr[i].busy) {
                return PC_INCREASE;
            }
        }

        return STALL;
    }

    [[nodiscard]] std::pair<ReservationStation*, int> getRelatedRS(InstructionType type) const {
        switch (type) {
            case ADD: case SUB:
                return to_ptr_with_length(add_sub_rs, NUM_ADD_SUB_RS);
            case MUL: case DIV:
                return to_ptr_with_length(mul_div_rs, NUM_MUL_DIV_RS);
            case LOAD:
                return to_ptr_with_length(load_rs, NUM_LOAD_RS);
            case JUMP:
                unimplemented();
            default:
                error("No such instruction type");
        }
        unreachable();
    }
};
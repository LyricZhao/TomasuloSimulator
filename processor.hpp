#pragma once

#include <map>

#include "instruction.hpp"
#include "timer.hpp"
#include "utils.hpp"

#define NUM_REGISTERS   32

#define NUM_ADD_SUB_RS  6
#define NUM_MUL_DIV_RS  3
#define NUM_LOAD_RS     3

#define NUM_ADD_SUB_FU  3
#define NUM_MUL_DIV_FU  2
#define NUM_LOAD_FU     2

struct FunctionalUnit {
    int remaining_cycles, dest, result;
};

// Reservation station for both FU and Loader
struct ReservationStation {
    bool busy;
    int vj, vk, addr;
    FunctionalUnit *fu; // Refer to functional units
    Instruction *instruction; // To be executed
    ReservationStation *qj, *qk;

    void clear_busy() {
        assert(busy);
        busy = false;
        fu = nullptr;
        vj = vk = addr = 0;
        instruction = nullptr;
        qj = qk = nullptr;
    }

    [[nodiscard]] std::string getInstructionName() const {
        return instruction == nullptr ? "empty" : instruction->getName();
    }

    [[nodiscard]] int getVj() const {
        return qj == nullptr ? vj : 0;
    }

    [[nodiscard]] int getVk() const {
        return qk == nullptr ? vk : 0;
    }
};

struct Register {
    int value;
    ReservationStation *rs; // During the issued to execution
};

enum TickAction {
    STALL,
    PC_INCREASE,
    SHUTDOWN
};

struct ToWriteBack {
    int dest, value;
};

// In-order issue and out-of-order execution/completion
class Processor {
    int cycle{};

    // Writing registers in current cycle
    std::vector<ToWriteBack> to_write_back;

    // Only after the write-back stage, the registers will be updated
    Register registers[NUM_REGISTERS]{};

    // Verify results
    int checker[NUM_REGISTERS]{};

    std::map<ReservationStation*, std::string> rs_names;

    ReservationStation add_sub_rs[NUM_ADD_SUB_RS]{};
    ReservationStation mul_div_rs[NUM_MUL_DIV_RS]{};
    ReservationStation load_rs[NUM_LOAD_RS]{};

    FunctionalUnit add_sub_fu[NUM_ADD_SUB_FU]{};
    FunctionalUnit mul_div_fu[NUM_MUL_DIV_FU]{};
    FunctionalUnit load_fu[NUM_LOAD_FU]{};

    [[nodiscard]] std::pair<ReservationStation*, int> getRelatedRS(Instruction *instruction) const {
        if (instruction == nullptr)
            return std::make_pair(nullptr, 0);

        switch (instruction->type) {
            case ADD: case SUB:
                return to_ptr_with_length(add_sub_rs, NUM_ADD_SUB_RS);
            case MUL: case DIV:
                return to_ptr_with_length(mul_div_rs, NUM_MUL_DIV_RS);
            case LOAD:
                return to_ptr_with_length(load_rs, NUM_LOAD_RS);
            default:
                error("No such instruction type");
        }
        unreachable();
    }

    [[nodiscard]] std::pair<FunctionalUnit*, int> getRelatedFU(Instruction *instruction) const {
        assert(instruction != nullptr);

        switch (instruction->type) {
            case ADD: case SUB:
                return to_ptr_with_length(add_sub_fu, NUM_ADD_SUB_FU);
            case MUL: case DIV:
                return to_ptr_with_length(mul_div_fu, NUM_MUL_DIV_FU);
            case LOAD:
                return to_ptr_with_length(load_fu, NUM_LOAD_FU);
            default:
                error("No such instruction type");
        }
        unreachable();
    }

public:
    Processor() {
        char buffer[256];
        rs_names[nullptr] = "null";
        for (int i = 0; i < NUM_ADD_SUB_RS; ++ i)
            sprintf(buffer, "ADD_SUB_RS[%d]", i), rs_names[&add_sub_rs[i]] = std::string(buffer);
        for (int i = 0; i < NUM_MUL_DIV_RS; ++ i)
            sprintf(buffer, "MUL_DIV_RS[%d]", i), rs_names[&mul_div_rs[i]] = std::string(buffer);
        for (int i = 0; i < NUM_LOAD_RS; ++ i)
            sprintf(buffer, "LOAD_RS[%d]", i), rs_names[&load_rs[i]] = std::string(buffer);
    }

    void print() {
        printf("Current cycle: %d\n", cycle);
        printf("  RS states:\n");
        for (int i = 0; i < NUM_ADD_SUB_RS; ++ i)
            printf("  - ADD_SUB_RS[%d]: busy=%d, op=%s, vj=%d, vk=%d, qj=%s, qk=%s\n",
                    i, add_sub_rs[i].busy, add_sub_rs[i].getInstructionName().c_str(),
                    add_sub_rs[i].getVj(), add_sub_rs[i].getVk(), rs_names[add_sub_rs[i].qj].c_str(), rs_names[add_sub_rs[i].qk].c_str());
        for (int i = 0; i < NUM_ADD_SUB_RS; ++ i)
            printf("  - MUL_DIV_RS[%d]: busy=%d, op=%s, vj=%d, vk=%d, qj=%s, qk=%s\n",
                   i, mul_div_rs[i].busy, mul_div_rs[i].getInstructionName().c_str(),
                   mul_div_rs[i].getVj(), mul_div_rs[i].getVk(), rs_names[mul_div_rs[i].qj].c_str(), rs_names[mul_div_rs[i].qk].c_str());
        for (int i = 0; i < NUM_ADD_SUB_RS; ++ i)
            printf("  - LOAD_RS[%d]: busy=%d, addr=%d\n", i, load_rs[i].busy, load_rs[i].addr);
        printf("  Register states:\n");
        for (int i = 0; i < NUM_REGISTERS; ++ i) {
            printf("  - Reg[%d]: value=%d, waiting_rs=%s\n",
                    i, registers[i].value, rs_names[registers[i].rs].c_str());
        }
        printf("\n");
    }

    void do_verify_step(Instruction *instruction) {
        switch (instruction->type) {
            case ADD:
                checker[instruction->dest] = checker[instruction->src1] + checker[instruction->src2];
                break;
            case SUB:
                checker[instruction->dest] = checker[instruction->src1] - checker[instruction->src2];
                break;
            case MUL:
                checker[instruction->dest] = checker[instruction->src1] * checker[instruction->src2];
                break;
            case DIV: {
                bool div_error = (checker[instruction->src2] == 0) or
                        (checker[instruction->src1] == INT32_MIN and checker[instruction->src2] == -1);
                checker[instruction->dest] = div_error? 0 : checker[instruction->src1] / checker[instruction->src2];
                break;
            }
            case LOAD:
                checker[instruction->dest] = instruction->addr;
                break;
            default:
                error("No such instruction type");
        }
    }

    void verify() {
        for (int i = 0; i < NUM_REGISTERS; ++ i) {
            if (checker[i] != registers[i].value) {
                error("Verify failed at register %d, correct=%d, actual=%d", i, checker[i], registers[i].value);
            }
        }
    }

    TickAction tick(Instruction *instruction) {
        // No-jump implementation
        if (instruction != nullptr and instruction->type == JUMP)
            error("No jump in processor");

        cycle ++;
        bool running = false;
        // printf("Running cycle %d ...\n", cycle);

        // Write back
        // printf("Writing back ...\n");
        for (auto write_back: to_write_back)
            registers[write_back.dest].value = write_back.value;
        to_write_back.clear();

        // Latest registers' value from just-finished RS
        // Just a like CDB (Common Data Bus), collecting result from RS an WB
        std::map<ReservationStation*, int> just_finished_cdb;
        bool registers_updated[NUM_REGISTERS]{};
        int latest_registers_values[NUM_REGISTERS];

        // RS and FU clean up
        // If a FU finishes, write to register next cycle
        // printf("RS and FU clean up ...\n");
        std::vector<ReservationStation*> to_execute;
        for (auto &rs: join3(add_sub_rs, mul_div_rs, load_rs)) {
            if (rs.busy) {
                running = true;
                if (rs.fu == nullptr) {
                    to_execute.push_back(&rs);
                } else {
                    rs.fu->remaining_cycles --;
                    if (rs.fu->remaining_cycles == 0) {
                        // Register is waiting for me
                        if (registers[rs.fu->dest].rs == &rs) {
                            registers[rs.fu->dest].rs = nullptr;
                            to_write_back.push_back(ToWriteBack{rs.fu->dest, rs.fu->result});
                            // CDB
                            registers_updated[rs.fu->dest] = true;
                            latest_registers_values[rs.fu->dest] = rs.fu->result;
                        }
                        rs.instruction->executed(cycle);
                        rs.instruction->written(cycle + 1);
                        // CDB
                        just_finished_cdb[&rs] = rs.fu->result;
                        rs.clear_busy();
                    }
                }
            }
        }

        // Check finish
        // printf("Checking finish ...\n");
        if ((not running) and instruction == nullptr)
            return SHUTDOWN;

        // Issue
        bool stall = true;
        // printf("Issuing ...\n");
        auto [ptr_rs, num_rs] = getRelatedRS(instruction);
        for (int i = 0; i < num_rs; ++ i) {
            // Current instruction is ready
            if (not ptr_rs[i].busy) {
                // printf("Issuing %s\n", instruction->getName().c_str());
                stall = false;
                // Set RS
                ptr_rs[i].busy = true;
                ptr_rs[i].fu = nullptr;
                ptr_rs[i].instruction = instruction;
                switch (instruction->type) {
                    case ADD: case SUB: case MUL: case DIV: {
                        auto set_src = [registers_updated, latest_registers_values] (Register &reg, int &v, ReservationStation* &q, int src) {
                            if (reg.rs != nullptr)
                                q = reg.rs, v = 0;
                            else
                                q = nullptr, v = registers_updated[src] ? latest_registers_values[src] : reg.value;
                        };
                        set_src(registers[instruction->src1], ptr_rs[i].vj, ptr_rs[i].qj, instruction->src1);
                        set_src(registers[instruction->src2], ptr_rs[i].vk, ptr_rs[i].qk, instruction->src2);
                        break;
                    }
                    case LOAD: {
                        ptr_rs[i].addr = instruction->addr;
                        break;
                    }
                    default:
                        error("No such instruction type");
                }
                // Set dest register
                registers[instruction->dest].rs = &ptr_rs[i];
                // Issue
                to_execute.push_back(&ptr_rs[i]);
                instruction->issued(cycle);
                break;
            }
        }

        // Check whether RS ready and execute the instructions
        // Sort the ready ones by instruction id
        // printf("Trying to execute ...\n");
        auto comp = [](ReservationStation* x, ReservationStation* y) -> bool {
            return x->instruction->id < y->instruction->id;
        };
        std::sort(to_execute.begin(), to_execute.end(), comp);

        // Check ready and execute
        for (auto &rs: to_execute) {
            auto [ptr_fu, num_fu] = getRelatedFU(rs->instruction);

            // Update sources
            if (rs->qj != nullptr and just_finished_cdb.count(rs->qj))
                rs->vj = just_finished_cdb[rs->qj], rs->qj = nullptr;
            if (rs->qk != nullptr and just_finished_cdb.count(rs->qk))
                rs->vk = just_finished_cdb[rs->qk], rs->qk = nullptr;

            // Check ready
            if (not (rs->instruction->type == LOAD or (rs->qj == nullptr and rs->qk == nullptr)))
                continue;

            // Find a functional unit and execute
            for (int i = 0; i < num_fu; ++ i) {
                if (ptr_fu[i].remaining_cycles == 0) {
                    // Get result
                    rs->fu = &ptr_fu[i];
                    assert(rs->qj == nullptr and rs->qk == nullptr);
                    int remaining_cycles, result;
                    bool div_error = (rs->vk == 0) or (rs->vj == INT32_MIN and rs->vk == -1);
                    switch (rs->instruction->type) {
                        case ADD:
                            remaining_cycles = ADD_EXE_CYCLES, result = rs->vj + rs->vk; break;
                        case SUB:
                            remaining_cycles = SUB_EXE_CYCLES, result = rs->vj - rs->vk; break;
                        case MUL:
                            remaining_cycles = MUL_EXE_CYCLES, result = rs->vj * rs->vk; break;
                        case DIV:
                            remaining_cycles = div_error ? DIV_ZERO_EXE_CYCLES: DIV_EXE_CYCLES;
                            result = div_error ? 0 : rs->vj / rs->vk; break;
                        case LOAD:
                            remaining_cycles = LOAD_EXE_CYCLES, result = rs->addr; break;
                        default:
                            error("No such instruction type");
                    }

                    // Set FU
                    ptr_fu[i].remaining_cycles = remaining_cycles;
                    ptr_fu[i].dest = rs->instruction->dest;
                    ptr_fu[i].result = result;
                    break;
                }
            }
        }

        if (not stall and instruction != nullptr)
            do_verify_step(instruction);

        return stall ? STALL : PC_INCREASE;
    }
};
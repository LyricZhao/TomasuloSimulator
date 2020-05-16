#pragma once

#define NUM_REGISTERS   32

class Verifier {
    int steps;
    int registers[NUM_REGISTERS];

public:
    int getSteps() {
        return steps;
    }

    int getReg(int id) {
        return registers[id];
    }

    int step(Instruction *instruction) {
        ++ steps;
        int offset = 1;
        switch (instruction->type) {
            case ADD:
                registers[instruction->dest] = registers[instruction->src1] + registers[instruction->src2];
                break;
            case SUB:
                registers[instruction->dest] = registers[instruction->src1] - registers[instruction->src2];
                break;
            case MUL:
                registers[instruction->dest] = registers[instruction->src1] * registers[instruction->src2];
                break;
            case DIV: {
                bool div_error = (registers[instruction->src2] == 0) or
                                 (registers[instruction->src1] == INT32_MIN and registers[instruction->src2] == -1);
                registers[instruction->dest] = div_error? 0 : registers[instruction->src1] / registers[instruction->src2];
                break;
            }
            case LOAD:
                registers[instruction->dest] = instruction->addr;
                break;
            case JUMP:
                if (registers[instruction->src] == instruction->condition)
                    offset = instruction->offset;
                break;
            default:
                error("No such instruction type");
        }
        return offset;
    }
};
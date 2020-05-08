#pragma once
#pragma ide diagnostic ignored "cert-err34-c"

#include <cstdio>
#include <string>
#include <vector>

#include "instruction.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 256

char buffer[BUFFER_SIZE];

class Runner {
    std::string path;
    std::vector<Instruction> instructions;

public:
    explicit Runner(const std::string &path) {
        this->path = path;
    }

    void read() {
        printf("Reading file %s ...\n", path.c_str());
        FILE* file = fopen(path.c_str(), "r");
        if (file == nullptr)
            error("Error while reading file %s, exit\n", path.c_str());

        while (true) {
            char *eof = fgets(buffer, BUFFER_SIZE, file);

            if (eof == nullptr)
                break;

            // ADD, SUB, MUL, DIV, LD, JUMP
            char c = buffer[0];
            if (c == 'A' or c == 'S' or c == 'M' or c == 'D') {
                char x, y, z;
                int dest, src1, src2;
                sscanf(buffer, "%c%c%c,R%d,R%d,R%d", &x, &y, &z, &dest, &src1, &src2);
                instructions.push_back(Instruction::createArithmetic(c, dest, src1, src2));
            } else if (c == 'L') {
                int dest, addr;
                sscanf(buffer, "LD,R%d,%x", &dest, &addr);
                instructions.push_back(Instruction::createLoad(dest, addr));
            } else if (c == 'J') {
                int condition, src, offset;
                sscanf(buffer, "JUMP,%x,R%d,%x", &condition, &src, &offset);
                instructions.push_back(Instruction::createJump(condition, src, offset));
            }
        }
    }

    void run() {
        unimplemented();
    }

    void writeLogs(const std::string& logs_path) {
        FILE* file = fopen(logs_path.c_str(), "r");
        unimplemented();
    }
};
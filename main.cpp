#include <cstdio>

#include "runner.hpp"
#include "utils.hpp"

// #define DEBUG

std::string logsPath(std::string filename) {
    std::string dir = "../Log/";
    filename.replace(filename.find(".nel"), 4, ".log");
    return dir + "2017011362_" + filename;
}

int main() {
    printf("Tomasulo Simulator, 2017011362\n");

    std::string dir = "../TestCase/";
#ifdef DEBUG
    std::string filenames[] = {"0.basic.nel"};
#else
    std::string filenames[] = {"0.basic.nel",
                               "1.basic.nel",
                               "2.basic.nel",
                               "3.basic.nel",
                               "4.basic.nel",
                               "Big_test.nel",
                               "Mul.nel"};
#endif

    for (const auto& filename: filenames) {
        Runner runner(dir + filename);

        runner.read();
        runner.simulate();
        runner.write(logsPath(filename));
    }

    return 0;
}

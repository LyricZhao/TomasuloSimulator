#pragma once

#include <cstdlib>

#define error(fmt, ...)                                                             \
{                                                                                   \
    fprintf(stderr, "\033[31mError in file %s on line %d: ", __FILE__, __LINE__);   \
    fprintf(stderr, fmt, ##__VA_ARGS__);                                            \
    fprintf(stderr, "\033[0m");                                                     \
    fflush(stderr);                                                                 \
    std::exit(EXIT_FAILURE);                                                        \
}

#define unimplemented() error("Unimplemented part in file %s on line %d\n", __FILE__, __LINE__)

#define unreachable() error("Unreachable part in file %s on line %d\n", __FILE__, __LINE__)
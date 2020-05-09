#pragma once

#include <boost/range/join.hpp>
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

template<class Range1, class Range2, class Range3>
boost::joined_range<boost::joined_range<Range1, Range2>, Range3> join3(Range1 &r1, Range2 &r2, Range3 &r3) {
    auto iter = boost::join(r1, r2);
    return boost::join(iter, r3);
}

template<class T>
std::pair<T*, int> to_ptr_with_length(const T array[], int length) {
    return std::make_pair((T*) (&array[0]), length);
}
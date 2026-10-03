#ifndef CPUCACHES_H
#define CPUCACHES_H
#include "cache.h"
#include <iostream>

class cpuCaches {
    unsigned int mem_cycles;
    unsigned int bsize;
    unsigned int wr_allocate;
    cache *l1;
    cache *l2;
    unsigned int l1_accesses;
    unsigned int l2_accesses;
    unsigned int l1_misses;
    unsigned int l2_misses;
    unsigned int mem_accesses;

   public:
    cpuCaches(unsigned int mem_cycles, unsigned int bsize, bool wr_allocate, cache *l1, cache *l2);
    ~cpuCaches() {};
    void do_operation(char op, unsigned long int addr);
    void r_operation(unsigned long int addr);
    void w_operation(unsigned long int addr);
    double getL1MissRate();
    double getL2MissRate();
    double getAvgAccTime();
    void printCurrNum() {
        std::cout << "l1 accesses: " << l1_accesses << " l1 misses: " << l1_misses << std::endl;
        std::cout << "l2 accesses: " << l2_accesses << " l2 misses: " << l2_misses << std::endl;
        std::cout << "mem accesses: " << mem_accesses << std::endl;
    }
};

#endif  // CPUCACHES_H

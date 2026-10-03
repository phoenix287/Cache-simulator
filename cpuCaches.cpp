#include "cpuCaches.h"
#include <iostream>

cpuCaches::cpuCaches(unsigned int mem_cycles, unsigned int bsize, bool wr_allocate, cache *l1,
                      cache *l2)
    : mem_cycles(mem_cycles), bsize(bsize), wr_allocate(wr_allocate), l1(l1), l2(l2) {
    l1_accesses = 0;
    l2_accesses = 0;
    l1_misses = 0;
    l2_misses = 0;
    mem_accesses = 0;
}

void cpuCaches::do_operation(char op, unsigned long int addr) {
    if (op == 'r' || op == 'R') {
        r_operation(addr);
    } else if (op == 'w' || op == 'W') {
        w_operation(addr);
    }
}

void cpuCaches::r_operation(unsigned long int addr) {
    // check in l1
    l1_accesses++;
    if (l1->access(addr)) {
        // L1 hit
        return;
    }
    // L1 miss
    l1_misses++;

    // l1 miss, check l2
    l2_accesses++;
    if (l2->access(addr)) {
        // L2 hit: insert into l1, and if that evicts a dirty block, the
        // write-back it was holding needs to land in L2 instead.
        bool was_evicted = false;
        unsigned int evicted_addr = 0;
        int evicted_dirty = 0;
        l1->insert(addr, &was_evicted, &evicted_addr, &evicted_dirty);
        if (was_evicted && evicted_dirty) {
            l2->make_mru(evicted_addr);
            l2->mark_dirty(evicted_addr);
        }
        return;
    }
    // L2 miss
    l2_misses++;

    // l1 & l2 miss, fetch the block from memory into both l1 and l2
    mem_accesses++;

    bool l2_evicted = false;
    unsigned int evicted_l2_addr = 0;
    int evicted_l2_dirty = 0;
    l2->insert(addr, &l2_evicted, &evicted_l2_addr, &evicted_l2_dirty);
    if (l2_evicted) {
        // preserve inclusivity: anything evicted from l2 must leave l1 too
        l1->invalidate(evicted_l2_addr);
    }

    bool l1_evicted = false;
    unsigned int evicted_l1_addr = 0;
    int evicted_l1_dirty = 0;
    l1->insert(addr, &l1_evicted, &evicted_l1_addr, &evicted_l1_dirty);
    if (l1_evicted && evicted_l1_dirty) {
        l2->make_mru(evicted_l1_addr);
        l2->mark_dirty(evicted_l1_addr);
    }
}

void cpuCaches::w_operation(unsigned long int addr) {
    // check in l1
    l1_accesses++;
    if (l1->access(addr)) {
        // L1 hit: write-back policy, so just modify here and mark dirty
        l1->mark_dirty(addr);
        return;
    }
    // L1 miss
    l1_misses++;

    // check in l2
    l2_accesses++;
    if (l2->access(addr)) {
        // L2 hit
        if (!wr_allocate) {
            l2->mark_dirty(addr);
            return;
        }
        // write-allocate: bring the block into l1 and write there
        bool was_evicted = false;
        unsigned int evicted_addr = 0;
        int evicted_dirty = 0;
        l1->insert(addr, &was_evicted, &evicted_addr, &evicted_dirty);
        if (was_evicted && evicted_dirty) {
            // a dirty block was evicted from l1; because of inclusivity
            // and write-back, l2's copy must now carry that update
            l2->make_mru(evicted_addr);
            l2->mark_dirty(evicted_addr);
        }
        l1->mark_dirty(addr);
        return;
    }
    // L2 miss
    l2_misses++;

    // l1 & l2 miss
    mem_accesses++;
    if (!wr_allocate) {
        // no write-allocate: write straight to memory, nothing to cache
        return;
    }

    // write-allocate: insert into l2
    bool l2_evicted = false;
    unsigned int evicted_l2_addr = 0;
    int evicted_l2_dirty = 0;
    l2->insert(addr, &l2_evicted, &evicted_l2_addr, &evicted_l2_dirty);
    if (l2_evicted) {
        l1->invalidate(evicted_l2_addr);
    }

    // insert into l1 too
    bool l1_evicted = false;
    unsigned int evicted_l1_addr = 0;
    int evicted_l1_dirty = 0;
    l1->insert(addr, &l1_evicted, &evicted_l1_addr, &evicted_l1_dirty);
    if (l1_evicted && evicted_l1_dirty) {
        l2->make_mru(evicted_l1_addr);
        l2->mark_dirty(evicted_l1_addr);
    }

    // the block just added to l1 is a fresh write, so it's dirty
    l1->mark_dirty(addr);
}

double cpuCaches::getL1MissRate() { return (double)l1_misses / l1_accesses; }

double cpuCaches::getL2MissRate() { return (double)l2_misses / l2_accesses; }

double cpuCaches::getAvgAccTime() {
    unsigned int l1cyc = l1->num_of_cycles;
    unsigned int l2cyc = l2->num_of_cycles;
    double l1miss = getL1MissRate();
    double l2miss = getL2MissRate();
    return l1cyc * (1 - l1miss) + (l1cyc + l2cyc) * l1miss * (1 - l2miss) +
           (l1cyc + l2cyc + mem_cycles) * l1miss * l2miss;
}

#include "cache.h"
#include <cmath> // to do log2

cache::cache(unsigned int cache_size, unsigned int block_size, unsigned int log_num_of_ways,
             unsigned int num_of_cycles, unsigned int write_allocate) {
    this->cache_size = pow(2, cache_size);
    this->block_size = pow(2, block_size);
    this->num_of_ways = pow(2, log_num_of_ways);
    this->num_of_cycles = num_of_cycles;
    this->write_allocate = write_allocate;
    this->num_of_sets = this->cache_size / (this->block_size * this->num_of_ways);
    this->offset_bits = (unsigned int)std::log2(this->block_size);
    this->set_bits = (unsigned int)std::log2(this->num_of_sets);
    this->access_counter = 0;
    sets.resize(num_of_sets);  // create our cache's entries

    for (unsigned int i = 0; i < num_of_sets; i++) {
        sets[i].resize(num_of_ways);  // every entry in our cache has num_of_ways blocks
        // every block is empty and invalid and not used at the beginning
        for (unsigned int j = 0; j < num_of_ways; j++) {
            sets[i][j].valid = false;
            sets[i][j].dirty = false;
            sets[i][j].last_used = 0;
            sets[i][j].tag = 0;
        }
    }
}

bool cache::access(unsigned int address) {
    unsigned int mask = (1 << set_bits) - 1;  // to get set
    unsigned int set = (address >> offset_bits) & mask;
    unsigned int current_tag = address >> (offset_bits + set_bits);
    // check if we already have this block in our cache by checking every way in the relevant set
    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (sets[set][i].tag == current_tag && sets[set][i].valid) {
            // update the accessed block's fields for LRU implementation
            access_counter++;
            sets[set][i].last_used = access_counter;
            return true;
        }
    }
    return false;
}

void cache::insert(unsigned int address, bool *was_evicted, unsigned int *evicted_addr,
                    int *evicted_dirty) {
    unsigned int mask = (1 << set_bits) - 1;  // to get set
    unsigned int set = (address >> offset_bits) & mask;
    unsigned int current_tag = address >> (offset_bits + set_bits);
    access_counter++;

    // check if there is an empty way in the relevant set to add this new address
    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (sets[set][i].valid == false) {
            sets[set][i] = Block(current_tag, true, false, access_counter);
            *was_evicted = false;  // we didn't throw any block out
            return;
        }
    }

    // all the ways in the relevant set are taken => evict via LRU
    unsigned long min = access_counter;
    unsigned int LRU_way = 0;
    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (min > sets[set][i].last_used) {
            min = sets[set][i].last_used;
            LRU_way = i;
        }
    }

    // found the victim; report it so the caller can remove it from L1 if
    // we're L2 (inclusivity), or write it back if dirty.
    unsigned int victim_tag = sets[set][LRU_way].tag;
    *was_evicted = true;
    *evicted_addr = (victim_tag << (offset_bits + set_bits)) | (set << offset_bits);
    *evicted_dirty = sets[set][LRU_way].dirty;

    // replace the victim block with the new one
    sets[set][LRU_way].dirty = false;
    sets[set][LRU_way].last_used = access_counter;
    sets[set][LRU_way].tag = current_tag;
    sets[set][LRU_way].valid = true;
}

// Removes a block from this cache -- used to remove a block from L1 when
// it's evicted from L2, to preserve the inclusion principle.
void cache::invalidate(unsigned int address) {
    unsigned int mask = (1 << set_bits) - 1;
    unsigned int set = (address >> offset_bits) & mask;
    unsigned int current_tag = address >> (offset_bits + set_bits);

    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (sets[set][i].valid && sets[set][i].tag == current_tag) {
            sets[set][i].valid = false;
            return;
        }
    }
}

void cache::make_mru(unsigned long int addr) {
    unsigned int mask = (1 << set_bits) - 1;
    unsigned int set = (addr >> offset_bits) & mask;
    unsigned int current_tag = addr >> (offset_bits + set_bits);
    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (sets[set][i].valid && sets[set][i].tag == current_tag) {
            access_counter++;
            sets[set][i].last_used = access_counter;
            return;
        }
    }
}

void cache::mark_dirty(unsigned long int addr) {
    unsigned int mask = (1 << set_bits) - 1;
    unsigned int set = (addr >> offset_bits) & mask;
    unsigned int current_tag = addr >> (offset_bits + set_bits);
    for (unsigned int i = 0; i < num_of_ways; i++) {
        if (sets[set][i].valid && sets[set][i].tag == current_tag) {
            sets[set][i].dirty = true;
            return;
        }
    }
}

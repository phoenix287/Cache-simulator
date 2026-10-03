#ifndef CACHE_H
#define CACHE_H
#include <vector>
#include <cmath>

struct Block {
    unsigned int tag;   // id of the block
    bool valid;         // valid bit
    bool dirty;         // dirty bit - for writing back to main memory
    unsigned long last_used; // counter to implement LRU
    Block(unsigned int tag,bool valid,bool dirty,unsigned long last_used) : tag(tag),valid(valid),dirty(dirty),last_used(last_used){}
    Block() : tag(0), valid(false), dirty(false), last_used(0) {}
};

class cache{
    private:
    unsigned int cache_size;
    unsigned int block_size;
    unsigned int num_of_ways;
    unsigned int write_allocate;
    unsigned long access_counter; // for LRU implementation
    
    //cache attributes
    unsigned int num_of_sets; //how many sets we can save in one way 
    unsigned int offset_bits;
    unsigned int set_bits;
    
    std::vector<std::vector<Block> > sets;//an array of sets such that every set can have at most num_of_ways tags
    public:
    unsigned int num_of_cycles;
    cache(unsigned int cache_size,unsigned int block_size,unsigned int log_num_of_ways,unsigned int num_of_cycles,unsigned int write_allocate);
    bool access(unsigned int address);
    // Inserts `address` into the cache. If this causes an eviction,
    // *was_evicted is set true, *evicted_addr receives the evicted block's
    // address, and *evicted_dirty receives its dirty bit. If no eviction
    // happens, *was_evicted is set false and the other two are untouched.
    void insert(unsigned int address, bool *was_evicted, unsigned int *evicted_addr, int *evicted_dirty);
    void invalidate(unsigned int address) ;
    void make_mru(unsigned long int addr);
    void mark_dirty(unsigned long int addr);
};

#endif  // CACHE_H
# Two-Level Cache Simulator

A C++ simulator for a CPU with L1 and separate L2 caches: set-associative,
write-back, write-allocate or no-write-allocate (configurable), LRU
replacement, and an inclusive L1/L2 hierarchy.

## What I wrote vs. provided skeleton

`cache.h`/`cache.cpp` (the cache itself) and `cpuCaches.h`/`cpuCaches.cpp`
(the read/write logic coordinating L1, L2, and memory) are me and my partner's
implementation. `cacheSim.cpp` is mostly course-provided skeleton for
argument parsing and reading the trace file -- the final few lines
(computing and printing the miss rates and average access time) are mine.

## How it works

- Each cache is split into sets, each holding `num_of_ways` blocks
  (set-associative). An address is split into offset / set index / tag
  based on the cache and block sizes.
- **Read**: check L1, then L2, then main memory, inserting into each level
  missed along the way.
- **Write**: write-back -- a hit just marks the block dirty rather than
  writing through to memory immediately. Whether a miss allocates a cache
  line is controlled by `--wr-alloc`.
- **Eviction**: LRU within a set. Because L1 is inclusive in L2, evicting a
  block from L2 invalidates it in L1 too; evicting a dirty block from L1
  marks the corresponding L2 block dirty, so the write-back isn't lost.

## Build & run

```bash
make
./cacheSim trace.txt --mem-cyc 100 --bsize 2 --l1-size 4 --l2-size 6 \
  --l1-cyc 1 --l2-cyc 5 --l1-assoc 0 --l2-assoc 1 --wr-alloc 1
```
`trace.txt` is a text file of `R 0x<hex address>` / `W 0x<hex address>`
lines. `--bsize`/`--l1-size`/`--l2-size`/`--l1-assoc`/`--l2-assoc` are all
given as log2 (e.g. `--bsize 2` means a 4-byte block).

Output: `L1miss`, `L2miss` (miss rates) and `AccTimeAvg` (average memory
access time in cycles).

## Bugs fixed from the original version

- A comment lost its `//` (`L2 miss` on its own line), which didn't compile.
- `cache::insert` signaled "no eviction" by returning `(unsigned int)-1`,
  but callers stored that in a 64-bit `unsigned long int`. The 32-bit `-1`
  doesn't equal the 64-bit `-1` once widened, so every `== -1` check
  silently evaluated false -- in one path this made the code invalidate an
  L1 block using a bogus address on *every* L2 insert, not just real
  evictions. Fixed by having `insert` report eviction through an explicit
  `bool*` output parameter instead of a sentinel return value.
- The L2-hit path in `r_operation` evicted a dirty L1 block and called
  `l2->make_mru()` but not `l2->mark_dirty()`, unlike the equivalent L1-miss
  path, so the write-back could be lost. Both call sites now mark the
  evicted block dirty in L2.
- Minor cleanup: removed a dead commented-out `access()` overload, an
  unused variable, and an unused `friend class cache;` declaration; fixed
  signed/unsigned loop-counter warnings; added include guards.

## Credits

Built with Roaia Mahajna for Computer Structure course,semester Winter 2025-2026.

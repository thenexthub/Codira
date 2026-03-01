##  Prerequisites
Build project with `-DENABLE_PERF_COUNTERS=true` for CMake or `enable_perf_counters` for GN.

## How to use
Using global object `ark::tooling::g_perf` create scoped collector object to measure counters for specific part of code.
It is posible to have multiple collectors in different threads. See `ark::tooling::Perf::CreateCollector`.
After all code completed use `ark:tooling::gperf::Report(std::ostream &)` to print counters. Do not forget to call
`ark::tooling::g_perf::Reset()` before next usage.

## Example
```
#include "runtime/tooling/perf_counter.h"

// Thread 1
{
   auto collector = ark::tooling::g_perf.CreateCollector();
   // Measured code
   ...
}

// Thread 2
{
   auto collector = ark::tooling::g_perf.CreateCollector();
   // Measured code
   ...
}

// After all code completed
std::cerr << ark::tooling::g_perf;
ark::tooling::g_perf.Reset();
```
### Customize counters
Required counters are passed to `ark::tooling::Perf::Perf(std::initializer_list<const PerfCounterDescriptor *> list)`

### Default counters output
It looks like
```
              1.689ms wall time
              5.413ms task-clock
           11 827 309 total cpu cycles (2.185 GHz)
            4 009 952 stalled backend cycles (0.339 of cycles)
           12 263 124 instructions
                1.037 insn per cycle
                0.327 stalled cycles per insn
```

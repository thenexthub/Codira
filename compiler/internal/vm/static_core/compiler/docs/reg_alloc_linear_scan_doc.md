# Register allocator
## Overview 

`Register allocator` assigns program variables to target CPU registers

## Rationality

Minimize load/store operations with memory

## Dependence 
* LivenessAnalysis
* DominatorsTree
* Reverse Post Order (RPO)

## Algorithm

Current algorithm is based on paper "Linear Scan Register Allocation" by Massimiliano Poletto. There are 4 main stages:

### Assigning registers and stack slots to the instructions' intervals

`Register allocator` works with instructions' lifetime intervals, computed by `LivenessAnalyzer`. It scans forward through the intervals, ordered by increasing starting point and assign registers while the number of the intervals at the point is less then the number of available registers. If at some point there is no available register to assign to the interval, `Register allocator` spills one active
interval to the stack. Intervals for general and vector registers are independent.

### Inputs resolution

#### Resolve instructions inputs and output

At this point register allocator resolves location for each instruction's input depending on register or stack slot that was assigned to an input instruction's life interval and assigns destination location.

If input's life interval was split into multiple segments then a location is taken from a segment covering the user. In case when input's interval was split at the user the location could be taken from both segments (the segment that ends before the user and the segment that starts at the user) and a register is preferred over a stack slot.

For PHI's inputs having segmented life intervals location at the end of the preceding block is taken.

For SaveState instruction input locations are taken at the SaveState's user rather then at SaveState itself, because this instruction should capture program state at its user. If SaveState have several users (for example, for virtual calls single SaveState instruction is reused by both NullCheck and CallVirtual) and at least one of the SaveState's input has different location at these users then SaveState is copied and original instruction is replaced by the copy for one of its inputs.

#### Adding spill-fill instructions

`SpillFillInst` is a pseudo instruction, holds information about moving data between registers and stack. 

```cpp
struct SpillFillData {
    LocationType src_type : 4;
    LocationType dst_type : 4;
    uint8_t src;
    uint8_t dest;
    DataType::Type type;
};

enum LocationType {
    INVALID_LOCATION = 0,
    REGISTER = 1,
    STACK = 2,
    IMMEDIATE = 3,
};
```

`Register allocator` iterates over instructions and inserts `SpillFillInst` in the following cases:
* Pop instruction's input form preassigned stack slot to spill-fill register;
* Push instruction's result from spill-fill register to preassigned stack slot; 
* Load immediate to register or slot; 
* For each phi input move it to phi destination register or stack slot if theirs locations are different;

Besides, `SpillFillInst` is assigned to the dymamic-inputs instructions (SaveStateInst, CallInst, IntrinsicInst) and containts information about theirs inputs location in the `src_type` and `src` fields. Fields `dst_type` and `dest` are not used in that case and should be `INVALID`.

### Life intervals split resolving

On this stage `Register allocator` connects life intervals that were split into multiple segments during assignment stage using spill-fill instructions. If different locations were assigned to a segment live at the end of some basic block and its sibling live at the beginning of a succeeding block then spill-fills are inserted to control-flow edge to copy value from predecessor's location to successor's location.

### Spill-fills resolving

On this stage `Register allocator` checks if there are overwriting in the chain of spill-fill moves and reorders them if needed:
```
from:
R1 -> R2, R2 -> R3
to:
R2 -> R3, R1 -> R3
```
Cyclical dependent moves will be resolved with spill-fill register: 
```
from:
R1 -> R2, R2 -> R1
to:
R1 -> TEMP, R2-> R1, TEMP -> R2
```

### Parameters locations

Depending on calling convention for target CPU parameters can be located either on registers or in the stack slots. This information is stored with `SpillFillData` inside parameter's instruction and it is filled at the IR-building stage.  


## Pseudocode

TBD

## Examples

```cpp
 Test Graph:
               [0]
                |
                v
         /-----[2]<----\
         |      |      |
         |      v      |
         |     [3]-----/
         |
         \---->[4]
                |
                v
              [exit]

    GRAPH(GetGraph()) {
        CONSTANT(0, 1);
        CONSTANT(1, 10);
        CONSTANT(2, 20);

        BASIC_BLOCK(2, 3, 4) {
            INST(3, Opcode::Phi).u64().Inputs({{0, 0}, {3, 7}});
            INST(4, Opcode::Phi).u64().Inputs({{0, 1}, {3, 8}});
            INST(5, Opcode::Compare).b().Inputs(4, 0);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }

        BASIC_BLOCK(3, 2) {
            INST(7, Opcode::Mul).u64().Inputs(3, 4);
            INST(8, Opcode::Sub).u64().Inputs(4, 0);
        }

        BASIC_BLOCK(4, -1) {
            INST(10, Opcode::Add).u64().Inputs(2, 3);
            INST(11, Opcode::Return).u64().Inputs(10);
        }
    }
    GetGraph()->RunPass<RegAllocLinearScan>();

----------------------------------------------------------------
 ID   INST(INPUTS)    LIFE NUMBER LIFE INTERVALS      ARM64 REGS
----------------------------------------------------------------
 0.   Constant        2           [2-22]              R4
 1.   Constant        4           [4-8]               R5
 2.   Constant        6           [6-24]              R6

 3.   Phi (0,7)       8           [8-16][22-24]       R7
 4.   Phi (1,8)       8           [8-18]              R8
 5.   Cmp (4,0)       10          [10-12]             R9
 6.   If    (5)       12          -
 
 7.   Mul (3,4)       16          [16-22]             R5
 8.   Sub (4,0)       18          [18-22]             R9
 9.   Add (2,3)       24          [24-26]             R4
```

## Links
Source code:

[reg_alloc_linear_scan.cpp](../optimizer/optimizations/regalloc/reg_alloc_linear_scan.cpp)

[reg_alloc_linear_scan.h](../optimizer/optimizations/regalloc/regalloc/reg_alloc_linear_scan.h)

Tests:

[reg_alloc_linear_scan_test.cpp](../tests/reg_alloc_linear_scan_test.cpp)

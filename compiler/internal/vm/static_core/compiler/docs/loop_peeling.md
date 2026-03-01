# Loop Peeling
## Overview 

`Loop peeling` optimization modifies the loops with exit-point at loop-header to the loops with exit-point at loop-backedge.

## Rationality

Simplify the loop and allow further loop optimizations.

## Dependence 

* Loop Analysis
* Dominators Tree
* Reverse Post Order (RPO)

## Algorithm

`Loop peeling` modifies loops with the following requirements:
 - loop is not irreducible;
 - loop-header is not OSR-entry;
 - there is only 1 back-edge;
 - loop-header is a single loop-exit point;
 - there are no inner loops;
 
```cpp
           [pre-header]
               |
               v
      /---->[header]--------\
      |        |            |
      |        v            v
      \----[back-edge]   [outer]
```
 
  There are two stages of the algorithm:

### 1. Insert pre-loop
 ```cpp
          [pre-header]
               |
               v
           [pre-loop]--------\
               |             |
               v             v
      /---->[header]-------->|
      |        |             |
      |        v             v
      \----[back-edge]   [resolver]
                             |
                             v
                          [outer]
 ```
Pre-loop basic block is a loop-header clone with all instructions, excluding `SafePoint`.

### 2. Move exit-point form the loop-header to the loop-backedge block
```cpp
         [pre-header]
               |
               v
           [pre-loop]--------\
               |             |
               v             v
      /---->[header]         |
      |        |             |
      |        v             v
      \----[back-edge]-->[resolver]
                             |
                             v
                          [outer]
```
All instructions from loop-header are moving to the loop-backedge block. Also control-flow edge between loop-header and resolver-block is moving to the loop-backedge.

## Pseudocode

```cpp
auto header = loop->GetHeader();
auto pre-loop = LoopPeeling->CreatePreLoop();
for (auto inst : header->GetInstructions()) {
    auto clone_inst = Clone(inst);
    pre-loop.AppendInst(clone_inst);  
}
auto exit_block = LoopPeeling->CreateExitBlock();
for (auto inst : header->GetInstructionsReverse()) {
        header->EraseInst(inst);
        exit_block->PrependInst(inst);
}
```

## Examples

               [0]
                |
                v
         /---->[2]-----\
         |      |      |
         |      v      v
         \-----[3]    [4]
                       |
                     [exit]
 
 
    GRAPH(GetGraph()) {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(2, 3, 4) {
            INST(3, Opcode::Phi).u64().Inputs(1, 5);
            INST(4, Opcode::Phi).u64().Inputs(2, 10);
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 3, 4).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(3, 2) {
            INST(9, Opcode::And).u64().Inputs(4, 5);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
        }
        BASIC_BLOCK(4, -1) {
            INST(11, Opcode::Return).u64().Inputs(4);
        }
    }

 `Loop peeling` transforms to:
 
               [0]
                |
                v
            [pre-loop]---------\
                |              |
         /---->[2]             |
         |      |              |
         |      v              |
         |     [3]             |
         |      |              |
         |      v              v
         \--[loop-exit]--->[loop-outer]
                               |
                               v
                              [4]
                               |
                               v
                             [exit]

    GRAPH(expected_graph) {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();
        BASIC_BLOCK(5, 2, 4) {
            INST(12, Opcode::Sub).u64().Inputs(1, 2);
            INST(13, Opcode::Compare).CC(CC_EQ).b().Inputs(12, 0);
            INST(14, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(13);
        }
        BASIC_BLOCK(2, 2, 4) {
            INST(3, Opcode::Phi).u64().Inputs({{5, 12}, {2, 5}});
            INST(4, Opcode::Phi).u64().Inputs({{5, 2}, {2, 10}});
            INST(15, Opcode::Phi).u64().Inputs({{5, 12}, {2, 5}});
            INST(9, Opcode::And).u64().Inputs(4, 15);
            INST(10, Opcode::Add).u64().Inputs(9, 4);
            INST(5, Opcode::Sub).u64().Inputs(3, 2);
            INST(6, Opcode::SafePoint).Inputs(0, 5, 10).SrcVregs({0, 1, 2});
            INST(7, Opcode::Compare).CC(CC_EQ).b().Inputs(5, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }
        BASIC_BLOCK(4, -1) {
            INST(16, Opcode::Phi).u64().Inputs({{5, 2}, {2, 10}});
            INST(11, Opcode::Return).u64().Inputs(16);
        }
    }

## Links
Source code:

[loop_peeling.cpp](../optimizer/optimizations/loop_peeling.cpp)

[loop_peeling.h](../optimizer/optimizations/loop_peeling.h)

Tests:

[loop_peeling_test.cpp](../tests/loop_peeling_test.cpp)
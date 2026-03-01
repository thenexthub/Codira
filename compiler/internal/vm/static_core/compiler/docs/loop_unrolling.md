# Loop Unrolling 
## Overview 

`Loop unrolling` optimization increases loop body by copying instructions of the original loop body.

## Rationality

Increase number of instructions for each loop iteration, reduce branch penalty.

## Dependence 

* Loop Analysis
* Dominators Tree
* Reverse Post Order (RPO)
* Loop Peeling (to make loop with exit-point from backedge)


## Algorithm

`Loop unrolling` modifies loops with the following requirements:

* loop is not irreducible;
* loop-header is not OSR-entry;
* there is only 1 back-edge;
* loop-backedge is a single loop-exit point;
* there are no inner loops;

Optimization settings:

**Instructions limit** - the maximum number of loop instructions after its unrolling;

**Unroll factor** - the number of loop body copies including the original one;

There two types of unrolling: with side-exits and without them. Unrolling without side-exits is applied for countable loops.

### Countable loops

Loop is countable if it contains compare between loop-index instruction and test-instruction defined outside loop. 
Loop-index should be incremented or decremented by a constant. Currently signed integer indexes are supported. 

```
[Loop-header]
Phi(init, update)
...

[Loop-backedge]
update(phi, constant)
Compare(update, test)
...
where `update` is Add or Sub instruction
```
### Unrolling without side-exits

There are 3 stages of unrolling loop without side-exits:

1. Clone loop-body without loop-backedge `factor` times;
2. Fix loop-backedge compare by incrementing/decrementing test input with constant, counted using formula: `factor * loop_step`. If compare has `not-equal` condition code, replace it by `less-than`/`greater-than`   
3. Clone loop-body with the original loop-backedge `factor` times, but replace edge to the loop-header with edge the loop-outer block;

Here `factor` means number of cloned loop bodies.

```cpp
/---->[header]
|        |
|        v
|     [loop-body]
|        |
|        v
\-----[backedge]----> ...
```
```
  /---->[header]
  |        |
  |        v
  |     [loop-body]
  |        |
  |        v
  |     [loop-body']
  |        |
  |        v
  |     [loop-body'']
  |        |
  |        v
  \-----[backedge]----> ...
           |
           v
       [loop-body]
           |
           v
       [backedge]------\
           |           |
           v           |
       [loop-body]     |
           |           |
           v           |
      [outer-block]<---/
```



### Unrolling with side-exits

For this case both loop-body and loop-backedge are cloned:
```cpp
   /---->[header]
  |         |
  |         v
  |     [loop-body]
  |         |
  |         v
  |     [backedge]------------\   << exit-block
  |         |                 |
  |         v                 |
  |    [loop-body-clone]      |
  |         |                 |
  |         v                 |
  \-----[backedge-clone]----->|       << last-block
                              |
                              v
                           [outer]-----> ...
```
## Pseudocode
```cpp
if (IsLoopCountable(loop)) {
    auto clone_loop = CloneLoop(loop);
    UnrollLoopBodyWithoutSideExits(loop);
    FixCompareInst(loop);
    UnrollLoopBodyWithSideExits(clone_loop);
    RemoveEdgeToLoopHeader(clone_loop);
} else {
    UnrollLoopBodyWithSideExits(loop);
}
```
## Examples

Countable loop unrolling:
```cpp
    auto graph = CreateEmptyGraph();
    GRAPH(graph) {
        CONSTANT(0, stop);
        CONSTANT(1, 0);  // a = 0, b = 0
        CONSTANT(2, step);
        BASIC_BLOCK(2, 3, 4) {
            INST(3, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);  // if a < stop
        }
        BASIC_BLOCK(3, 3, 4) {
            INST(5, Opcode::Phi).s32().Inputs(1, 7);  // a
            INST(6, Opcode::Phi).s32().Inputs(1, 8);  // b
            INST(7, Opcode::Add).s32().Inputs(5, 2);  // a += step
            INST(8, Opcode::Add).s32().Inputs(6, 7);  // b += a
            INST(9, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(7, 0);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);  // if a < stop
        }
        BASIC_BLOCK(4, -1) {
            INST(11, Opcode::Phi).s32().Inputs(1, 6);
            INST(12, Opcode::Return).s32().Inputs(11);  // return b;
        }
    }
    return graph;

```
```cpp
    uint32_t UNROLL_FACTOR = 2;

    GRAPH(graph_unroll) {
        CONSTANT(0, 10);
        CONSTANT(1, 0);  // a = 0, b = 0
        CONSTANT(2, 1);
        BASIC_BLOCK(2, 3, 5) {
            INST(20, Opcode::SubI).s32().Inputs(0).Imm(UNROLL_FACTOR - 1);
            INST(3, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(1, 20);  // if (a < 10 - (UNROLL_FACTOR - 1))
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }
        BASIC_BLOCK(3, 3, 5) {
            INST(5, Opcode::Phi).s32().Inputs(1, 21);   // a
            INST(6, Opcode::Phi).s32().Inputs(1, 22);   // b
            INST(7, Opcode::Add).s32().Inputs(5, 2);    // a + 1
            INST(8, Opcode::Add).s32().Inputs(6, 7);    // b + 1
            INST(21, Opcode::Add).s32().Inputs(7, 2);   // a + 1
            INST(22, Opcode::Add).s32().Inputs(8, 21);  // b + 1
            INST(9, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(21, 20);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);  // if (a < 10 - (UNROLL_FACTOR - 1))
        }
        BASIC_BLOCK(5, 6, 4) {
            INST(11, Opcode::Phi).s32().Inputs(1, 8);
            INST(25, Opcode::Phi).s32().Inputs(1, 21);                                       // a
            INST(26, Opcode::Phi).s32().Inputs(1, 22);                                       // b
            INST(27, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(25, 0);  // if (a < 10)
            INST(28, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(27);
        }
        BASIC_BLOCK(6, 4) {
            INST(29, Opcode::Add).s32().Inputs(25, 2);   // a + 1
            INST(30, Opcode::Add).s32().Inputs(26, 29);  // b + 1
        }
        BASIC_BLOCK(4, -1) {
            INST(31, Opcode::Phi).s32().Inputs(11, 26);
            INST(12, Opcode::Return).s32().Inputs(31);  // return b
        }
    }

```
## Links
Source code:

[loop_unroll.cpp](../optimizer/optimizations/loop_unroll.cpp)

[loop_unroll.h](../optimizer/optimizations/loop_unroll.h)

Tests:

[loop_unroll_test.cpp](../tests/loop_unroll_test.cpp)
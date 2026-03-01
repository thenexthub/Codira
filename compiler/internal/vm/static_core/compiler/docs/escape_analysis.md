# EscapeAnalysis

## Overview
**Escape Analysis** pass performs optimizations using information about object's scope. Currently there is only one such an optimization - **Scalar replacement** which eliminates an object allocation if the object never escapes a method.

## Dependencies
* CatchInputs
* LoopAnalysis
* Inlining

## Algorithm
Escape analysis pass implements control flow sensitive partial escape analysis algorithm described by Stadler et al in [Stadler 2014]. In addition to objects that never escape a method this algorithm also capable of finding execution paths where an object is not escaping and move its allocation to a path where it can escape.

To check if an object is escaping a method the algorithm initially marks all the allocations as virtual and iterates over instructions in the basic blocks to check if an instruction can lead to object's materialization. If the object requires materialization then the information about it's state prior to materialization will be bound to the instruction that caused materialization. Iteration over the basic blocks continues until there are no new materializations.
At the end of the analysis each object allocation has a state describing if an allocation could be eliminated or moved to some other place.

Escape analysis assigns a state to each basic block (BasicBlockState) that binds an object state to each ref-typed instruction.
The object state is either virtual state (VirtualState) or implicit materialized state. Multiple instructions can share the same state (it can happen, for example, when an instruction is just an alias to original allocation, like NullCheck).

VirtualState contains information about the instruction it was originally created for and mapping between fields and instructions whose values were stored to these fields.
Information about fields serve several purposes:
- avoid materialization on StoreObject/LoadObject instructions;
- correctly initialize the object if materialization is required;
- replace LoadObject from virtual object with the instruction that was stored in the field.

At the beginning of the block its state is evaluated using states of predecessing blocks. If there is only one block then its state is simply copied. Multiple predecessors require states merge. Merge works only with the objects that are alive at the beginning of the current block and are alive at the end of all the predecessing blocks. If the object was materialized in at least one predecessor then it should be also materialized in the current block. For the objects that are virtual in all the predecessors the fields having different values in a predecessor should be merged by inserting a phi into the current block and replacing field's value with that phi. Note that insertion of the phi may cause further materialization.

The state of loop's header could not be evaluated using procedure described above as initially there will be no state for the back edge. The loop header's state is copied from the pre-header and then the loop blocks get processed like any other blocks until back edge states are ready. After that the header's state is merged from the pre-header and the back edges and the resulting state is compared with the initial header's state. This process continues until the header's state stop changing.

Escape analysis produce following state:
* set of objects whose allocation could be either eliminated, or at least moved to some other place;
* set of phi instructions that should be inserted into each block;
* alias mapping - a map from an instruction in the original method to an in instruction that should replace all original instruction's uses after scalar replacement;
* virtual object states at each materialization site;
* information about virtual inputs for each save state.

Scalar replacement uses the state described above to remove virtual objects allocations and insert object's materialization where needed.
Scalar replacement algorithm performs following steps:
* Allocates all required phi instructions;
* Materialize virtual objects where needed;
* Replace all aliased instructions using alias mapping;
* Resolve inputs of all newly created phi instructions;
* Update save states and users of materialized allocations;
* Remove dead allocations, loads/stores and alias instructions.

Decompose analysis splits deoptimization insts carry CAN_DEOPTIMIZE flag into two basic blocks for further optimization.
Decompose analysis algorithm performs following steps:
* Identify deopt insts have CAN_DEOPTIMIZE flag and not deoptimization inst;
* Split basic blocks at above deopt insts position in reversed order;
* Create new deopt basic block and create new deoptimization inst in this block;
* Replace origin deopt insts by new compare and ifimm insts;
* Create new savestate inst for users in splited basic block;
* If materialization happened in new deopt basic block, will finish analysis;
* Otherwise, recover origin insts and control flow graph before decompose analysis.

## Pseudocode
``
EscapeAnalysis:
    Calculate LiveIns

    while there are some new materializations:
        for each basic block:
            if block is a loop header:
                ProcessLoop(block)
            else:
                MergePredecessorsState(block)
                for each inst from the block:
                    VisitInst(inst)


MergePredecessorsState(block):
    for each phi:
        materialized phi's input in correponding predecessor

    while there are materializations happened durnig a merge:
        for each ref-typed instruction inst from block's live ins:
            if inst has virtual state in at least one block's predecessor:
                materialize inst in block's dominator
            else:
                MergeFields(block, inst)


MergeFields(block, inst):
    for each field from inst's virtual state:
        if predecessors' states have the same field value:
            vstate(inst) <- field
        else:
            materialize instructions associated with field
            vstate(inst) <- phi(field values from predecessors)


ProcessLoop(block):
    header_state <- preheader's state copy

    while header_state changes:
        for each loop's block:
            MergePredecessorsState(loop's block)
            for each inst from loop's block:
                VisitInst(inst)
        header_state <- MergePredecessorsState(header)


VisitInst(inst):
    if inst is an allocation:
        if inst does not require materialiation:
            vstate(inst) <- new vstate
        else:
            materialize(inst)

    else if inst is load:
        if load's input is virtual:
            record inst as an alias to a field from input's vstate

    else if inst is store:
        if store's target is virtual:
            vstate(target) <- field
        else:
            materialize stored instruction

    else if inst is a save state:
        materialize all instruction having this save state as materialization site
        capture which inputs are virtual

    else:
        materialize inst's virtual inputs if it can escape through the inst


ScalarReplacement:
    create new phi instructions (requested during fields and states merge);
    create new allocations at materialization sites;
    replace load, stores and other aliases with aliased instrutions;
    updated all save states by removing virtual objects and inserting new materialized objects;
    remove dead instructions.
``

### Difference from original algorithm
Unlike the algorithm described in [Stadler 2014] the current escape analysis pass merges object states only for the objects that are known to be alive at the beginning of the target block. It slightly simplifies the implementation, but requires live-ins calculation.

Current implementation does not support array allocation, but it could be extended to work with it. It should reduce allocation in case of inlined vararg methods.

## Examples
Refer to [EscapeAnalysisTest](../tests/escape_analysis_test.cpp) for examples.

## Links
[Stadler 2014] Stadler, Lukas, Thomas Würthinger, and Hanspeter Mössenböck. "Partial escape analysis and scalar replacement for Java"

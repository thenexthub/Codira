# Common Subexpression Elimination (Cse)

## Overview
`Cse` eliminates duplicate computations. If there are two instructions whose inputs and opcodes and types are all the same, then we move users from second instruction to first instructions(first instruction is dominate), and then delete the second instruction.

## Rationality
Reducing the number of instructions.

## Dependence
* RPO analysis;
* Dominators Tree;

## Algorithm
Our Cse consists of three consective parts, which handle different scenario.

### LocalCse
`LocalCse` eliminates the duplicate computations for every basic block. If there are two instructions whose inputs and opcodes and types are all the same, then we move users from second instruction to first instructions(first instruction is at front of the second), and then delete the second instruction.

### Pseudocode
```
ArenaVector<Inst*> deleted_insts;
for (auto bb : GetGraph()->GetBlocksRPO()) {
    ArenaVector<Inst*> candidates;
    for (auto inst : bb->AllInsts()) {
        if (inst is not a binary expression) {
            continue;
        }
        if (there exists instn in candidates such that inst and instn are duplicate){
            inst->ReplaceUsers(instn);
            deleted_insts.push_back(inst);
        } else {
            candidates.push_back(inst);
        }
    }
}
for (auto inst : deleted_insts) {
    inst->GetBasicBlock()->RemoveInst(inst);
}
```

### DomTreeCse
`DomTreeCse` eliminates the duplicate computations along dominator tree. If there are two instructions inst1 and inst2 with the same expression such that inst1's block dominates inst2's block, then we can move the users of inst2 into inst1, and then delete inst2. Here we make a convention that a block does not dominate itself. It can be viewed as a generation of `LocalCse`.

### Pseudocode
```
ArenaVector<Inst*> candidates;
ArenaVector<std::pair<Inst*, Inst*>> replace_pair
for (auto bb : GetGraph()->GetBlocksRPO()) {
    for (auto inst : bb->Insts()) {
        if (inst is not a binary expression) {
            continue;
        }
        ArenaVector<Inst*> candidates_erase_tmp;
        bool inst_available = true; 
        if (there exists instn in candidates such that inst and instn have same expression){
            if (instn Dominates inst) {
                replace_pair.emplace_back(inst, instn);
                inst_available = false;
                break;
            }
            if (inst Dominates instn) {
                replace_pair.emplace_back(instn, inst);
                candidates_erase_tmp.push_back(instn);
            }
        }

        if (inst_available) {
            candidates.push_back(inst);
        }
        for (auto instn : candidates_erase_tmp) {
            candidates.erase(instn);
        }
    }
}

ArenaVector<std::pair<Inst*, Inst*>> min_replace_star;
for (auto rpair : replace_pair) {
    find the longest path of the form rpair = (inst, inst1), (inst1, inst2), ..., (inst(r-1), instr), in which every pair is in replace_pair;
    min_replace_star.emplace_back(instr, inst);
}

ArenaVector<Inst*> deleted_insts;
for (auto pair : min_replace_star) {
    pair.second->ReplaceUsers(pair.first);
    deleted_insts.push_back(pair.second);
}
for (auto inst : deleted_insts) {
    DeleteInstLog(inst);
    inst->GetBasicBlock()->RemoveInst(inst);
}
```

### GlobalCse
`GlobalCse` eliminates the redundant computations whose result can be obtained from its (two) predecessors. In this case we will use a new Phi instruction to take place of the redundant instruction.

### Pseudocode
```
ArenaVector<Inst*> deleted_insts;
ArenaVector<std::pair<Inst*, std::pair<Inst*, Inst*>>> matched_tuple;
for (auto bb : GetGraph()->GetBlocksRPO()) {
    if (bb->GetPredsBlocks().size() != 2) {
        continue;
    }
    ArenaVector<std::pair<Inst*, Inst*>>same_exp_pair;
    auto bbl = bb->GetPredsBlocks()[0];
    auto bbr = bb->GetPredsBlocks()[1];
    for (auto instl : bbl->Insts()) {
        if (instl is not a binary expression || instl is in deleted_insts) {
            continue;
        }
        for (auto instr : bbr->Insts()) {
            if (instr is not a binary expression || instr is in deleted_insts) {
                continue;
            }
            if (instl and instr have same expression) {
                same_exp_pair.emplace_back(instl, instr);
            }
        }
    }

    for (auto inst : bb->Insts()) {
        if (there exists a pair in same_exp_pair whose expression is same as inst) {
            matched_tuple.emplace_back(inst, pair);
        }
    }
}

for (auto tuple : matched_tuple) {
    auto inst = tuple.first;
    auto phi = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
    inst->ReplaceUsers(phi);
    inst->GetBasicBlock()->AppendPhi(phi);
    auto pair = tuple.second;
    phi->AppendInput(pair.first);
    phi->AppendInput(pair.second);
}

for (auto inst : deleted_insts) {
    DeleteInstLog(inst);
    inst->GetBasicBlock()->RemoveInst(inst);
}

bool Cse::RunImpl() {
    LocalCse();
    DomTreeCse();
    GlobalCse();
}
```

## Examples
Before Cse:
```
BB 0
prop: start
    0.u64  Parameter                  arg 0 -> (v10, v6, v7, v13)
    1.u64  Parameter                  arg 1 -> (v10, v6, v7, v13)
    2.f64  Parameter                  arg 2 -> (v11, v9)
    3.f64  Parameter                  arg 3 -> (v11, v9)
    4.f32  Parameter                  arg 4 -> (v8, v12)
    5.f32  Parameter                  arg 5 -> (v8, v12)
succs: [bb 2]

BB 2  preds: [bb 0]
    6.u64  Add                        v0, v1 -> (v32)
    7.u32  Sub                        v1, v0 -> (v32)
    8.f32  Mul                        v4, v5 -> (v32)
    9.f64  Div                        v3, v2 -> (v32)
   10.u32  Sub                        v1, v0 -> (v32)
   11.f64  Div                        v3, v2 -> (v32)
   12.f32  Mul                        v4, v5 -> (v32)
   13.u64  Add                        v0, v1 -> (v32)  
   14.u64  Mod                        v0, v1 -> (v32)
   15.u64  Min                        v0, v1 -> (v32)
   16.u64  Max                        v0, v1 -> (v32)
   17.u64  Shl                        v0, v1 -> (v32)
   18.u64  Shr                        v0, v1 -> (v32)
   19.u64  AShr                       v0, v1 -> (v32)
   20.b    And                        v0, v1 -> (v32)
   21.b    Or                         v0, v1 -> (v32)
   22.b    Xor                        v0, v1 -> (v32)
   23.u64  Mod                        v0, v1 -> (v32)
   24.u64  Min                        v0, v1 -> (v32)
   25.u64  Max                        v0, v1 -> (v32)
   26.u64  Shl                        v0, v1 -> (v32)
   27.u64  Shr                        v0, v1 -> (v32)
   28.u64  AShr                       v0, v1 -> (v32)
   29.b    And                        v0, v1 -> (v32)
   30.b    Or                         v0, v1 -> (v32)
   31.b    Xor                        v0, v1 -> (v32)
   32.b    CallStatic                 v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31
   33.     ReturnVoid
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```
After Cse:
```
BB 0
prop: start
    0.u64  Parameter                  arg 0 -> (v10, v6, v7, v13)
    1.u64  Parameter                  arg 1 -> (v10, v6, v7, v13)
    2.f64  Parameter                  arg 2 -> (v11, v9)
    3.f64  Parameter                  arg 3 -> (v11, v9)
    4.f32  Parameter                  arg 4 -> (v8, v12)
    5.f32  Parameter                  arg 5 -> (v8, v12)
succs: [bb 2]

BB 2  preds: [bb 0]
    6.u64  Add                        v0, v1 -> (v32)
    7.u32  Sub                        v1, v0 -> (v32)
    8.f32  Mul                        v4, v5 -> (v32)
    9.f64  Div                        v3, v2 -> (v32)
   14.u64  Mod                        v0, v1 -> (v32)
   15.u64  Min                        v0, v1 -> (v32)
   16.u64  Max                        v0, v1 -> (v32)
   17.u64  Shl                        v0, v1 -> (v32)
   18.u64  Shr                        v0, v1 -> (v32)
   19.u64  AShr                       v0, v1 -> (v32)
   20.b    And                        v0, v1 -> (v32)
   21.b    Or                         v0, v1 -> (v32)
   22.b    Xor                        v0, v1 -> (v32)
   32.b    CallStatic                 v6, v7, v8, v9, v7, v9, v8, v6, v14, v15, v16, v17, v18, v19, v20, v21, v22, v14, v15, v16, v17, v18, v19, v20, v21, v22
   33.     ReturnVoid
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

## Links
Source code:
[cse.cpp](../optimizer/optimizations/cse.cpp)
[cse.h](../optimizer/optimizations/cse.h)

Tests:
[cse_test.cpp](../tests/cse_test.cpp)

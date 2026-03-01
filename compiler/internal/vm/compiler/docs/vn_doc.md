# Value Numbering

## Overview 

Value numbering sets special numbers(`vn`) to all instructions. If two instruction has equal VN, so the instructions are equals.    
At the case we move users from second instruction to first instructions(first instruction is dominate).

## Rationality

Reducing the number of instructions. 

## Dependence 

RPO analysis, DominatorsTree.

## Algorithm

All instructions have field `vn_`.  
We pass through all instructions in PRO order. If the instruction has attribute NO_Cse, we set next `vn` to the field.  
For other instructions we save information: opcode, type, `vn` of instruction inputs, advanced properties(for example type_id).  
Based on the collected information, we are looking for a equivalent instructions in the hash map.

1. If equivalent instructions were found:  
    a. If some equivalent instruction dominates current instruction, we move users from current instruction.  
    b. If all equivalent instructions do not dominate current instruction, we insert the instruction in the equivalent instructions vector, and also get `vn` from the first equivalent instruction ans set to current.  
2. If equivalent instructions weren't found, we set next `vn` to the current instruction field and add information about the instruction in the hash map.  

Value numbering doesn't remove instructions. Instructions without users will be removed by [Cleanup](cleanup_doc.md) in next passes

## Pseudocode

```cpp
bool ValNum::RunImpl() {
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FindEqualVnOrCreateNew(inst);
        }
    }
    return true;
}

void ValNum::FindEqualVnOrCreateNew(Inst* inst) {
    if (inst->IsNotCseApplicable()) {
        inst->SetVN(vn_++);
        return;
    }
    auto obj = GetGraph()->GetLocalAllocator()->New<VnObject>();
    obj->Add(inst);
    auto it = map_insts_.find(obj);
    if (it == map_insts_.cend()) {
        inst->SetVN(vn_++);
        map_insts_.insert({obj, {inst}});
        return;
    }
    auto& equiv_insts = it->second;
    if (!TryToApplyCse(inst, &equiv_insts)) {
        equiv_insts.push_back(inst);
    }
}

bool ValNum::TryToApplyCse(Inst* inst, InstVector* equiv_insts) {
    inst->SetVN((*equiv_insts)[0]->GetVN());
    for (auto equiv_inst : *equiv_insts) {
        if (equiv_inst->IsDominate(inst)) {
            inst->ReplaceUsers(equiv_inst);
            return true;
        }
    }
    return false;
}
```

## Examples

Before ValNum:

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
    6.u64  Add                        v0, v1 -> (v14)
    7.u32  Sub                        v1, v0 -> (v14)
    8.f32  Mul                        v4, v5 -> (v14)
    9.f64  Div                        v3, v2 -> (v14)
   10.u32  Sub                        v1, v0 -> (v14)
   11.f64  Div                        v3, v2 -> (v14)
   12.f32  Mul                        v4, v5 -> (v14)
   13.u64  Add                        v0, v1 -> (v14)
   14.b    CallStatic                 v6, v7, v8, v9, v10, v11, v12, v13
   15.     ReturnVoid
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

After ValNum

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
    6.u64  Add                        v0, v1 -> (v14, v14)
    7.u32  Sub                        v1, v0 -> (v14, v14)
    8.f32  Mul                        v4, v5 -> (v14, v14)
    9.f64  Div                        v3, v2 -> (v14, v14)
   10.u32  Sub                        v1, v0
   11.f64  Div                        v3, v2
   12.f32  Mul                        v4, v5
   13.u64  Add                        v0, v1
   14.b    CallStatic                 v6, v7, v8, v9, v7, v9, v8, v6
   15.     ReturnVoid
succs: [bb 1]
```

After DCE:

```
BB 0
prop: start
    0.u64  Parameter                  arg 0 -> (v6, v7)
    1.u64  Parameter                  arg 1 -> (v6, v7)
    2.f64  Parameter                  arg 2 -> (v9)
    3.f64  Parameter                  arg 3 -> (v9)
    4.f32  Parameter                  arg 4 -> (v8)
    5.f32  Parameter                  arg 5 -> (v8)
succs: [bb 2]

BB 2  preds: [bb 0]
    6.u64  Add                        v0, v1 -> (v14, v14)
    7.u32  Sub                        v1, v0 -> (v14, v14)
    8.f32  Mul                        v4, v5 -> (v14, v14)
    9.f64  Div                        v3, v2 -> (v14, v14)
   14.b    CallStatic                 v6, v7, v8, v9, v7, v9, v8, v6
   15.     ReturnVoid
succs: [bb 1]
```

## Links

Source code:   
[vn.cpp](../optimizer/optimizations/vn.cpp)  
[vn.h](../optimizer/optimizations/vn.h)  

Tests:  
[vn_test.cpp](../tests/vn_test.cpp)

# Lowering
## Overview 

Lowering makes instructions more low-level(close to the architecture).  

## Rationality

Reducing the number of instructions. 

## Dependence 

* RPO analysis.
* Encoder from codegen.
* Peephole optimization.

## Algorithm

We pass through all instructions in PRO order.
If the instruction and its input can be encoded with single instruction for a specific archecture, then we change the instruction.  
The main conversation is to replace an instruction with a constant input with the instruction with a constant as an operand, if the architecture supports these instructions.  
The encoder is used for these  

Lowering doesn't remove instructions. Instructions without users will be removed by [Cleanup](cleanup_doc.md) in next passes

## Pseudocode

for (auto inst: All insts in RPO order) {
    VisitInstruction(inst);
}

void VisitAdd/Sub(Inst* inst) {
    if (!inst->GetInput(1)->IsConst()) {
        return
    }
    uint64_t val = inst->GetInput(1)->GetConst();
    if (encoder->CanEncodeImmAddSubCmp(val)) {
        Inst* new_inst = CreateInstAddI(inst->GetType(), inst->GetPc(), val);
        inst->ReplaceUsers(new_inst)
    }
}

}

## Examples

Before Lowering  

```
BB 0
prop: start
    0.u64  Parameter                  arg 0 -> (v9, v8, v7, v6, v4, v5, v17, v17, v18, v18, v19, v20, v10)
   11.f64  Parameter                  arg 1 -> (v13, v14, v10)
   12.f32  Parameter                  arg 2 -> (v15, v16, v10)
    1.i64  Constant                   0xc -> (v7, v4, v19, v10)
    2.i64  Constant                   0xffffffffffffffff -> (v8, v5, v20, v10)
    3.i64  Constant                   0x5f5e100 -> (v9, v6, v10)
   21.f64  Constant                   1.2 -> (v13, v14, v10)
   22.f32  Constant                   0.5 -> (v15, v16, v10)
succs: [bb 2]

BB 2  preds: [bb 0]
    4.u64  Add                        v0, v1 -> (v10)
    5.u64  Add                        v0, v2 -> (v10)
    6.u64  Add                        v0, v3 -> (v10)
    7.u64  Sub                        v0, v1 -> (v10)
    8.u64  Sub                        v0, v2 -> (v10)
    9.u64  Sub                        v0, v3 -> (v10)
   13.f64  Add                        v11, v21 -> (v10)
   14.f64  Sub                        v11, v21 -> (v10)
   15.f32  Add                        v12, v22 -> (v10)
   16.f32  Sub                        v12, v22 -> (v10)
   17.u64  Add                        v0, v0 -> (v10)
   18.u64  Sub                        v0, v0 -> (v10)
   19.u16  Add                        v0, v1 -> (v10)
   20.u16  Add                        v0, v2 -> (v10)
   10.     SaveState                  v0(vr0), v1(vr1), v2(vr2), v3(vr3), v4(vr4), v5(vr5), v6(vr6), v7(vr7), v8(vr8), v9(vr9), v11(vr10), v12(vr11), v13(vr12), v14(vr13), v15(vr14), v16(vr15), v17(vr16), v18(vr17), v19(vr18), v20(vr19), v21(vr20), v22(vr21)
   23.     ReturnVoid
succs: [bb 1]
```

After Lowering and DCE

```
BB 0
prop: start
    0.u64  Parameter                  arg 0 -> (v27, v26, v25, v24, v9, v6, v17, v17, v18, v18, v19, v20, v10)
   11.f64  Parameter                  arg 1 -> (v13, v14, v10)
   12.f32  Parameter                  arg 2 -> (v15, v16, v10)
    1.i64  Constant                   0xc -> (v19)
    2.i64  Constant                   0xffffffffffffffff -> (v20)
    3.i64  Constant                   0x5f5e100 -> (v9, v6)
   21.f64  Constant                   1.2 -> (v13, v14)
   22.f32  Constant                   0.5 -> (v15, v16)
succs: [bb 2]

BB 2  preds: [bb 0]
   24.u64  AddI                       v0, 0xc -> (v10)
   25.u64  SubI                       v0, 0x1 -> (v10)
    6.u64  Add                        v0, v3 -> (v10)
   26.u64  SubI                       v0, 0xc -> (v10)
   27.u64  AddI                       v0, 0x1 -> (v10)
    9.u64  Sub                        v0, v3 -> (v10)
   13.f64  Add                        v11, v21 -> (v10)
   14.f64  Sub                        v11, v21 -> (v10)
   15.f32  Add                        v12, v22 -> (v10)
   16.f32  Sub                        v12, v22 -> (v10)
   17.u64  Add                        v0, v0 -> (v10)
   18.u64  Sub                        v0, v0 -> (v10)
   19.u16  Add                        v0, v1 -> (v10)
   20.u16  Add                        v0, v2 -> (v10)
   10.     SaveState                  v0(vr0), v20(vr19), v19(vr18), v18(vr17), v24(vr4), v25(vr5), v6(vr6), v26(vr7), v27(vr8), v9(vr9), v11(vr10), v12(vr11), v13(vr12), v14(vr13), v15(vr14), v16(vr15), v17(vr16),  0xc(vr1),  0x3f000000(vr21),  0x3ff3333333333333(vr20),  0xffffffffffffffff(vr2),  0x5f5e100(vr3)
   23.     ReturnVoid
succs: [bb 1]

```

## Links

Source code:   
[lowering.cpp](../optimizer/optimizations/lowering.cpp)  
[lowering.h](../optimizer/optimizations/lowering.h)  

Tests:  
[lowering_test.cpp](../tests/lowering_test.cpp)

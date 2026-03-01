# Code generator
## Overview 
Codegen is the backend for current compiler implementation. It was implemented with the main idea - to be independent of specific encoder-architecture and have the possibility to work with different calling-conventions. It was designed to be independence from target-architecture - also, for this target - was created special compiler-independent library - encoder ([encoder.md](../optimizer/code_generator/encoder.md)). 

## Dependence

It is needed Regalloc-pass and LinearOrder-analysis for codegen-work. First - because it is needed to fill register for each operand, second - because of needed jump-instruction generation for major edges.

## Implementation details

List of major codegen-dependency:
1. Encoder library (calling convention, encoder, and register description).
2. CodeBuilder - contain binary data and hold header with information for stack-walker.
3. CFrameLayout - is responsible for stack-frame layout in runtime(class described in `libarkbase/utils`).
4. SlowPath - class, which is responsible for side exits.

Codegen internal-implementation is responsibility for:
1. Conversions from IR to encoder structures (like conditions, immediate, registers, etc).
2. Filling meta-info for recovery frame-information in exceptions (state stamps, which used in stack-walker).
3. Getting information from runtime about current methods and objects.
4. Supporting logic for build AOT-code (there are needed runtime calls during aot-code execution).
5. Correct parameter transferring.

## Logic of work

Constructor makes additional work - it walks through call-instructions and calculates parameters count (they need for reserve stack-slots). 

The main logic was made in RunImpl-method:
1. Generates Calling-Convention prologue.
2. Creates encode-visitor and visits each instruction in linear order by blocks.
3. After that - side exits (SlowPath) are emitted (also for OSR).
4. Generate epilog for correctness exit from method
5. At the end - there is the finalization of the encoder(emit real offsets for each branch) and CodeBuilder-header filling.

## Additional features

Also, there is implemented ScopedDisasmPrinter(for print disassembly), EncodeVisitor (for generating special opcodes).

Example of disasm-dump:
```
- START_METHOD
0000: stp x0, x20, [sp, #-32]!
0004: stp x29, x30, [sp, #16]
0008: add x29, sp, #0x10 (16)
...
- CallRuntime
0014: mov x16, #0xcbd4
0018: movk x16, #0x52, lsl #16
001c: blr x16
0020: ldp x30, xzr, [sp], #16
...
```

## Pseudocode 
codegen.cpp:
```
void Codegen::RunImpl() {
  GetCallingConvention()->BeginMethod(); // emit moves from parameters to dst-regs etc.

  for (auto bb : GetGraph()->GetBlocksLinearOrder()) {
     GetEncoder()->BindLabel(bb->GetId());
     for (auto inst : bb->AllInsts()) {
        visitor.VisitInstruction(inst);
     }
  }

  EmitSlowPaths(); // Emit code, which responsibility for side exits.
  GetEncoder()->Finalize(); // After that - it is possible to use generated code.
  GetGraph()->SetCode(EncodeDataType(code_entry, code_size)); // This data(entry and code) was used in code-cache
}

void Codegen::CreateCall(inst) {
    auto callconv = GetCallingConvention();
    auto dst_reg = ConvertRegister(call_inst->GetDstReg(), call_inst->GetType());
    // 1-st parameter register
    Reg param = GetRegfile()->GetSpecialRegister(SpecialReg::DEFAULT_PARAMETER);
    if (GetGraph()->IsAotMode()) {
        ... // AOT code
    } else {
        GetEncoder()->SaveCallerRegisters();
        SetCallParameters(call_inst);
        auto method = ...->GetMethodById(...->GetMethod(), call_inst->GetCallMethodId());
        // Move immediate-value to parameter
        GetEncoder()->EncodeMov(param_0, Imm(method));
    }
    // Get offset of 
    size_t entry_point_offset = ...->GetCompiledEntryPointOffset();
    // Major call instruction
    GetEncoder()->MakeCall(MemRef(param_0, entry_point_offset));
    ... 
    GetEncoder()->LoadCallerRegisters(dst_reg);
}

class EncodeVisitor : public GraphVisitor {
   /* VisitAdd, VisitSub, VisitMul etc... */
   VisitInstruction(inst) {
      switch (inst->Opc()) {
        case (Opcode::Add) VisitAdd(inst); break;
        case (Opcode::Cmp) VisitCmp(inst); break;
        case (Opcode::Call) VisitCallStatic(inst); break;
        ...
      }
   }

   // Full logic for generate one instruction:
   void VisitAdd(inst) {
        auto dst = GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
        auto src0 = GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
        enc->GetEncoder()->EncodeAdd(dst, src0);
   }

    void EncodeVisitor::VisitCallStatic(inst) {
        GetCodegen()->CreateCall(inst);
    }

    void EncodeVisitor::VisitCmp(GraphVisitor* visitor, Inst* inst) {
        auto* enc = static_cast<EncodeVisitor*>(visitor);
        auto cmp_inst = inst->CastToCmp();
        auto dst = GetCodegen()->ConvertRegister(inst->GetDstReg());
        auto src0 = GetCodegen()->ConvertRegister(inst->GetSrcReg(0));
        auto src1 = GetCodegen()->ConvertRegister(inst->GetSrcReg(1));
        Condition cc = ...
        GetEncoder()->EncodeCmp(dst, src0, src1, cc);
    }

}
```

And them will have different encoding for each architecture:
```
target/aarch32/encode.cpp:
void Aarch32Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc) {
    CompareHelper(src0, src1, &cc); // Method for calculate flags for each src1 and src2 types
    __ Mov(Convert(cc), VixlReg(dst), 0x1);
    __ Mov(Convert(cc).Negate(), VixlReg(dst), 0x0);
}

target/aarch64/encode.cpp:
void Aarch64Encoder::EncodeCompare(Reg dst, Reg src0, Reg src1, Condition cc) {
    __ Cmp(VixlReg(src0), VixlReg(src1));
    __ Cset(VixlReg(dst), Convert(cc));
}
```

## Links

[codegen.cpp](../optimizer/code_generator/codegen.cpp)  
[codegen.h](../optimizer/code_generator/codegen.h)  
[slow_path.cpp](../optimizer/code_generator/slow_path.cpp)  
[slow_path.h](../optimizer/code_generator/slow_path.h)  

Tests:  
[codegen_test_1.cpp](../tests/codegen/codegen_test_1.cpp), [codegen_test_2.cpp](../tests/codegen/codegen_test_2.cpp), [inst_generator_test.cpp](../tests/inst_generator_test.cpp) - test with generation each instruction.
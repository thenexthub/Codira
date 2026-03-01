# Encoder library

## Building

Current realization is not building outside compiler.

## Sub-modules description

There are two major variable-parts - architecture and execution model. Current architecture has aim to make easy switching of this two directions.

### Register file  (concrete implementation is architecture dependent)
RegistersDescription - is class for get access for all possible information about registers, which are used in the target architecture: amount of available registers, theirs type and size. Description for special registers and temps. It also may holds low-level information about callee and caller-saved registers. Also it is useful to have conversation from abstract register to arch-special registers(for example for vixlReg).

This file must not use other interfaces.

### Encoder  (concrete implementation is architecture dependent)
Encoder  - is class for emit main list of instructions - math and memory-usage.  

This class uses RegistersDescription.

### Calling Convention   (concrete implementation is architecture dependent)
CallingConvention - is class for control calls/jumps, for spill/fill registers and access for native parameters. That's why it contains Labels list for have possibility to know concrete address for special branch-target.

This class uses Encoder and RegistersDescription classes.

Current stack layout must be like next one:
```
----------+----------------------+
 Caller   | Default calling convention frame:
 Frame    | Caller saved + parameters, if they
          | don't fit in registers
          |
----------+-Encoder-Frame-Start--+   For CallingConvention::Begin(FUNCTION):
  --------+----------------------+   (CallConv::PushHeader) {
   Pre    |         lr           |             |
   Header |         fp           |             |
          |     1-st param       |  - (method) |
          |   allignment reg     | (push one reg for padding)
  --------+----------------------+   } // (CallConv::PushHeader)
  --------+----------------------+   (CallConv::PushRegs(GetCalleeSavedR + V)) {
   Header | scalar registers     |             |
   callee | + optional allign-reg|             |
          | vector registers     |             |
          | + optional allign-reg|             |
  --------+----------------------+   } // (CallConv::PushRegs(GetCalleeSavedR + V))
  --------+----------------------+   CallConv::IncrementStack(SIZE_IN_BYTES)
          |                      |
          |                      |    Memory, avaliable for user
          |                      |
  --------+----------------------+   CallingConvention::Begin(NATIVE):
  --------+----------------------+   (CallConv::PushRegs(GetCallerSavedR + V)) {
   Header | scalar registers     |             |
   caller | + optional allign-reg|             |
          | vector registers     |             |
          | + optional allign-reg|             |
  --------+----------------------+   } // (CallConv::PushRegs(GetCallerSavedR + V))
   Param  | Here will be         |
          | parameters, if they  |  Must be manually filled
          | don't fit in regs    |
          |                      |
----------+-Encoder-Frame-End------+
----------+------------------------+
 Native   | Default calling convention frame:
 Frame    | Callee saved ... e.t.c

```


### Execution model  (concrete implementation - must be architecture independent)
ExecModel  - is class for control emitting instructions. It must to be able to create default headers for emitted code and implement execution-specific code, e.g interpreter-ExecState access values.

This class uses all above classes.

### Codegen  (concrete implementation - must be architecture independent)
Codegen - must not be in Encode-library, not to make dependency from compiler. It also must not to make dependency from special architecture.

This class also must use all of above classes and have possibility to switch between arc or models.

## Operands

### TypeInfo

Class **TypeInfo** contains information about supported types: BOOL, INT8, INT16, INT32, INT64, FLOAT32, FLOAT64  
You can get next information about type: size, scalar or vector.  
Example:  
```
    auto type = TypeInfo(FLOAT32); // TypeInfo for float type  
    ASSERT(type->GetSize() == 32 && type->IsFloat());
```

### Register

Class **Reg** contains number of register(id) and **TypeInfo**.     
You can get next information about register: type, id, size, scalar or vector.  

Example:  
```
    auto reg = Reg(0, TypeInfo(INT32)); // scalar word regster   
    ASSERT(reg->GetId() == 0 && reg->GetType() == TypeInfo(INT32) && reg->GetSize() == 32 && reg->IsScalar());
```

### Immediate

Class **Imm** contains 64-bit storage for the immediate, it doesn't contain any information about it's type. It is intended to store both float and integer types.

A user is responsible for correct type deduction. Commonly, the type can be deduced by a register that accompanies the immediate in an instruction.

Example:  
```
    EncodeInst(Reg src, Imm imm)
    {
        if (src.GetType() == FLOAT32_TYPE) {
            float val = imm.GetAsFloat();
            ...
        } else if (src.GetType() == FLOAT64_TYPE) {
            double val = imm.GetAsDouble();
            ...
        } else {
            int64_t val = imm.GetAsInt();
            ...
        }
    }
```

### Memory

Class **MemRef** contains base **Reg**, index **Reg**, scale **Imm** and disp **Imm**.  
The memory address is calculated using the following formula: `base + (index << scale) + disp`  
If a parameter is not defined, it is considered equal to 0. The base must be defined. 

Example:  
```
    // memory with base register and disp
    auto base_reg = Reg(5, TypeInfo(INT64));
    auto disp = Imm(static_cast<int642>(16));
    auto mem_disp = MemRef(base_reg, disp); // base_reg + disp
    ASSERT(mem_disp.HasBase() && !mem_disp.HasIndex() && !mem_disp.HasScale() && mem_disp.HasDisp());
    ASSERT(mem_disp.GetBase() == base_reg && mem_disp.GetDisp() == disp);
    ASSERT(mem_disp.GetIndex() == INVALID_REGISTER && mem_disp.GetScale() == 0);

    // memory with base , index registers and scale
    auto base_reg = Reg(5, TypeInfo(INT64));
    auto index_reg = Reg(6, TypeInfo(INT64));
    auto scale = Imm(static_cast<int32>(3));
    auto mem_scale = MemRef(base_reg, index_reg, scale); // base_reg + (index_reg << scale)
    ASSERT(mem_scale.HasBase() && mem_scale.HasIndex() && mem_scale.HasScale() && !mem_scale.HasDisp());
    ASSERT(mem_scale.GetBase() == base_reg && mem_scale.GetIndex() == index_reg && mem_scale.GetScale() == scale);
    ASSERT(mem_scale.GetDisp() == 0);
```

## Code Example (WIP)

```
// Example of usage encode-library
/* == Sequence for create and configure encoder ==*/
// 1. Create encoder for special-architecture
Encoder *enc= buildEncoder(Arch);
// 2. Create execution model - must be target independent
ExecModel* exec = buildExecutionModel(Model::JIT);
// 3. Create Calling convention-model - architecture-dependent
CallingConvention *callconv = buildCallingConvention(exec, Arch);
// 4. Fill Encoder
enc->Configure(exec, callconv);
/*======= Here encoder is fully configured =======*/
// Usage:


// Method start header
callconv->Begin(CallingConvention::Reason::StartMethod);
callconv->CallerStart(CallingConvention::Reason::StartMethod); // (?)

// Use encoding:
// Get register information
auto register_description = enc->GetRegisterDescription();
// Get special registers data
auto sp = register_description->GetSpecialRegister(SpecialReg::stack_pointer);
ASSERT(sp.IsValid());
// Get all un-reserved register list
auto available_regs = register_description->GetRegAllocatable()

// Build memory operand
auto memory = enc->BuildMem(sp, callconv->GetParamOffset(2));

auto tmp1 = available_regs[0];
enc->EncodeLoad(tmp1, memory);

auto tmp2 = available_regs[1];
enc->EncodeMov(tmp2, Imm(0x123));

enc->EncodeAdd(tmp1, tmp2, tmp1);

auto label = callconv->CreateLabel();
// or CreateLabel<type>

callconv->EncodeJump(label, CallingConvention::Reason::Jump);

callconv->BindLabel(label);
enc->EncodeAdd(tmp1, tmp2, tmp1);

enc->EncodeStore(tmp1, memory);
callconv->EncodeBranch(label);

callconv->CallerEnd(CallingConvention::Reason::ExitMethod); // (?)
callconv->EncodeReturn(CallingConvention::Reason::ExitMethod);
// End of method
callconv->End(CallingConvention::Reason::EndMethod);

enc->Finalize();
auto code = enc->GetEncodeData();
code_allocator->Allocate(code.start(), code.size());
///Example from encoder-part

void Aarch64Encoder::EncodeAShr(Reg dst, Reg src0, Reg src1) {
if (dst.GetSize() < MAX_REG_SIZE) {
__ And(VixlReg(src1), (VixlReg(src1), VixlImm(dsr.GetSize() - 1));
}
__ Asr(VixlReg(dst), VixlReg(src0), VixlReg(src1));
}
```

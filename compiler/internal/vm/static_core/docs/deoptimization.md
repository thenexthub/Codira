# Deoptimization

## Overview

Deoptimization is the process of taking action if compiled code cannot continue execution for some reason.

The compiler may make some speculative assumptions about the execution of the method. Based on these assumptions, the compiler makes certain optimizations, inserting checks about the correctness of the assumptions. The special instruction `DeoptimizeIf` processes the result of the check and if the result is incorrect the output to the interpreter occurs.

## Speculative optimizations  

 - [Devirtualization with Class Hierarchy Analysis](../compiler/docs/inlining.md#devirtualization-with-class-hierarchy-analysis)
 - BoundCheck Elimination

## Algorithm

### DeoptimizeIf
The instruction `DeoptimizeIf` is encoded as conditional branch to the slow path if input value is true.  
Details in [codegen.cpp](../compiler/optimizer/code_generator/codegen.cpp), function EncodeVisitor::VisitDeoptimizeIf

### Slow path

The slow path encodes a call to the runtime function `Deoptimize`. We do the following for this:
1. fill the `Frame`(saves pointer to current CFrame) and `is_compiled_frame_`(saves true) fields in the thread register. 
2. save caller saved registers
3. create StateStamp(in compile time) for this address(np - native pc)
4. Call `Deoptimize` function

Details in [slow_path.cpp](../compiler/optimizer/code_generator/slow_path.cpp) case RuntimeInterface::EntrypointId::DEOPTIMIZE

### The function Deoptimize

The function `Deoptimize` calculates bitecode pc where we should start executing code in the interpreter, restores interpreter frame from current CFrame.  
If deoptimization occurred in the inlined method, we restore all interpreter frames for all inlined methods and calculate the number of inlined methods.

pseudocode:

```
voud Deoptimize(...) {
    uint32_t num_inlined_methods = 0;
    Frame* iframe = frame.ConvertToIFrame(&prev_frame_kind, &num_inlined_methods);
    Frame* last_iframe = iframe;
    while (num_inlined_methods-- != 0) {
        last_iframe = last_iframe->GetPrevFrame();
    }
    if (IsIframe(last_iframe->GetPrevFrame())) {
        DeoptimizeAfterIFrame(thread, pc, iframe, cframe, last_iframe);
    } else {
        DeoptimizeAfterCFrame(thread, pc, iframe, cframe, last_iframe);
    }
}

Frame* StackWalker::ConvertToIFrame(CFrameType& cframe, uint32_t* num_inlined_methods) {
    void* prev_frame;
    if (cframe.IsInlined()) {
        auto caller_cframe = cframe.GetCallerFrame();
        *num_inlined_methods = *num_inlined_methods + 1;
        prev_frame = ConvertToIFrame(caller_cframe, num_inlined_methods);
    } else {
        prev_frame = GetPrevFrame(cframe);
    }
    Frame* frame = CreateFrameAndSetsVRegs(cframe, prev_frame);
    frame->SetBytecodeOffset(cframe.GetBytecodePc());
```

If the deoptimized method was called from interpreter, `Deoptimize` calls the bridge `DeoptimizeAfterIFrame`, otherwise `DeoptimizeAfterCFrame`. 

Both bridges have following parameters:
* pointer to the current thread
* bytecode pc of the entry
* pointer to first restoring interpreter Frame
* pointer to CFrame origin
* pointer to last restoring interpreter Frame

Details in [deoptimization.cpp](../runtime/deoptimization.cpp)

### DeoptimizeAfterIFrame

`DeoptimizeAfterIFrame` restores SP, FP and LR to values before calling the method, restores callee saved registers (which we saved in the method) and call `InvokeInterpreter` to execute the method in the interpreter. Since we changed return address (LR), we return from the function to the place in interpreter where the method was called.

Details in [deoptimization_aarch64.S](../runtime/bridge/arch/aarch64/deoptimization_aarch64.S), [deoptimization_arm.S](../runtime/bridge/arch/arm/deoptimization_arm.S), [deoptimization_amd64.S](../runtime/bridge/arch/amd64/deoptimization_amd64.S)

### DeoptimizeAfterCFrame

`DeoptimizeAfterCFrame` changes current CFrame to I2C(Interpreter To Compile) bridge, set last IFrame's previous frame to this C2I bridge frame and call `InvokeInterpreter` for execute the method in the interpreter. After return from `InvokeInterpreter`, we restore callee saved registers(which we saved in the method), restore SP, FP and LR to values before calling the method and return to compiled code where the method was called.

Details in [deoptimization_aarch64.S](../runtime/bridge/arch/aarch64/deoptimization_aarch64.S), [deoptimization_arm.S](../runtime/bridge/arch/arm/deoptimization_arm.S), [deoptimization_amd64.S](../runtime/bridge/arch/amd64/deoptimization_amd64.S)

### InvokeInterpreter

`InvokeInterpreter` has next parameters:
* pointer to current thread  
* bytecode pc of the entry  
* pointer to first restoring interpreter Frame  
* pointer to last restoring interpreter Frame  

`InvokeInterpreter` change `Frame`(saves pointer to interpreter Frame) and `is_compiled_frame_`(saves false) fields in the thread register and calls interpreter from bytecode pc.
If deoptimization occurred in the inlined method, we call interpreter for all inlined methods from bytecode pc which is taken from the corresponding interpreter frame.
The last restoring interpreter Frame is used for limitation number of calls to the interpreter for inlined methods. 

pseudocode:

```
int64_t InvokeInterpreter(ManagedThread* thread, const uint8_t* pc, Frame* frame, Frame* last_frame) {
    thread->SetCurrentFrame(frame);
    thread->SetCurrentFrameIsCompiled(false);

    interpreter::Execute(thread, pc, frame, thread->HasPendingException());

    auto acc = frame->GetAcc();
    auto prev_frame = frame->GetPrevFrame();
    thread->SetCurrentFrame(prev_frame);
    FreeFrame(frame);

    while (prev_frame != nullptr && (last_frame != frame)) {
        frame = prev_frame;
        prev_frame = frame->GetPrevFrame();
        pc = GetPcFromFrame(frame);
        frame->GetAcc() = acc;
        interpreter::Execute(thread, pc, frame, thread->HasPendingException());

        acc = frame->GetAcc();

        thread->SetCurrentFrame(prev_frame);
        FreeFrame(frame);
    }

    return GetValue(acc);
}
```

Details in [bridge.cpp](runtime/bridge/bridge.cpp)

## Tests

### deoptimization after wrong devirtualization:

[inline_cha.pa](../tests/checked/inline_cha.pa)

### deoptimization after BoundCheck Elimination:

[deopt_true_test.pa](../tests/regression/deopt_true_test.pa)   
[deopt_true_call_test.pa](../tests/regression/deopt_true_call_test.pa)  


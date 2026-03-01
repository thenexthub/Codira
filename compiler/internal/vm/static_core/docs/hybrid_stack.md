# ArkTS 1.2 crash with hybrid stack


## 1.2 crash stack
In 1.2 application, the crash often happens in cpp code. When a crash occurs in C++ code, we will obtain a native C++ crash stack. For example: 
```
...
ProcessMemory(kB):...
Device Memory(kB):...
Reason: ...
Fault thread info:
TID:..., Name:...
#00 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#01 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#02 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#03 pc 0000000000XXXXXX /system/lib64/platformsdk/libace_compatible.z.so
...
```
The main reasons of crash are fatal checks and abnormal pointers.
#### 1. Caused by fatal check reason
If a crash is caused by fatal reason. Usually it happened when running a wrong branch or exception scenarios. Additional error information will appear in the crash log in `LastFatalMessage`.

For example:
```
...
Reason:Signal:SIGABRT(SI_TKILL)...
LastFatalMessage:runtim:Invalid class state transition XXX -> XXX;
Fault thread info:
Tid:..., Name:...
...
```
We can get the error reason and more info in `LastFatalMessage` field. Like `class state` in the example.

And can find the error location in the code by fatal info. In the example, the error code is in `arkcompiler/runtime_core/static_core/runtime/class.cpp` file, method `SetState` (`void Class::SetState(Class::State state)`).

```
void Class::SetState(Class::State state)
{
    if (...) {
        LOG(FATAL, RUNTIME) << "Invalid class state transition " << state_ << " -> " << state;
    }

    state_ = state;
}
```

#### 2. Invalid pointer
Another common type of crash usually caused by invalid pointer, for example:
```
···
Reason:Signal:SIGSEGV(SEGV_MAPERR)@0x0000000000000044 probably caused by NULL pointer dereference
Fault thread info:
Tid:..., Name:...
···
```
This is usually caused by a memory issue, and no extra fatal info. We can get info in log, or we can disassemble the so and find the crash place by pc in the top frame of the stack.

### Disassemble so to obtain the code location
The top frame of the stack usually in a specially so file. and we can get the crash location by the info from stack and disassemble the so file.

In the example, the top frame of the stack is in `libarkruntime.so`, so we can disassemble this shared library first:

```
aarch64-linux-gnu-objdump -S -d -l libarkruntime.so > libarkruntime.so.dump
```

We can use `aarch64-linux-gnu-objdump` or other objdump tools.

And we can see the disassemble result in `libarkruntime.so.dump`.

For example:
```
...
0000000000XXXXXX <_ZN3ark5Class8SetStateENS0_5StateE>:
_ZN3ark5Class8SetStateENS0_5StateE():
  1cXXXX:    d105XXXX    sub   sp, sp, #x170
  1cXXXX:    a913XXXX    stp   x29, x30, [sp, #304]
...
```

In the dumped file, the first column is PC value. Then find the assembly info and method name by the PC value in stack top.

### ets/js language stack
The current 1.2 crash stack only provides the native call stack. Therefore, the ETS call stack is not included in crash logs. And when having interop with 1.1, there will be no js stack in the crash log. So if we want get ets/js language stack, we should use other interfaces, as shown below.

## Get 1.2 ets stack
The ets stack info in 1.2 is store in the stack in each thread, and we can get it by `StackWalker`.

`StackWalker` can create by the thread (`StackWalker::Create(ManagedThread::GetCurrent())`).
And `StackWalker` has a function for Dump: `void StackWalker::Dump(std::ostream &os, bool print_vregs /* = false */)` .

The function can be called in cpp runtime code for printing ets call stack. And we can choose if we should print vregs by `print_vregs` parameter (`false` by default).

For Example:
```
    auto thread = ManagedThread::GetCurrent();
    auto stack = StackWalker::Create(thread);
    std::stringstream ss;
    stack.Dump(ss);
```

Output Example:

```
Panda call stack:
   0: 0x7f000XXXX in std.core.Array::slice (managed)
   1: 0x7f000XXXX in std.core.Array::shift (managed)
   2: 0x7f000XXXX in std.concurrency.TimerTable::lambda_invoke-1 (managed)
   3: 0x7f000XXXX in std.concurrency.%%lambda-lambda_invoke-1::$_invoke (managed)
   4: 0x7f000XXXX in std.concurrency.ConcurrencyHelpers::spinlockGuard (managed)
   5: 0x7f000XXXX in std.concurrency.TimerTable::removeClearedTimers (managed)
...
```

Insert this code at the desired location, and use StackWalker to obtain the 1.2 ETS stack. A complete example is shown below and can be used directly by developers.

#### Usage Example:

##### 1. Add StackWalker code in C++

Insert the following snippet into any C++ runtime location where you want to inspect the ETS call stack.
For example, add it inside `HeapManager::AllocateObject` after the object header has been initialized:

```
auto *kls = object->ClassAddr<Class>();
LOG(ERROR, RUNTIME) << "Alloc object at " << mem
                    << " size: " << size
                    << " cls: " << kls->GetName();

// Dump ETS stack
std::ostringstream osstr;
StackWalker::Create(thread, UnwindPolicy::ALL).Dump(osstr, false);

// Print to log
LOG(ERROR, RUNTIME) << "ETS Stack:\n" << osstr.str();
```
Placing this code at any point in the runtime will print the ETS call stack for the current managed thread.

##### 2. Build the updated runtime so

Pull the OpenHarmony source code and build so.

##### 2. Create a simple ETS test to trigger the stack dump
Create an ETS file with the following minimal test:

```
function allocOnly() {
    let m = new Map<number, number>();
}

export function runAllocTest() {
    allocOnly();
}

runAllocTest();
```
Calling new Map() triggers object allocation, which executes the modified AllocateObject() and prints the ETS stack.

##### 3. Run the application and check logs

After building and running the application in DevEco Studio, you can check the hilog output, which will show something similar to:
```
E/runtime: Alloc object at 0xc5070 size: 16 cls: std.core.Tuple2
E/runtime: ETS Stack:
Panda call stack:
   0: 0x71defa7c0300 in std.core.Map::allocDataAndBuckets (managed)
   1: 0x71defa7c0200 in std.core.Map::<ctor> (managed)
   2: 0x71defa7c0080 in test.ETSGLOBAL::allocOnly (managed)
   3: 0x71defa7c0000 in test.ETSGLOBAL::runAllocTest (managed)
   ...
```
The stack clearly shows:
- Builtin calls (std.core.Map::<ctor>, etc.)
- The developer’s own ETS functions (allocOnly, runAllocTest)
This confirms that StackWalker successfully prints the 1.2 ETS call stack.

## Get 1.1 js stack in interop scenarios

When run the pure 1.2 application, there is no 1.1 js stack in the application. So it does not involve the js call stack.

When running an interop application, JS stack information is not included in crash logs, so we need to use the DFX mechanism in 1.1 to retrieve it.

`BuildJsStackTrace` is a method in `DFXJSNapi`: `bool DFXJSNapi::BuildJsStackTrace(const EcmaVM *vm, std::string &stackTraceStr)`

This function can get the js stack info by current vm. We should first obtain the current VM and then retrieve the stack string using `BuildJsStackTrace`.

For Example:
```
  auto engine = reinterpret_cast<NativeEngine*>(env);
  auto vm = engine->GetEcmaVm();
  std::string stack;
  DFXJSNapi::BuildJsStackTrace(vm, stack);
```

Output Example:

```
  at anonymous (entry|entry|1.0.0|src/main/ets/workers/workerNewEnvTest.ts:15:12)
  at add (entry|entry|1.0.0|src/main/ets/workers/workerNewEnvTest.ts:21:16)
```

Add the code in the place you want, and use the method `BuildJsStackTrace` get the js stack in 1.1 vm. A complete example is shown below and can be used directly by developers.

#### Usage Example:

##### 1. Modify the Runtime to Invoke BuildJsStackTrace

Insert the following code to collect the JS call stack when an unhandled JS exception is triggered in the place you want. For example, add the code in function `void NapiUncaughtExceptionCallback::CallbackTask(napi_value &obj)`:
```
auto engine = reinterpret_cast<NativeEngine*>(env_);
std::string stack;
engine->BuildJsStackTrace(stack);
```
This retrieves the full JS stack trace into the stack string whenever a JS exception is caught by the NAPI callback.

##### 2.Build the Updated Shared Library

Rebuild the corresponding shared library and push shared library (.so file).

##### 3. Deploy the Shared Library to the Device
Push the newly built shared library:
```
hdc target mount
hdc file send xxx.so /path_to_so_in_phone
hdc shell reboot
```

##### 4.Create an Interop Test Case
You can verify the JS stack retrieval by writing a simple interop example in the 1.1 module.

Example file structure:
```
library/
 └─ src/main/ets/components/
      ├─ file1.ts
      ├─ file2.ts
      └─ MainPage.ets
```
**file2.ts**
```
export function function2() {
  console.log(new Error().stack);
  return true;
}
```
**file1.ts**
```
import { function2 } from './file2';

export function function1() {
  function2();
  return true;
}
```
**MainPage.ets**
```
export function main() {
  function1();
  return true;
}
```
Calling new Error().stack triggers the stack trace generation, which is intercepted by the modified runtime.

##### 5. Run the Demo and Check the Log Output

After launching the application and clicking the test button, the JS call stack will appear in hilog output.
An example log looks like:
```
at function2 library (library/src/main/ets/components/file2.ts:2:15)
at function1 library (library/src/main/ets/components/file1.ts:4:3)
at main library (library/src/main/ets/components/MainPage.ets:27:3)
```

This demonstrates a complete 1.1 JS stack for the interop scenario.
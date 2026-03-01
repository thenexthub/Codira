# String Builder optimizations

## Overview

This set of optimizations targets String Builder usage specifics.
All the optimizations described below preserve original behavior of String/StringBuilder operations and their side effects. This is tested by the wide set of tests (see Links Section below).

## Rationality

String Builder is used to construct a string out of smaller pieces. In some cases it is optimal to use String Builder to collect intermediate parts, but in other cases we prefer naive string concatenation, due to an overhead introduced by String Builder object.

## Dependence

### Dependence on analysis

* BoundsAnalysis
* AliasAnalysis
* LoopAnalysis
* DominatorsTree

### Dependence on stdlib

Optimizations depends on the following stdlib API classes and their methods:

* ```std.core.StringBuilder```
  * ```<ctor>```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```append```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```toString```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```length```, field accessed by IR LoadObject when possible in ```AOT, JIT```
  * ```stringLength```, may emit this call instead of length field access in ```BCO```
* ```std.core.String```
  * ```length```, may replace with StringBuilder stringLength() in ```BCO``` or length in ```AOT, JIT```
  * ```getLength```, may replace with StringBuilder stringLength() in ```BCO```or length in ```AOT, JIT```

### Dependence on compiler intrinsics

Optimizations depends on the following compiler intrinsics:

* ```Intrinsic```
  * ```StdCoreSbAppendString```, may optimize out or emit in ```AOT, JIT```
  * ```StdCoreSbAppendString2```, may emit in ```AOT, JIT```
  * ```StdCoreSbAppendString3```, may emit in ```AOT, JIT```
  * ```StdCoreSbAppendString4```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat2```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat3```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat4```, may emit in ```AOT, JIT```

## Algorithms

**Remove unnecessary String Builder**

Example:

```TS
let input: String = ...
let sb = new StringBuilder(input)
let output = sb.toString()
```

Since there are no `StringBuilder::append()`-calls in between `constructor` and `toString()`-call, `input` string is equal to `output` string. So, the example code is equivalent to

```TS
let input: String = ...
let output = input
```

**Replace String Builder with string concatenation**

String concatenation expressed as a plus-operator over string operands turned into a String Builder usage by a frontend.

Example:

```TS
let a, b: String
...
let output = a + b
```

Frontend output equivalent:

```TS
let a, b: String
...
let sb = new StringBuilder(a)
sb.append(b)
let output = sb.toString()
```

The overhead of String Builder object exceeds the benefits of its usage (comparing to a naive string concatenation) for a small number of operands (two in the example above). So, we replace String Builder in such a cases back to naive string concatenation.

**Optimize concatenation loops**

Consider a string accumulation loop example:

```TS
let input: string = "input,"
let output = "output:"
for (let i = 0; i < 10; i++)
    output += input
```

Like in **Replace String Builder with string concatenation** section, frontend replaces string accumulation `output += input` with a String Builder usage, resulting in a huge performance degradation (comparing to a naive string concatenation), because *at each loop iteration* String Builder object is constructed and the first operand is appended, then tne second one is appended, builds resulting string, and discarded.

The equivalent code looks like the following:

```TS
let input: string = "input,"
let output = "output:"
for (let i = 0; i < 10; i++) {
    let sb = new StringBuilder(output)
    sb.append(input)
    output = sb.toString()
}
```

To optimize cases like this, we implement the following equivalent transformation:

```TS
let input: string = "input,"
let output = "output:"
let sb = new StringBuilder(output)
for (let i = 0; i < 10; i++) {
    sb.append(input)
}
let output = sb.toString()
```

**Merge StringBuilder::append calls chain**

Consider a code sample like the following (default constructor is used):

```TS
// String semantics
let result = str0 + str1

// StringBuilder semantics
let sb = new StringBuilder()
sb.append(str0)
sb.append(str1)
let result = sb.toString()
```

Here we call `StringBuilder::append` twice. Proposed algorith merges them into a single call to (in this case) `StringBuilder::append2`. Merging up to 4 consecutive calls supported.

Optimized example is equivalent to:

```TS
// StringBuilder semantics
let sb = new StringBuilder()
sb.append2(str0, str1)
let result = sb.toString()
```

Consider a code sample like the following (constructor with a string argument is used):

```TS
// String semantics
let result = str0 + str1 + str2

// StringBuilder semantics
let sb = new StringBuilder(str0)
sb.append(str1)
sb.append(str2)
let result = sb.toString()
```

Here, we call `StringBuilder::append` twice after calling the constructor that appends the first string. The proposed algorithm combines the last two calls into a single call (in this case, `StringBuilder::append2`). Merging up to 4 consecutive calls supported.

Optimized example is equivalent to:

```TS
// StringBuilder semantics
let sb = new StringBuilder(str0)
sb.append2(str1, str2)
let result = sb.toString()
```

**Merge StringBuilder objects chain**

Consider a code sample like the following:

```TS
// String semantics
let result0 = str00 + str01
let result = result0 + str10 + str11

// StringBuilder semantics
let sb0 = new StringBuilder(str00)
sb0.append(str01)
let result0 = sb0.toString()

let sb1 = new StringBuilder(result0)
sb1.append(str10)
sb1.append(str11)
let result = sb1.toString()
```

Here we construct `result0` and `sb0` and use it only once as a first argument of the concatenation which comes next. As we can see, two `StringBuilder` objects created. Instead, we can use only one of them as follows:

```TS
// StringBuilder semantics
let sb0 = new StringBuilder(str00)
sb0.append(str01)
sb0.append(str10)
sb0.append(str11)
let result = sb0.toString()
```

Proposed algorithm merges consecutive chain of `StringBuilder` objects into a single object (if possible).

**Replace String length accessors with StringBuilder versions**

While performing above optimizations calls to std.core.String.length or getLength() will be replaced with StringBuilder.length field access or StringBuilder.stringLength() method.
Consider a code sample like the following:

```TS
    let input: String = ...
    let output: String = "";
    for (let i = 0; i < n; ++i) {
        output += output.length; // or += output.getLength();
        output += input;
    }
```

Equivalent code after transformation will be the following:

```TS
    let input: String = ...
    let sb = new StringBuilder()
    for (let i = 0; i < n; ++i) {
        sb.append(sb.stringLength()); // or sb.length in AOT, JIT
        sb.append(input);
    }
    let output = sb.toString()
```

**Remove extra call for stringLength**

Example:
```TS
let sb = new StringBuilder()
let len0 = sb.stringLength;
let len1 = sb.stringLength;
```
Since there are no `StringBuilder::append()`-calls or any call modifying the StringBuilder object in between `stringLength`  calls, `len0` value is equal to `len1` value. So, the example code is equivalent to
```TS
let sb = new StringBuilder()
let len0 = sb.stringLength;
let len1 = len0;
```

## Pseudocode

**Complete algorithm**

```C#
function SimplifyStringBuilder(graph: Graph)
    foreach loop in graph
        OptimizeStringConcatenation(loop)

    OptimizeStringBuilderChain()

    foreach block in graph (in RPO)
        OptimizeStringBuilderToString(block)
        OptimizeStringConcatenation(block)
        OptimizeStringBuilderAppendChain(block)
        OptimizeStringBuilderStringLength(block)
```

Below we describe the algorithm in more details

**Remove unnecessary String Builder**

The algorithm works as follows: first we search for all the StringBuilder instances in a basic block, then we replace all their toString-call usages with instance constructor argument until we meet any other usage in RPO.

```C#
function OptimizeStringBuilderToString(block: BasicBlock)
    foreach ctor being StringBuilder constructor with String argument in block
        let instance be StringBuilder instance of ctor-call
        let arg be ctor-call string argument
        foreach usage of instance (in RPO)
            if usage is toString-call
                replace usage with arg
            else
                break
        if instance is not used
            remove ctor from block
            remove instance from block
```

**Replace String Builder with string concatenation**

This algorithm is designed for use with default and a string-argument constructors and works as follows:
- In the case of default constructor first we search for all the StringBuilder instances in a basic block, then we check if the use of instance matches concatenation pattern for 2, 3, or 4 arguments. If so, we replace the whole use of StringBuilder object with concatenation intrinsics;
- In the case of a constructor with a string argument, we do the same actions as for default one, but, since the first data addition operation is performed by the constructor itself during the call, we check if the use of instance matches concatenation pattern for 1, 2, or 3 arguments. If so, we construct a fake append instruction instead of the constructor and put the new instruction at the top of the list of the other append instructions, then replace the whole use of StringBuilder object with concatenation intrinsics.

```C#
function OptimizeStringConcatenation(block: BasicBlock)
    foreach ctor being StringBuilder default constructor in block
        let instance be StringBuilder instance of ctor-call
        let match = MatchConcatenation(instance)
        let appendCount = match.appendCount be number of append-calls of instance
        let append = match.append be an array of append-calls of instance
        let toStringCall = match.toStringCall be toString-call of instance
        if ctor is constructor with string argument
            create the new append instruction
            put it at the top of the append list,before append[0]
        let concat01 = ConcatIntrinsic(append[0].input(1), append[1].input(1))
        remove append[0] from block
        remove append[1] from block
        switch appendCount
            case 2
                replace toStringCall with concat01
                break
            case 3
                let concat012 = ConcatIntrinsic(concat01, append[2].input(1))
                remove append[2] from block
                replace toStringCall with concat012
                break
            case 4
                let concat23 = ConcatIntrinsic(append[2].input(1), append[3].input(1))
                let concat0123 = ConcatIntrinsic(concat01, concat23)
                remove append[2] from block
                remove append[3] from block
                replace toStringCall with concat0123
        remove toStringCall from block
        remove ctor from block
        remove instance from block

function ConcatIntrinsic(arg0, arg1): IntrinsicInst
    return concatenation intrinsic for arg0 and arg1

type Match
    toStringCall: CallInst
    appendCount: Integer
    append: array of CallInst

function MatchConcatenation(instance: StringBuilder): Match
    let match: Match
    foreach usage of instance
        if usage is toString-call
            set match.toStringCall = usage
        elif usage is append-call
            add usage to match.append array
            increment match.appendCount
    return match
```

**Optimize concatenation loops**

This algorithm is designed for use with default and string-argument constructors and works as follows:
- All loops are processed, starting from the inner loops and ending with the outer loops;
- For each processed loop string concatenation patterns are searched;
- For each pattern found StringBuilder usage instructions are reconnected in a correct way (making them point to the only one instance we have chosen); 
- The object creation instruction of chosen String Builder and, in the case of default constructor, initial string value appending instruction, are moved to a loop pre-header;
- The toString call instruction of chosen StringBuilder is moved to loop exit block;
- All unused instructions are cleaned at the end of process during the cleanup pass.

```C#
function OptimizeStringConcatenation(loop: Loop)
    foreach innerLoop being inner loop of loop
        OptimizeStringConcatenation(innerLoop)
    let matches = MatchConcatenationLoop(loop)
    foreach match in matches)
        ReconnectInstructions(match)
        HoistInstructionsToPreHeader(match)
        HoistInstructionsToExitBlock(match)
    }
    Cleanup(loop, matches)

type Match
    accValue: PhiInst
    initialValue: Inst // e.g.,a LoadString instruction
    // instructions to be hoisted to preheader
    preheader: type
        instance: Inst // a NewObject instruction
        appendAccValue: Inst // a first append(or append intrinsic) instruction or a constructor with a string argument
    // instructions to be left inside loop
    loop: type
        append: array of Inst // other append instructions
    // instructions to be deleted
    temp: array of type
        toStringCall: Inst
        instance: Inst
        appendAccValue: Inst
    // instructions to be hoisted to exit block
    exit: type
        toStringCall: Inst

function MatchConcatenationLoop(loop: Loop)
    let matches: array of Match
    foreach accValue being string accumulator in a loop
        let match: Match
        foreach instance being StringBuilder instance used to update accValue (in RPO)
            if match is empty
                // Fill preheader and exit parts of a match
                set match.accValue = accValue
                set match.initialValue = FindInitialValue(accValue)
                set match.exit.toStringCall = FindToStringCall(instance)
                set match.preheader.instance = instance
                set match.preheader.appendAccValue = FindAppend(instance, accValue) ||
                    FindCtorStr(instance, accValue)
                // Init loop part of a match
                add other append to match.loop.append array
            else
                // Fill loop and temporary parts of a match
                let temp: TemporaryInstructions
                set temp.instance = instance
                set temp.toStringCall = FindToStringCall(instance)
                foreach append in FindAppend(instance)
                    if append.input(1) is accValue
                        set temp.appendAccValue = append
                    else
                        add append to match.loop.append array
                add temp to match.temp array
        add match to matches array
    return matches

function ReconnectInstructions(match: Match)
    match.preheader.appendAcc.setInput(0, match.preheader.instance)
    match.preheader.appendAcc.setInput(1, be match.initialValue)
    match.exit.toStringCall.setInput(0, match.preheader.instance)
    foreach user being users of match.accValue outside loop
        user.replaceInput(match.accValue, match.exit.toStringCall)

function HoistInstructionsToPreHeader(match: Match)
    foreach inst in match.preheader
        hoist inst to loop preheader
    fix broken save states

function HoistInstructionsToExitBlock(match: Match)
    let exitBlock be to exit block of loop
    hoist match.exit.toStringCall to exitBlock
    foreach input being inputs of match.exit.toStringCall inside loop
        hoist input to exitBlock

function Cleanup(loop: Loop, matches: array of Match)
    foreach block in loop
        fix save states in block
    foreach match in matches
        foreach temp in match.temp
            foreach inst in temp
                remove inst
    foreach block in loop
        foreach phi in block
            if phi is not used
                remove phi from block
```

**Merge StringBuilder::append calls chain**

The algorithm works as follows. First, we find all the `StringBuilder` objects and their `append` calls in a current `block`. Second, we split vector of calls found into a groups of 2, 3, or 4 elements. We replace each group by a corresponding `StringBuilder::appendN` call.

```C#
function OptimizeStringBuilderAppendChain(block: BasicBlock)
    foreach [instance, calls] being StringBuilder instance and its vector of append calls in block
        foreach group being consicutive subvector of 2, 3, or 4 from calls
            replace group with instance.appendN call
```

**Merge StringBuilder objects chain**

This algorithm is designed for use with default and string-argument constructors and works as follows:

The algorithm traverses blocks of graph in Post Order, and instructions of each block in Reverse Order, this allows us iteratively merge potentially long chains of `StringBuilder` objects into a single object. First, we search a pairs `[instance, inputInstance]` of `StringBuilder` objects which we can merge, merge condition is: last call to `inputInstance.toString()` appended as a first argument to `instance` by calling `StringBuilder::append` (in case of default constructor) or `StringBuilder::ctor` (in case of constructor with a string argument). Then we remove first call to `StringBuilder::append` or `StringBuilder::ctor` from `instance` and last call to `StringBuilder::toString` from `inputInstance`. We retarget remaining calls of `instance` to `inputInstance`.

```C#
function OptimizeStringBuilderChain()
    foreach block in graph in PO
        foreach two objects [instance, inputInstance] being a consicutive pair of Stringbuilders in block in reverse order
            if CanMerge(instance, inputInstance)
                let firstAppend or ctorStrArg be 1st StringBuilder::append call of instance
                let lastToString be last StringBuilder::toString call of inputInstance
                remove firstAppend or ctorStrArg from instance users
                remove lastToString from inputInstance users
                foreach call being user of instance
                    call.setInput(0, inputInstance) // retarget append call to inputInstance
```

**Remove extra call for stringLength**
The algorithm works as follows. In the basic block for all instructions, if it is a stringLength instruction and it is in the cashMap, then this instruction is removed from the block. If the instruction is not in the cashMap, then it is written there together with the first input as a key. If it is not stringLength instruction, all its inputs are checked, and if one of the inputs is in the cashMap, the entry with the input key is removed from the cashMap.
```C#
function OptimizeStringBuilderStringLength(block: BasicBlock)
    foreach instructions in block
        if instruction is StringBuilder.stringLength
            let sbInstance be 1st input instruction
            if sbInstance in cashMap
                remove instruction from block
            else
                (sbInstance, instruction) add into cashMap
        if instruction is StringBuilder.stringLength
            continue
        foreach input in instruction.inputs
            if input in cashMap
                remove (input, instruction) from cashMap
```

## Examples

**Remove unnecessary String Builder**

ETS checked test ets_stringbuilder.ets, function example:

```TS
function toString0(str: String): String {
    return new StringBuilder(str).toString();
}
```

BCO IR before transformation:

(Save state and null check instructions are skipped for simplicity)

```
Method: ets_stringbuilder.ETSGLOBAL::toString0

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v4)
r253 -> r253 [ref]
succs: [bb 0]

BB 0  preds: [bb 1]
prop: bc
    2.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v3)
    3.ref  NewObject 544              v2, ss -> (v7, v4)
    4.void CallStatic 762 std.core.StringBuilder::<ctor> v3, v0, ss
    7.ref  CallStatic 809 std.core.StringBuilder::toString v3, ss -> (v9)
    9.ref  Return                     v7
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

BCO IR after transformation:

```
Method: ets_stringbuilder.ETSGLOBAL::toString0

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v9)
r253 -> r253 [ref]
succs: [bb 0]

BB 0  preds: [bb 1]
prop
    9.ref  Return                     v0
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR before transformation:

(Save state and null check instructions are skipped for simplicity)

```
Method: std.core.String ETSGLOBAL::toString0(std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v5)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
    3.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v4)
    4.ref  NewObject 15300             v3, ss -> (v5, v10)
    5.void CallStatic 51211 std.core.StringBuilder::<ctor> v4, v0, ss
   10.ref  Intrinsic.StdCoreSbToString v4, ss -> (v11)
   11.ref  Return                      v10
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR after transformation:

```
Method: std.core.String ETSGLOBAL::toString0(std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   10.ref  Return                     v0
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Replace String Builder with string concatenation**

ETS checked test ets_string_concat.ets, function example:

```TS
function concat0(a: String, b: String): String {
    return a + b;
}
```

Optimization in BCO is not supported since there are no append intrinsics yet to replace them with Intrinsic.StdCoreStringConcat2.

AOT IR before transformation:

```
Method: std.core.String ETSGLOBAL::concat0(std.core.String, std.core.String)

BB 1
prop: start
   14.ref  Parameter                  arg 0 -> (v20, v19, v16)
   15.ref  Parameter                  arg 1 -> (v23, v21, v20, v16)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v16 -> (v18)
   18.ref  NewObject 390              v17, v16 -> (v21, v25, v24, v22, v21, v20, v19)
   19.void CallStatic 443 std.core.StringBuilder::<ctor> v18, v14, v20
   23.ref  Intrinsic.StdCoreSbAppendString v22, v15, v21
   26.ref  Intrinsic.StdCoreSbToString v25, v24 -> (v27)
   27.ref  Return                     v26
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR after transformation:

```
Method: std.core.String ETSGLOBAL::concat0(std.core.String, std.core.String)

BB 1
prop: start
   14.ref  Parameter                  arg 0 -> (v31, v30, v16)
   15.ref  Parameter                  arg 1 -> (v31, v30, v16)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v16
   30.ref  Intrinsic.StdCoreStringConcat2 v14, v15, v31 -> (v27)
   27.ref  Return                     v30
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Optimize concatenation loops**

ETS checked test ets_string_concat_loop.ets, function example:

```TS
function concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;
    return str;
}
```

BCO IR before transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::concat_loop0

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v21) r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12) r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   25.i32  Constant                   0x1 -> (v26)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 481             v4 -> (v7p)
    6.     SaveStateDeoptimize        inlining_depth=0
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.ref  Phi                        v3(bb0), v24(bb2) -> (v28, v17)
   8p.i32  Phi                        v5(bb0), v26(bb2) -> (v26, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v14 -> (v16)
   16.ref  NewObject 390              v15, v14 -> (v24, v21, v17)
   18.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 443 std.core.StringBuilder::<ctor> v16, v7p, v18
   19.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 452 std.core.StringBuilder::append v16, v0, v19
   22.     SaveState                  inlining_depth=0 -> (v24)
   24.ref  CallStatic 462 std.core.StringBuilder::toString v16, v22 -> (v7p)
   26.i32  Add                        v8p, v25 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   27.     SaveState                  inlining_depth=0
   28.ref  Return                     v7p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

BCO IR after transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::concat_loop0

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v21) r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12) r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   25.i32  Constant                   0x1 -> (v26)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 481             v4 -> (v17)
    6.     SaveStateDeoptimize        inlining_depth=0
   29.     SaveState                  inlining_depth=0 -> (v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v29 -> (v16)
   30.     SaveState                  inlining_depth=0 -> (v16)
   16.ref  NewObject 390              v15, v30 -> (v17, v24, v21)
   31.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 443 std.core.StringBuilder::<ctor> v16, v3, v31
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   8p.i32  Phi                        v5(bb0), v26(bb2) -> (v26, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   19.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 452 std.core.StringBuilder::append v16, v0, v19
   26.i32  Add                        v8p, v25 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   27.     SaveState                  inlining_depth=0 -> (v24)
   24.ref  CallStatic 462 std.core.StringBuilder::toString v16, v27 -> (v28)
   28.ref  Return                     v24
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

AOT IR before transformation:

```

Method: std.core.String ETSGLOBAL::concat_loop0(std.core.String, i32)

BB 4
prop: start
   13.ref  Parameter                  arg 0 -> (v25, v28, v32, v33, v35, v36, v41, v19, v18, v15)     
   14.i32  Parameter                  arg 1 -> (v26, v28, v32, v33, v36, v41, v19, v18, v15)          
   16.i64  Constant                   0x0 -> (v20p, v19, v18)                                         
   40.i64  Constant                   0x1 -> (v39)                                                    
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
   15.     SaveState                  v13(vr3), v14(vr4), caller=8, inlining_depth=1
   18.     SaveState                  v16(vr0), v13(vr3), v14(vr4), caller=8, inlining_depth=1 -> (v17)
   17.ref  LoadString 481             v18 -> (v21p, v19, v19)
   19.     SaveStateDeoptimize        v16(vr0), v17(vr1), v13(vr3), v14(vr4), v17(ACC), caller=8, inlining_depth=1
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  20p.i32  Phi                        v16(bb0), v39(bb2) -> (v39, v36, v33, v32, v28, v28, v26)       
  21p.ref  Phi                        v17(bb0), v38(bb2) -> (v42, v41, v32, v31, v28, v25)            
   25.     SafePoint                  v13(vr3), v21p(vr1), caller=8, inlining_depth=1
   26.b    Compare GE i32             v20p, v14 -> (v27)
   27.     IfImm NE b                 v26, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   28.     SaveState                  v20p(vr0), v21p(vr1), v13(vr3), v14(vr4), v20p(ACC), caller=8, inlining_depth=1 -> (v30, v29)
   29.ref  LoadAndInitClass 'std.core.StringBuilder' v28 -> (v30)
   30.ref  NewObject 390              v29, v28 -> (v37, v36, v34, v33, v33, v32, v31)
   32.     SaveState                  v20p(vr0), v21p(vr1), v13(vr3), v14(vr4), v30(ACC), caller=8, inlining_depth=1 -> (v31)
   31.void CallStatic 443 std.core.StringBuilder::<ctor> v30, v21p, v32
   33.     SaveState                  v20p(vr0), v30(vr1), v13(vr3), v14(vr4), v30(ACC), caller=8, inlining_depth=1 -> (v35, v34)
   34.ref  NullCheck                  v30, v33 -> (v35)
   35.ref  Intrinsic.StdCoreSbAppendString v34, v13, v33
   36.     SaveState                  v20p(vr0), v30(vr1), v13(vr3), v14(vr4), caller=8, inlining_depth=1 -> (v38, v37)
   37.ref  NullCheck                  v30, v36 -> (v38)
   38.ref  Intrinsic.StdCoreSbToString v37, v36 -> (v21p)
   39.i32  Add                        v20p, v40 -> (v20p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   41.     SaveState                  v14(vr4), v21p(vr1), v13(vr3), caller=8, inlining_depth=1
   42.ref  Return                     v21p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

AOT IR after transformation:

```

Method: std.core.String ETSGLOBAL::concat_loop0(std.core.String, i32)

BB 4
prop: start
   13.ref  Parameter                  arg 0 -> (v46, v43, v44, v45, v25, v33, v35, v41, v19, v18)     
   14.i32  Parameter                  arg 1 -> (v46, v43, v44, v45, v26, v33, v41, v19, v18)          
   16.i64  Constant                   0x0 -> (v43, v44, v45, v20p, v19, v18)                          
   40.i64  Constant                   0x1 -> (v39)                                                    
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
   18.     SaveState                  v16(vr0), v13(vr3), v14(vr4), caller=8, inlining_depth=1 -> (v17)
   17.ref  LoadString 481             v18 -> (v43, v44, v45, v31, v19, v19)
   19.     SaveStateDeoptimize        v16(vr0), v17(vr1), v13(vr3), v14(vr4), v17(ACC), caller=8, inlining_depth=1
   43.     SaveState                  v16(vr0), v13(vr3), v14(vr4), v17(bridge), caller=8, inlining_depth=1 -> (v29)
   29.ref  LoadAndInitClass 'std.core.StringBuilder' v43 -> (v30)
   44.     SaveState                  v16(vr0), v13(vr3), v14(vr4), v17(bridge), caller=8, inlining_depth=1 -> (v30)
   30.ref  NewObject 390              v29, v44 -> (v41, v46, v25, v45, v31, v37, v34, v33, v33)
   45.     SaveState                  v16(vr0), v13(vr3), v14(vr4), v30(bridge), v17(bridge), caller=8, inlining_depth=1 -> (v31)
   31.void CallStatic 443 std.core.StringBuilder::<ctor> v30, v17, v45
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  20p.i32  Phi                        v16(bb0), v39(bb2) -> (v39, v33, v26)
   25.     SafePoint                  v13(vr3), v30(bridge), caller=8, inlining_depth=1
   26.b    Compare GE i32             v20p, v14 -> (v27)
   27.     IfImm NE b                 v26, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   33.     SaveState                  v20p(vr0), v30(vr1), v13(vr3), v14(vr4), v30(ACC), caller=8, inlining_depth=1 -> (v35, v34)
   34.ref  NullCheck                  v30, v33 -> (v35)
   35.ref  Intrinsic.StdCoreSbAppendString v34, v13, v33
   39.i32  Add                        v20p, v40 -> (v20p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   41.     SaveState                  v14(vr4), v30(bridge), v13(vr3), caller=8, inlining_depth=1 -> (v38)
   46.     SaveState                  v14(vr4), v30(bridge), v13(vr3), caller=8, inlining_depth=1 -> (v37)
   37.ref  NullCheck                  v30, v46 -> (v38)
   38.ref  Intrinsic.StdCoreSbToString v37, v41 -> (v42)
   42.ref  Return                     v38
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

**Merge StringBuilder::append calls chain**

ETS checked test ets_string_builder_append_merge_part4.ets, function example:

```TS
function append2(str0: string, str1: string): string {
    let sb = new StringBuilder();

    sb.append(str0);
    sb.append(str1);

    return sb.toString();
}
```

Optimization in BCO is not supported since there are no append intrinsics yet to replace them with Intrinsic.StdCoreSbAppendString2.

AOT IR before transformation:

```
Method: std.core.String ETSGLOBAL::append2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v13)
    1.ref  Parameter                  arg 1 -> (v14)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' v3 -> (v5)
    5.ref  NewObject 15705            v4, ss -> (v19, v16, v13, v6)
    6.void CallStatic 86153 std.core.StringBuilder::<ctor> v5, ss
   13.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   16.ref  Intrinsic.StdCoreSbAppendString v5, v1, ss
   19.ref  Intrinsic.StdCoreSbToString v5, ss
   20.ref  Return                     v19
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR after transformation:

```
Method: std.core.String ETSGLOBAL::append2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v13)
    1.ref  Parameter                  arg 1 -> (v14)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 15705            v4, ss -> (v19, v18, v6)
    6.void CallStatic 86153 std.core.StringBuilder::<ctor> v5, ss
   18.ref  Intrinsic.StdCoreSbAppendString2 v5, v0, v1, ss
   19.ref  Intrinsic.StdCoreSbToString v5, ss
   20.ref  Return                     v19
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Merge StringBuilder objects chain (default constructor)**

ETS checked test ets_string_concat.ets, function example:

```TS
function concat2(a: String, b: String): String {
    let sb0 = new StringBuilder()
    sb0.append(a)
    let sb1 = new StringBuilder()
    sb1.append(sb0.toString())
    sb1.append(b)
    return sb1.toString();
}
```

AOT IR before transformation:

```
Method: std.core.String ETSGLOBAL::concat2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
    1.ref  Parameter                  arg 1 -> (v24)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 12080            v4, ss -> (v18, v10, v6)
    6.void CallStatic 86229 std.core.StringBuilder::<ctor> v5, ss
   10.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   12.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v13)
   13.ref  NewObject 12080            v12, ss -> (v27, v24, v21, v14)
   14.void CallStatic 86229 std.core.StringBuilder::<ctor> v13, ss
   18.ref  Intrinsic.StdCoreSbToString v5, ss -> (v21)
   21.ref  Intrinsic.StdCoreSbAppendString v13, v18, ss
   24.ref  Intrinsic.StdCoreSbAppendString v13, v1, ss
   27.ref  Intrinsic.StdCoreSbToString v13, ss -> (v28)
   28.ref  Return                     v27
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR after transformation:

```
Method: std.core.String ETSGLOBAL::concat2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
    1.ref  Parameter                  arg 1 -> (v24)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 12080            v4, ss -> (v27, v24, v18, v10, v6)
    6.void CallStatic 86229 std.core.StringBuilder::<ctor> v5, ss
   10.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   24.ref  Intrinsic.StdCoreSbAppendString v5, v1, ss
   27.ref  Intrinsic.StdCoreSbToString v5, ss -> (v28)
   28.ref  Return                     v27
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Merge StringBuilder objects chain (constructor with a string argument)**

```TS
function concat2(a: String, b: String): String {
    let sb0 = new StringBuilder(a)
    let sb1 = new StringBuilder(sb0.toString())
    sb1.append(b)
    return sb1.toString();
}
```

AOT IR before transformation:

```

Method: std.core.String sb_test_obj_chain.ETSGLOBAL::concat2(std.core.String, std.core.String) 0x7375fbd99ba0

BB 1
prop: start
   14.ref  Parameter                  arg 0 -> (v20, v19, v16)
   15.ref  Parameter                  arg 1 -> (v29, v31, v28, v24, v21, v20, v16)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   16.     SaveState                  v14(vr2), v15(vr3), caller=9, inlining_depth=1 -> (v18, v17)
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v16 -> (v18)
   18.ref  NewObject 390              v17, v16 -> (v21, v22, v20, v19)
   20.     SaveState                  v14(vr2), v15(vr3), v18(ACC), caller=9, inlining_depth=1 -> (v19)
   19.void CallStatic 443 std.core.StringBuilder::<ctor> v18, v14, v20
   21.     SaveState                  v18(ACC), v15(vr3), caller=9, inlining_depth=1 -> (v23, v22)
   22.ref  NullCheck                  v18, v21 -> (v23)
   23.ref  Intrinsic.StdCoreSbToString v22, v21 -> (v24, v28, v27, v24)
   24.     SaveState                  v23(vr1), v23(ACC), v15(vr3), caller=9, inlining_depth=1 -> (v26, v25)
   25.ref  LoadAndInitClass 'std.core.StringBuilder' v24 -> (v26)
   26.ref  NewObject 390              v25, v24 -> (v28, v29, v33, v32, v30, v29, v27)
   28.     SaveState                  v23(vr1), v26(ACC), v15(vr3), caller=9, inlining_depth=1 -> (v27)
   27.void CallStatic 443 std.core.StringBuilder::<ctor> v26, v23, v28
   29.     SaveState                  v26(vr0), v15(vr3), v26(ACC), caller=9, inlining_depth=1 -> (v31, v30)
   30.ref  NullCheck                  v26, v29 -> (v31)
   31.ref  Intrinsic.StdCoreSbAppendString v30, v15, v29
   32.     SaveState                  v26(vr0), caller=9, inlining_depth=1 -> (v34, v33)
   33.ref  NullCheck                  v26, v32 -> (v34)
   34.ref  Intrinsic.StdCoreSbToString v33, v32 -> (v35)
   35.ref  Return                     v34
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end

```

AOT IR after transformation:

```

Method: std.core.String sb_test_obj_chain.ETSGLOBAL::concat2(std.core.String, std.core.String) 0x7375fbd99ba0

BB 1
prop: start
   14.ref  Parameter                  arg 0 -> (v20, v19, v16)
   15.ref  Parameter                  arg 1 -> (v24, v29, v31, v21, v20, v16)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   16.     SaveState                  v14(vr2), v15(vr3), caller=9, inlining_depth=1 -> (v18, v17)
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v16 -> (v18)
   18.ref  NewObject 390              v17, v16 -> (v32, v24, v29, v30, v33, v21, v22, v20, v19)
   20.     SaveState                  v14(vr2), v15(vr3), v18(ACC), caller=9, inlining_depth=1 -> (v19)
   19.void CallStatic 443 std.core.StringBuilder::<ctor> v18, v14, v20
   21.     SaveState                  v18(ACC), v15(vr3), caller=9, inlining_depth=1 -> (v22)
   22.ref  NullCheck                  v18, v21
   24.     SaveState                  v15(vr3), v18(bridge), caller=9, inlining_depth=1 -> (v25)
   25.ref  LoadAndInitClass 'std.core.StringBuilder' v24
   29.     SaveState                  v15(vr3), v18(bridge), caller=9, inlining_depth=1 -> (v31, v30)
   30.ref  NullCheck                  v18, v29 -> (v31)
   31.ref  Intrinsic.StdCoreSbAppendString v30, v15, v29
   32.     SaveState                  v18(bridge), caller=9, inlining_depth=1 -> (v34, v33)
   33.ref  NullCheck                  v18, v32 -> (v34)
   34.ref  Intrinsic.StdCoreSbToString v33, v32 -> (v35)
   35.ref  Return                     v34
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end

```

**Replace String length accessors with StringBuilder versions**

ETS checked test ets_string_concat_loop.ets, function example:

```TS
function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;
        str += a;
    }
    return str;
}
```

BCO IR before transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v38) r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12) r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   45.i32  Constant                   0x1 -> (v46)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 516             v4 -> (v7p)
    6.     SaveStateDeoptimize        inlining_depth=0
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.ref  Phi                        v3(bb0), v41(bb2) -> (v21, v48, v17)
   8p.i32  Phi                        v5(bb0), v46(bb2) -> (v46, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   14.     SaveState                  inlining_depth=0 -> (v16, v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v14 -> (v16)
   16.ref  NewObject 406              v15, v14 -> (v27, v24, v17)
   18.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 468 std.core.StringBuilder::<ctor> v16, v7p, v18
   19.     SaveState                  inlining_depth=0 -> (v21)
   21.i32  CallStatic 450 std.core.String::%%get-length v7p, v19 -> (v24)
   22.     SaveState                  inlining_depth=0 -> (v24)
   24.ref  CallStatic 477 std.core.StringBuilder::append v16, v21, v22
   25.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 497 std.core.StringBuilder::toString v16, v25 -> (v34, v30)
   28.     SaveState                  inlining_depth=0 -> (v30, v29)
   29.ref  LoadClass 'std.core.String' v28 -> (v30)
   30.     CheckCast 387              v27, v29, v28
   31.     SaveState                  inlining_depth=0 -> (v33, v32)
   32.ref  LoadAndInitClass 'std.core.StringBuilder' v31 -> (v33)
   33.ref  NewObject 406              v32, v31 -> (v41, v38, v34)
   35.     SaveState                  inlining_depth=0 -> (v34)
   34.void CallStatic 468 std.core.StringBuilder::<ctor> v33, v27, v35
   36.     SaveState                  inlining_depth=0 -> (v38)
   38.ref  CallStatic 487 std.core.StringBuilder::append v33, v0, v36
   39.     SaveState                  inlining_depth=0 -> (v41)
   41.ref  CallStatic 497 std.core.StringBuilder::toString v33, v39 -> (v7p, v44)
   42.     SaveState                  inlining_depth=0 -> (v44, v43)
   43.ref  LoadClass 'std.core.String' v42 -> (v44)
   44.     CheckCast 387              v41, v43, v42
   46.i32  Add                        v8p, v45 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   47.     SaveState                  inlining_depth=0
   48.ref  Return                     v7p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

BCO IR after transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v44)
r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12)
r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   51.i32  Constant                   0x1 -> (v52)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 857             v4 -> (v21)
    6.     SaveStateDeoptimize        inlining_depth=0
   57.     SaveState                  inlining_depth=0 -> (v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v57 -> (v16)
   58.     SaveState                  inlining_depth=0 -> (v16)
   16.ref  NewObject 645              v15, v58 -> (v55, v44, v21, v30, v27, v17)
   59.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 761 std.core.StringBuilder::<ctor> v16, v59
   60.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 799 std.core.StringBuilder::append v16, v3, v60
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1, bc: 0x0000000b
   8p.i32  Phi                        v5(bb0), v52(bb2) -> (v52, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   56.     SaveState                  inlining_depth=0 -> (v55)
   55.i32  CallStatic 770 std.core.StringBuilder::%%get-stringLength v16, v56 -> (v27)
   25.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 789 std.core.StringBuilder::append v16, v55, v25
   34.     SaveState                  inlining_depth=0 -> (v35)
   35.ref  LoadAndInitClass 'std.core.StringBuilder' v34
   42.     SaveState                  inlining_depth=0 -> (v44)
   44.ref  CallStatic 799 std.core.StringBuilder::append v16, v0, v42
   48.     SaveState                  inlining_depth=0 -> (v49)
   49.ref  LoadClass 'std.core.String' v48
   52.i32  Add                        v8p, v51 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   53.     SaveState                  inlining_depth=0 -> (v30)
   30.ref  CallStatic 829 std.core.StringBuilder::toString v16, v53 -> (v54, v33)
   62.     SaveState                  inlining_depth=0 -> (v32)
   32.ref  LoadClass 'std.core.String' v62 -> (v33)
   61.     SaveState                  inlining_depth=0 -> (v33)
   33.     CheckCast 626              v30, v32, v61
   54.ref  Return                     v30
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

AOT IR before transformation:

```
Method: std.core.String ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1(std.core.String, i32)

BB 4
prop: start
   13.ref  Parameter                  arg 0 -> (v25, v28, v32, v33, v38, v41, v44, v47, v51, v52, v54, v55, v58, v63, v19, v18, v15)
   14.i32  Parameter                  arg 1 -> (v26, v28, v32, v33, v38, v41, v44, v47, v51, v52, v58, v63, v55, v19, v18, v15)
   16.i64  Constant                   0x0 -> (v20p, v19, v18)
   36.i64  Constant                   0x2 -> (v37)
   62.i64  Constant                   0x1 -> (v61)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
   15.     SaveState                  v13(vr4), v14(vr5), caller=8, inlining_depth=1
   18.     SaveState                  v16(vr0), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v17)
   17.ref  LoadString 516             v18 -> (v21p, v19, v19)
   19.     SaveStateDeoptimize        v16(vr0), v17(vr1), v13(vr4), v14(vr5), v17(ACC), caller=8, inlining_depth=1
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  20p.i32  Phi                        v16(bb0), v61(bb2) -> (v61, v58, v55, v52, v51, v47, v44, v41, v38, v33, v32, v28, v28, v26)
  21p.ref  Phi                        v17(bb0), v57(bb2) -> (v64, v63, v41, v38, v34, v33, v32, v31, v28, v25)
   25.     SafePoint                  v13(vr4), v21p(vr1), caller=8, inlining_depth=1
   26.b    Compare GE i32             v20p, v14 -> (v27)
   27.     IfImm NE b                 v26, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   28.     SaveState                  v20p(vr0), v21p(vr1), v13(vr4), v14(vr5), v20p(ACC), caller=8, inlining_depth=1 -> (v30, v29)
   29.ref  LoadAndInitClass 'std.core.StringBuilder' v28 -> (v30)
   30.ref  NewObject 406              v29, v28 -> (v42, v41, v39, v38, v33, v33, v32, v31)
   32.     SaveState                  v20p(vr0), v21p(vr1), v13(vr4), v14(vr5), v30(ACC), caller=8, inlining_depth=1 -> (v31)
   31.void CallStatic 468 std.core.StringBuilder::<ctor> v30, v21p, v32
   33.     SaveState                  v20p(vr0), v21p(vr1), v30(vr3), v13(vr4), v14(vr5), v30(ACC), caller=8, inlining_depth=1 -> (v34)
   34.ref  NullCheck                  v21p, v33 -> (v35)
   35.i32  LenArray                   v34 -> (v37)
   37.i32  Shr                        v35, v36 -> (v40, v38)
   38.     SaveState                  v20p(vr0), v21p(vr1), v30(vr3), v13(vr4), v14(vr5), v37(ACC), caller=8, inlining_depth=1 -> (v40, v39)
   39.ref  NullCheck                  v30, v38 -> (v40)
   40.ref  Intrinsic.StdCoreSbAppendInt v39, v37, v38
   41.     SaveState                  v20p(vr0), v21p(vr1), v30(vr3), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v43, v42)
   42.ref  NullCheck                  v30, v41 -> (v43)
   43.ref  Intrinsic.StdCoreSbToString v42, v41 -> (v44, v47, v51, v50, v47, v46, v44)
   44.     SaveState                  v20p(vr0), v43(vr1), v43(ACC), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v46, v45)
   45.ref  LoadClass 'std.core.String' v44 -> (v46)
   46.     CheckCast 387              v43, v45, v44
   47.     SaveState                  v20p(vr0), v43(vr1), v43(ACC), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v49, v48)
   48.ref  LoadAndInitClass 'std.core.StringBuilder' v47 -> (v49)
   49.ref  NewObject 406              v48, v47 -> (v51, v52, v56, v55, v53, v52, v50)
   51.     SaveState                  v20p(vr0), v43(vr1), v49(ACC), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v50)
   50.void CallStatic 468 std.core.StringBuilder::<ctor> v49, v43, v51
   52.     SaveState                  v20p(vr0), v49(vr1), v49(ACC), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v54, v53)
   53.ref  NullCheck                  v49, v52 -> (v54)
   54.ref  Intrinsic.StdCoreSbAppendString v53, v13, v52
   55.     SaveState                  v20p(vr0), v49(vr1), v14(vr5), v13(vr4), caller=8, inlining_depth=1 -> (v57, v56)
   56.ref  NullCheck                  v49, v55 -> (v57)
   57.ref  Intrinsic.StdCoreSbToString v56, v55 -> (v58, v21p, v60, v58)
   58.     SaveState                  v20p(vr0), v57(vr1), v57(ACC), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v60, v59)
   59.ref  LoadClass 'std.core.String' v58 -> (v60)
   60.     CheckCast 387              v57, v59, v58
   61.i32  Add                        v20p, v62 -> (v20p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   63.     SaveState                  v14(vr5), v21p(vr1), v13(vr4), caller=8, inlining_depth=1
   64.ref  Return                     v21p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

AOT IR after transformation:

```
Method: std.core.String ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1(std.core.String, i32)

BB 4
prop: start
   13.ref  Parameter                  arg 0 -> (v71, v70, v69, v66, v67, v68, v25, v33, v38, v47, v52, v54, v55, v58, v63, v19, v18)
   14.i32  Parameter                  arg 1 -> (v58, v47, v71, v70, v69, v66, v67, v68, v26, v33, v38, v52, v63, v55, v19, v18)
   16.i64  Constant                   0x0 -> (v66, v67, v68, v20p, v19, v18)                          
   62.i64  Constant                   0x1 -> (v61)                                                    
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
   18.     SaveState                  v16(vr0), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v17)
   17.ref  LoadString 516             v18 -> (v66, v67, v68, v31, v19, v19)
   19.     SaveStateDeoptimize        v16(vr0), v17(vr1), v13(vr4), v14(vr5), v17(ACC), caller=8, inlining_depth=1
   66.     SaveState                  v16(vr0), v13(vr4), v14(vr5), v17(bridge), caller=8, inlining_depth=1 -> (v29)
   29.ref  LoadAndInitClass 'std.core.StringBuilder' v66 -> (v30)
   67.     SaveState                  v16(vr0), v13(vr4), v14(vr5), v17(bridge), caller=8, inlining_depth=1 -> (v30)
   30.ref  NewObject 406              v29, v67 -> (v63, v69, v33, v25, v58, v47, v52, v55, v52, v68, v52, v55, v34, v53, v56, v31, v42, v39, v38, v33)
   68.     SaveState                  v16(vr0), v13(vr4), v14(vr5), v30(bridge), v17(bridge), caller=8, inlining_depth=1 -> (v31)
   31.void CallStatic 468 std.core.StringBuilder::<ctor> v30, v17, v68
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  20p.i32  Phi                        v16(bb0), v61(bb2) -> (v61, v58, v55, v52, v47, v38, v33, v26)  
   25.     SafePoint                  v13(vr4), v30(bridge), caller=8, inlining_depth=1
   26.b    Compare GE i32             v20p, v14 -> (v27)
   27.     IfImm NE b                 v26, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   33.     SaveState                  v20p(vr0), v30(ACC), v30(vr3), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v34)
   34.ref  NullCheck                  v30, v33 -> (v65)
   65.i32  LoadObject 821250 std.core.StringBuilder.length v34 -> (v38, v40)
   38.     SaveState                  v20p(vr0), v65(ACC), v30(vr3), v13(vr4), v14(vr5), caller=8, inlining_depth=1 -> (v40, v39)
   39.ref  NullCheck                  v30, v38 -> (v40)
   40.ref  Intrinsic.StdCoreSbAppendInt v39, v65, v38
   47.     SaveState                  v20p(vr0), v14(vr5), v30(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v48)
   48.ref  LoadAndInitClass 'std.core.StringBuilder' v47
   52.     SaveState                  v20p(vr0), v30(vr1), v30(ACC), v13(vr4), v14(vr5), v30(bridge), caller=8, inlining_depth=1 -> (v54, v53)
   53.ref  NullCheck                  v30, v52 -> (v54)
   54.ref  Intrinsic.StdCoreSbAppendString v53, v13, v52
   55.     SaveState                  v20p(vr0), v30(vr1), v14(vr5), v13(vr4), v30(bridge), caller=8, inlining_depth=1 -> (v56)
   56.ref  NullCheck                  v30, v55
   58.     SaveState                  v20p(vr0), v14(vr5), v30(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v59)
   59.ref  LoadClass 'std.core.String' v58
   61.i32  Add                        v20p, v62 -> (v20p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   63.     SaveState                  v14(vr5), v30(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v43)
   69.     SaveState                  v14(vr5), v30(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v42)
   42.ref  NullCheck                  v30, v69 -> (v43)
   43.ref  Intrinsic.StdCoreSbToString v42, v63 -> (v70, v71, v64, v46)
   71.     SaveState                  v14(vr5), v43(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v45)
   45.ref  LoadClass 'std.core.String' v71 -> (v46)
   70.     SaveState                  v14(vr5), v43(bridge), v13(vr4), caller=8, inlining_depth=1 -> (v46)
   46.     CheckCast 387              v43, v45, v70
   64.ref  Return                     v43
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

**Remove extra call for stringLength**

ETS function example:
```TS
function StringCalculation() {
    let sb = new StringBuilder();
    let len0 = sb.stringLength;
    let len1 = sb.stringLength;
    let len2 = sb.stringLength;
    return len2.toString();
}
```
IR before transformation:
(Save state instructions are skipped for simplicity)
```
BB 1
prop: start
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
    1.ref  LoadAndInitClass 'std.core.StringBuilder' v0 -> (v2) 
    2.ref  NewObject 327              v1, v0 -> (v13, v10, v7, v3) 
    3.void CallStatic 380 std.core.StringBuilder::<ctor> v2, v4 
    7.i32  CallStatic 371 std.core.StringBuilder::%%get-stringLength v2, v5  
   10.i32  CallStatic 371 std.core.StringBuilder::%%get-stringLength v2, v9 
   13.i32  CallStatic 371 std.core.StringBuilder::%%get-stringLength v2, v11 -> (v15)
   16.     InitClass 'std.core.Int'    v14  
   15.ref  CallStatic 353 std.core.Int::toString v13, v14 -> (v17)
   17.ref  Return                     v15
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```
IR after transformation:
```
BB 1
prop: start
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
    1.ref  LoadAndInitClass 'std.core.StringBuilder' v0 -> (v2)
    2.ref  NewObject 327              v1, v0 -> (v7, v3)
    3.void CallStatic 380 std.core.StringBuilder::<ctor> v2, v4 
    7.i32  CallStatic 371 std.core.StringBuilder::%%get-stringLength v2, v5 -> (v15)
   16.     InitClass 'std.core.Int'    v14
   15.ref  CallStatic 353 std.core.Int::toString v7, v14 -> (v17)
   17.ref  Return                     v15
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

## Links

* Implementation
  * [simplify_string_builder.h](../optimizer/optimizations/simplify_string_builder.h)
  * [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
* Depends on
  * Runtime specialization [runtime_adapter_ets.h](../../plugins/ets/compiler/runtime_adapter_ets.h)
  * Intrinsics [ets_libbase_runtime.yaml](../../plugins/ets/runtime/ets_libbase_runtime.yaml)
* Tests
  * [ets_stringbuilder.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_stringbuilder.ets)
  * [ets_string_builder_append_merge_part1.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part1.ets)
  * [ets_string_builder_append_merge_part2.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part2.ets)
  * [ets_string_builder_append_merge_part3.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part3.ets)
  * [ets_string_builder_append_merge_part4.ets](../../plugins/ets/tests/checked_urunner/simple_run/ets_string_builder_append_merge_part4.ets)
  * [ets_string_builder_merge.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_merge.ets)
  * [ets_string_concat.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_concat.ets)
  * [ets_string_concat_loop.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_concat_loop.ets)
  * [ets_stringbuilder_length.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_stringbuilder_length.ets)
  * other checked tests

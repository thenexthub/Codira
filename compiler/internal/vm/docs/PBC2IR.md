| PBC | IR |
|-----|----|
| nop | - |
| mov | - |
| mov.64 | - |
| mov.obj | - |
| movi | i64 Constant |
| movi.64 | i64 Constant |
| fmovi | f32 Constant |
| fmovi.64 | f64 Constant |
| mov.null | i64 Constatnt 0 |
| lda | - |
| lda.64 | - |
| lda.obj | - |
| ldai | i64 Constant |
| ldai.64 | i64 Constant |
| fldai | f32 Constant |
| fldai.64 | f64 Constant |
| lda.str | ref LoadString |
| lda.const ||
| lda.type | ref LoadType |
| lda.null | i64 Constant 0 |
| sta | - |
| sta.64 | - |
| sta.obj | - |
| cmp.64 | i32 Cmp |
| ucmp | i32 Cmp |
| ucmp.64 | i32 Cmp |
| fcmpl | i32 Cmp |
| fcmpl.64 | i32 Cmp |
| fcmpg | i32 Cmp |
| fcmpg.64 | i32 Cmp |
| jmp | - |
| jeq.obj | bool Compare EQ, IfImm |
| jne.obj | bool Compare NE, IfImm |
| jeqz.obj | i64 Constant 0, bool Compare EQ, IfImm |
| jnez.obj | i64 Constant 0, bool Compare NE, IfImm |
| jeqz | i64 Constant 0, bool Compare EQ, IfImm |
| jnez | i64 Constant 0, bool Compare NE, IfImm |
| jltz | i64 Constant 0, bool Compare LT, IfImm |
| jgtz | i64 Constant 0, bool Compare GT, IfImm |
| jlez | i64 Constant 0, bool Compare LE, IfImm |
| jgez | i64 Constant 0, bool Compare GE, IfImm |
| jeq | bool Compare EQ, IfImm |
| jne | bool Compare NE, IfImm |
| jlt | bool Compare LT, IfImm |
| jgt | bool Compare GT, IfImm |
| jle | bool Compare LE, IfImm |
| jge | bool Compare GE, IfImm |
| fneg | f32 Neg |
| fneg.64 | f64 Neg |
| neg | i32 Neg |
| neg.64 | i64 Neg |
| not | i32 Not|
| not.64 | i64 Not |
| add2 | i32 Add |
| add2.64 | i64 Add |
| sub2 | i32 Sub |
| sub2.64 | i64 Sub |
| mul2 | i32 Mul |
| mul2.64 | i64 Mul |
| and2 | i32 And |
| and2.64 | i64 And |
| or2 | i32 Or |
| or2.64 | i64 Or |
| xor2 | i32 Xor |
| xor2.64 | i64 Xor |
| shl2 | i32 Shl |
| shl2.64 | i64 Shl |
| shr2 | i32 Shr |
| shr2.64 | i64 Shr |
| ashr2 | i32 Ashr |
| ashr2.64 | i64 Ashr |
| fadd2 | f32 Add |
| fadd2.64 | f64 Add |
| fsub2 | f32 Sub |
| fsub2.64 | f64 Sub |
| fmul2 | f32 Mul |
| fmul2.64 | f64 Mul |
| fdiv2 | f32 Div |
| fdiv2.64 | f64 Div |
| fmod2 | f32 Mod |
| fmod2.64 | f64 Mod |
| div2 | i32 Div |
| div2.64 | i64 Div |
| mod2 | i32 Mod |
| mod2.64 | i64 Mod |
| divu2 | u32 Div |
| divu2.64 | u64 Div |
| modu2 | u32 Div |
| modu2.64 | u64 Mod |
| addi | i64 Constant, i32 Add |
| subi | i64 Constant, i32 Sub |
| muli | i64 Constant, i32 Mul |
| andi | i64 Constant, i32 And |
| ori | i64 Constant, i32 Or |
| xori | i64 Constant, i32 Xor |
| shli | i64 Constant, i32 Shl |
| shri | i64 Constant, i32 Shr |
| ashri | i64 Constant, i32 Ashr |
| divi | i64 Constant, i32 Div |
| modi | i64 Constant, i32 Mod |
| add | i32 Add |
| sub | i32 Sub |
| mul |i32 Mul |
| and | i32 And |
| or | i32 Or |
| xor | i32 Xor |
| shl | i32 Shl |
| shr | i32 Shr|
| ashr | i32 Ashr |
| div | i32 Div |
| mod | i32 Mod |
| inci | i64 Constant, s32 Add |
| i32tof32 | f32 Cast |
| i32tof64 | f64 Cast |
| u32tof32 | f32 Cast |
| u32tof64 | f64 Cast |
| i64tof32 | f32 Cast |
| i64tof64 | f64 Cast |
| u64tof32 | f32 Cast |
| u64tof64 | f64 Cast |
| f32tof64 | f64 Cast |
| f32toi32 | i32 Cast |
| f32toi64 | i64 Cast |
| f32tou32 | u32 Cast |
| f32tou64 | u64 Cast |
| f64toi32 | i32 Cast |
| f64toi64 | i64 Cast |
| f64tou32 | u32 Cast |
| f64tou64 | u64 Cast |
| f64tof32 | f32 Cast |
| i32tou1 | u1 Cast |
| i64tou1 | u1 Cast |
| u32tou1 | u1 Cast |
| u64tou1 | u1 Cast |
| i32toi64 | i64 Cast |
| i32toi16 | i16 Cast |
| i32tou16 | u16 Cast |
| i32toi8 | i8 Cast |
| i32tou8 | u8 Cast |
| i64toi32 | i32 Cast |
| u32toi64 | i64 Cast |
| u32toi16 | i16 Cast |
| u32tou16 | u16 Cast |
| u32toi8 | i8 Cast |
| u32tou8 | u8 Cast |
| u64toi32 | i32 Cast |
| u64tou32 | u32 Cast |
| ldarr.8 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, i8 LoadArray |
| ldarru.8 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, u8 LoadArray |
| ldarr.16 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, i16 LoadArray |
| ldarru.16 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, u16 LoadArray |
| ldarr | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, i32 LoadArray |
| ldarr.64 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, i64 LoadArray |
| fldarr.32 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, f32 LoadArray |
| fldarr.64 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, f64 LoadArray |
| ldarr.obj | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck, ref LoadArray |
| starr.8 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  i8 StoreArray |
| starr.16 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  i16 StoreArray |
| starr | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  i32 StoreArray |
| starr.64 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  i64 StoreArray |
| fstarr.32 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  f32 StoreArray |
| fstarr.64 | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  f64 StoreArray |
| starr.obj | SaveState, ref NullCheck, i32 LenArray, i32 BoundsCheck,  ref StoreArray |
| lenarr | SaveState, ref NullCheck, i32 LenArray |
| newarr | SaveState, ref NegativeCheck, ref LoadAndInitObject, i32 NewArray |
| newobj | SaveState, ref LoadAndInitObject, ref NewObject |
| initobj.short | SaveState, ref LoadAndInitObject, ref NewObject, void CallStatic |
| initobj | SaveState, ref LoadAndInitObject, ref NewObject, void CallStatic |
| initobj.range | SaveState, ref LoadAndInitObject, ref NewObject, void CallStatic |
| ldobj | SaveState, ref NullCheck, i32 LoadObject |
| ldobj.64 | SaveState, ref NullCheck, i64 LoadObject |
| ldobj.obj | SaveState, ref NullCheck, ref LoadObject |
| stobj | SaveState, ref NullCheck, i32 StoreObject |
| stobj.64 | SaveState, ref NullCheck, i64 StoreObject |
| stobj.obj | SaveState, ref NullCheck, ref StoreObject |
| ldobj.v | SaveState, ref NullCheck, i32 LoadObject |
| ldobj.v.64 | SaveState, ref NullCheck, i64 LoadObject |
| ldobj.v.obj | SaveState, ref NullCheck, ref LoadObjec |
| stobj.v | SaveState, ref NullCheck, i32 StoreObject |
| stobj.v.64 | SaveState, ref NullCheck, i64 StoreObject |
| stobj.v.obj | SaveState, ref NullCheck, ref StoreObject |
| ldstatic | SaveState, ref LoadAndInitObject, i32 LoadStatic |
| ldstatic.64 | SaveState, ref LoadAndInitObject, i64 LoadStatic |
| ldstatic.obj | SaveState, ref LoadAndInitObject, ref LoadStatic |
| ststatic | SaveState, ref LoadAndInitObject, i32 LoadStatic |
| ststatic.64 | SaveState, ref LoadAndInitObject, i64 LoadStatic |
| ststatic.obj | SaveState, ref LoadAndInitObject, ref LoadStatic |
| return | i32 Return|
| return.64 | i64 Return|
| return.obj | ref Return |
| return.void | ReturnVoid |
| throw | SaveState, Throw |
| checkcast | LoadClass, CheckCast |
| isinstance | LoadClass, i32 IsInstance |
| call.short | SaveState, i32 CallStatic |
| call | SaveState, i64 CallStatic |
| call.range | SaveState, i64 CallStatic |
| call.acc.short | SaveState, i64 CallStatic |
| call.acc | SaveState, i64 CallStatic |
| call.virt.short | SaveState, ref NullCheck, i32 CallVirtal |
| call.virt | SaveState, ref NullCheck, i32 CallVirtal |
| call.virt.range | SaveState, ref NullCheck, i32 CallVirtal |
| call.virt.acc.short | SaveState, ref NullCheck, i32 CallVirtal |
| call.virt.acc | SaveState, ref NullCheck, i32 CallVirtal |
| mov.dyn | Not implemented yet |
| lda.dyn | Not implemented yet |
| sta.dyn | Not implemented yet |
| ldai.dyn ||
| fldai.dyn ||
| return.dyn | Not implemented yet |
| calli.dyn.short ||
| calli.dyn ||
| calli.dyn.range | Not implemented yet |

This document generated by compiler/tools/pbc_2_ir_doc_gen.sh.

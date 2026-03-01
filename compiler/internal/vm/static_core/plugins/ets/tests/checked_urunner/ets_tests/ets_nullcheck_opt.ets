/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// The starting IR, and the IR after optimization differ only by register allocation
// We check that acc is allocated in proper places
//! CHECKER     BCO: load/store eliminations near ets.nullcheck
//! SKIP_IF       @architecture == "arm32"
//! RUN_BCO       options: ""
//!
//! METHOD        "ets_nullcheck_opt.A.getx"
//! PASS_BEFORE   "RegAccAlloc"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.A.*v\d+ -> \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+ -> \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+, v\d+/
//! INST_NEXT     /Return.*v\d+/
//! PASS_AFTER    "RegAllocGraphColoring"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.A.*v\d+ -> r\d+ \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+\(r\d+\) -> acc \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+\(acc\), v\d+/
//! INST_NEXT     /Return.*v\d+\(acc\)/
//!
//! METHOD        "ets_nullcheck_opt.B.getx"
//! PASS_BEFORE   "RegAccAlloc"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.A.*v\d+ -> \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+ -> \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+, v\d+/
//! INST_NEXT     /Return.*v\d+/
//! PASS_AFTER    "RegAllocGraphColoring"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.A.*v\d+ -> r\d+ \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+\(r\d+\) -> acc \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+\(acc\), v\d+/
//! INST_NEXT     /Return.*v\d+\(acc\)/
//!
//! METHOD        "ets_nullcheck_opt.C.getx"
//! PASS_BEFORE   "RegAccAlloc"
//! INST          /LoadAndInitClass.*std\.core\.Double.*v\d+ -> \(v\d+\)/
//! INST          /Constant.* -> \(v\d+\)/
//! INST_NEXT     /InitObject.*std\.core\.Double::<ctor> v\d+, v\d+.*/
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.C.*v\d+ -> \(v\d+\)/
//! INST_NEXT     /StoreStatic.*v\d+, v\d+/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+, v\d+/
//! INST_NEXT     /Return.*v\d+/
//! PASS_AFTER    "RegAllocGraphColoring"
//! INST          /LoadAndInitClass.*std\.core\.Double.*v\d+ -> r\d+ \(v\d+\)/
//! INST          /Constant.* -> r\d+ \(v\d+\)/
//! INST_NEXT     /InitObject.*std\.core\.Double::<ctor> v\d+\(r\d+\), v\d+\(r\d+\).*/
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.C.*v\d+ -> r\d+ \(v\d+\)/
//! INST_NEXT     /StoreStatic.*v\d+\(r\d+\), v\d+\(acc\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+\(acc\), v\d+/
//! INST_NEXT     /Return.*v\d+\(acc\)/
//!
//! RUN          options: "--compiler-enable-jit=false", entry: "ets_nullcheck_opt.ETSGLOBAL::main"

// The test below is negative: not optimized when the intrinsic EtsEquals is present. acc is only
// assigned to the Return command
//! CHECKER     BCO: load/store elimination does not work if another intrinsic is present near ets.nullcheck
//! SKIP_IF       @architecture == "arm32"
//! RUN_BCO       options: ""
//! METHOD        "ets_nullcheck_opt.C.isThat"
//! PASS_BEFORE   "RegAccAlloc"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.C.*v\d+ -> \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+ -> \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+, v\d+/
//! INST          /Intrinsic.CompilerEtsEquals v\d+, v\d+, v\d+ -> \(v\d+\)/
//! INST_NEXT     /Return.*v\d+/
//! PASS_AFTER    "RegAllocGraphColoring"
//! INST          /LoadAndInitClass.*ets_nullcheck_opt\.C.*v\d+ -> r\d+ \(v\d+\)/
//! INST_NEXT     /LoadStatic.*v\d+\(r\d+\) -> r\d+ \(v\d+, v\d+\)/
//! INST          /Intrinsic.CompilerEtsNullcheck v\d+\(r\d+\), v\d+/
//! INST          /Intrinsic.CompilerEtsEquals v\d+\(r\d+\), v\d+\(r\d+\), v\d+ -> \(v\d+\)/
//! INST_NEXT     /Return.*v\d+\(acc\)/

class A {
    static x: Object = new String("foo")
    getx():Object { 
        return A.x
    }
}

class B {
    getx():Object {
        return A.x
    }
}

class C {
    static x: Object = new String("bar")
    getx():Object {
        C.x = 3.14
        return C.x
    }
    isThat(that: Object): boolean {
        return (C.x == that)
    }
}

function main(): void
{
    let objA = new A
    let objB = new B
    let objC = new C
    arktest.assertEQ(objA.getx(), "foo")
    arktest.assertEQ(objB.getx(), "foo")
    arktest.assertEQ(objC.isThat("bar"), true)
    arktest.assertEQ(objC.getx(), 3.14)
    arktest.assertEQ(objC.isThat("bar"), false)
    arktest.assertEQ(objC.isThat(3.14), true)
}

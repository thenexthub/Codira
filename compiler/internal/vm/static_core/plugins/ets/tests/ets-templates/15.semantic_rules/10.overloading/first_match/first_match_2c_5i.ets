/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: First match for overload. Check interface order linearization - in the textual order of the implements clause 
---*/

/* Case scenario:

I1  I2   I3
  \ /   /
   C1  I4  I5
    \ /___/
     C2
*/

class P1 {}
class P2 {}
class P3 {}
class P4 {}
class P5 {}

interface I1 {
        foo(): void { }
        foo_i1_p1(p: P1): string { return "I1" }
        overload foo { foo, foo_i1_p1 }
    }
interface I2 {
        foo(): void
        foo_i2_p1(p: P1): string { return "I2" }
        foo_i2_p2(p: P2): string { return "I2" }
        overload foo { foo, foo_i2_p1, foo_i2_p2 }
    }
class C1 implements I1, I2 { }

interface I3 { 
        foo(): void
        foo_i3_p1(p: P1): string { return "I3" }
        foo_i3_p2(p: P2): string { return "I3" }
        foo_i3_p3(p: P3): string { return "I3" }
        foo_i3_p4(p: P4): string { return "I3" }
        overload foo { foo, foo_i3_p1, foo_i3_p2, foo_i3_p3, foo_i3_p4 } 
    }
interface I4 extends I3 { 
        foo_i4_p1(p: P1): string { return "I4" }
        foo_i4_p2(p: P2): string { return "I4" }
        foo_i4_p3(p: P3): string { return "I4" }
        overload foo { foo, foo_i4_p1, foo_i4_p2, foo_i4_p3 }
    }
interface I5 { 
        foo(): void
        foo_i5_p1(p: P1): string { return "I5" }
        foo_i5_p2(p: P2): string { return "I5" }
        foo_i5_p3(p: P3): string { return "I5" }
        foo_i5_p4(p: P4): string { return "I5" }
        foo_i5_p5(p: P5): string { return "I5" }
        overload foo { foo, foo_i5_p1, foo_i5_p2, foo_i5_p3, foo_i5_p4, foo_i5_p5 } 
    }
class C2 extends C1 implements I4, I5 { foo() {} }

let c1 = new C1()
let c2 = new C2()

// interface order for first match 
arktest.assertEQ(c2.foo(new P1), "I1") // SET: (I1 -> I2 -> I4 -> I3 -> I5) for param P1 type
arktest.assertEQ(c2.foo(new P2), "I2") // SET: (I2 -> I4 -> I3 -> I5)       for param P2 type
arktest.assertEQ(c2.foo(new P3), "I4") // SET: (I4 -> I3 -> I5)             for param P3 type
arktest.assertEQ(c2.foo(new P4), "I3") // SET: (I3 -> I5)                   for param P4 type
arktest.assertEQ(c2.foo(new P5), "I5") // SET: (I5)                         for param P5 type

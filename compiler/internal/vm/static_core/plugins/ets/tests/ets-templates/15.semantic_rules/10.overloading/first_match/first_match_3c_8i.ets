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
desc: First match for overload. Checks for case with 3 classes and 8 interfaces 
---*/

/* Case scenario:

I1  I2   I3
  \ /   /
   C1  I4  I5  I6
    \ /___/   /
     C2      I7  I8
      \ ____/ __/
      C3
*/

class P1 {}
class P2 {}
class P3 {}

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
class C1 implements I1, I2 {foo(p: P1): string { return "C1" }}

interface I3 { 
        foo(): void
        foo_i3_p1(p: P1): string { return "I3" }
        overload foo { foo, foo_i3_p1 } 
    }
interface I4 extends I3 { 
        foo_i4_p1(p: P1): string { return "I4" }
        foo_i4_p2(p: P2): string { return "I4" }
        overload foo { foo, foo_i4_p1, foo_i4_p2 }
    }
interface I5 { 
        foo(): void
        foo_i5_p1(p: P1): string { return "I5" }
        overload foo { foo, foo_i5_p1 } 
    }
class C2 extends C1 implements I4, I5 {   
    foo() {}
    foo(p: P1): string { return "C2" }
}

interface I6 { 
        foo(): void
        foo_i6_p1(p: P1): string { return "I6" }
        foo_i6_p2(p: P2): string { return "I6" }
        overload foo { foo, foo_i6_p1, foo_i6_p2 } 
    }
interface I7 extends I6 { 
        foo_i7_p1(p: P1): string { return "I7" }
        overload foo { foo, foo_i7_p1 }  
    }
interface I8 { 
        foo(): void
        foo_i8_p1(p: P1): string { return "I8" }
        foo_i8_p3(p: P3): string { return "I8" }
        overload foo { foo, foo_i8_p1, foo_i8_p3 }  
    }
class C3 extends C2 implements I7, I8 {
    foo() {}
    foo(p: P1): string { return "C3" }
}

let c1 = new C1()
let c2 = new C2()
let c3 = new C3()

// first match for full set
arktest.assertEQ(c3.foo(new P1), "C3") // SET: (C3 -> C2 -> C1 -> I1 -> I2 -> I4 -> I3 -> I5 -> I7 -> I6 -> I8) for param P1 type
arktest.assertEQ(c2.foo(new P1), "C2") // SET: (C2 -> C1 -> I1 -> I2 -> I4 -> I3 -> I5 -> I7 -> I6 -> I8)       for param P1 type
arktest.assertEQ(c1.foo(new P1), "C1") // SET: (C1 -> I1 -> I2 -> I4 -> I3 -> I5 -> I7 -> I6 -> I8)             for param P1 type

// first match for interface order by classes
arktest.assertEQ(c3.foo(new P2), "I2") // SET: (I2 -> I4 -> I6) for param P2 type
arktest.assertEQ(c2.foo(new P2), "I2") // SET: (I2 -> I4)       for param P2 type
arktest.assertEQ(c1.foo(new P2), "I2") // SET: (I2)             for param P2 type

// first match for boundary case - I8
arktest.assertEQ(c3.foo(new P3), "I8") // SET: (I8) for param P3 type

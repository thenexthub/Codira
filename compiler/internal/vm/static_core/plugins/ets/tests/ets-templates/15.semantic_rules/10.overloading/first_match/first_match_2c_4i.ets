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
desc: First match for overload. Classes selected first 
---*/

/* Case scenario:

I1  I2   I3
  \ /   /
   C1  I4
    \ /
     C2
*/

class P1 {}
class P2 {}

interface I1 {
        foo(): void { }
        foo_i1_p1(p: P1): string { return "I1" }
        foo_i1_p2(p: P2): string { return "I1" }
        overload foo { foo, foo_i1_p1, foo_i1_p2 }
    }
interface I2 {
        foo(): void
        foo_i2_p1(p: P1): string { return "I2" }
        foo_i2_p2(p: P2): string { return "I2" }
        overload foo { foo, foo_i2_p1, foo_i2_p2 }
    }
class C1 implements I1, I2 { 
        foo(p: P1): string { return "C1" } 
        foo(p: P2): string { return "C1" }
    }

interface I3 { 
        foo(): void
        foo_i3_p1(p: P1): string { return "I3" }
        foo_i3_p2(p: P2): string { return "I3" }
        overload foo { foo, foo_i3_p1, foo_i3_p2 } 
    }
interface I4 extends I3 { 
        foo_i4_p1(p: P1): string { return "I4" }
        foo_i4_p2(p: P2): string { return "I4" }
        overload foo { foo, foo_i4_p1, foo_i4_p2 }
    }
class C2 extends C1 implements I4 {   
    foo() {}
    foo(p: P1): string { return "C2" }
}

let c1 = new C1()
let c2 = new C2()

// classes selected first 
arktest.assertEQ(c2.foo(new P1), "C2") // SET: (C2 -> C1 -> I1 -> I2 -> I4) for param P1 type
arktest.assertEQ(c2.foo(new P2), "C1") // SET: (C1 -> I1 -> I2 -> I4)       for param P2 type

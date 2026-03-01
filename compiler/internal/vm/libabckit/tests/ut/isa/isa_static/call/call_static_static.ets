/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */
// Before AOP:

function bar() {
    console.log("bar")
}

class B {
    foo() {
        console.log("B::foo")
    }
}
class C {
    foo() {
        console.log("C::foo")
    }
}

class A {
    static foo() {
        console.log("A::foo")
    }
    static fooArgs(b: B, c: C) {
        b.foo();
        c.foo();
    }
}

function main() {}

// After AOP1:
// function bar() {
//     console.log("bar")
// }
//
// class A {
//     static foo() {
//         console.log("foo")
//     }
// }
//
// function main() {
//     A.foo();
// }

// After AOP2:
// function bar() {
//     console.log("bar")
// }
//
// class A {
//     static foo() {
//         console.log("foo")
//     }
// }
//
// function main() {
//     bar();
// }

// After AOP3:
//
// class B {
//     foo() {
//         console.log("B::foo")
//     }
// }
// class C {
//     foo() {
//         console.log("C::foo")
//     }
// }

// class A {
//     static foo() {
//         console.log("A::foo")
//     }
//     static fooArgs(b: B, c: C) {
//         b.foo();
//         c.foo();
//     }
// }

// function main() {
//     let b = new B();
//     let c = new C();
//     fooArgs(b, c);
// }

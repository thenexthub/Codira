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

/*---
desc: >-
    If the object to the left of ('?.') is undefined or null, then the evaluation of the entire
    surrounding expression is dropped, and undefined is used as the result of the expression.
---*/
function test(b: boolean) {
  if (!b) {
      throw new Error()
  }
}

class A {
    fldA: string|null = "abc"
    fldAn: string|null = null
    metA(p: string): string|null {
        return p == "null" ? null : p
    }
}

class B {
    fldB: A|null = new A()
    fldBn: A|null = null
    metB(p: string): A|null {
        return p == "null" ? null : new A()
    }
}

function boo(p: string): B|null {
    return p == "null" ? null : new B()
}


function main() {
    test(boo("not null")?.fldB?.fldA === "abc")
    test(boo("not null")?.fldB?.fldAn === null)
    test(boo("not null")?.fldB?.metA("xyz") === "xyz")
    test(boo("not null")?.fldB?.metA("null") === null)

    test(boo("not null")?.metB("not null")?.fldA === "abc")
    test(boo("not null")?.metB("not null")?.fldAn === null)
    test(boo("not null")?.metB("not null")?.metA("xyz") === "xyz")
    test(boo("not null")?.metB("not null")?.metA("null") === null)

    test(boo("not null")?.fldBn?.fldA === undefined)
    test(boo("not null")?.fldBn?.fldAn === undefined)
    test(boo("not null")?.fldBn?.metA("xyz") === undefined)
    test(boo("not null")?.fldBn?.metA("null") === undefined)

    test(boo("not null")?.metB("null")?.fldA === undefined)
    test(boo("not null")?.metB("null")?.fldAn === undefined)
    test(boo("not null")?.metB("null")?.metA("xyz") === undefined)
    test(boo("not null")?.metB("null")?.metA("null") === undefined)

    test(boo("null")?.fldB?.fldA === undefined)
    test(boo("null")?.fldB?.fldAn === undefined)
    test(boo("null")?.fldB?.metA("xyz") === undefined)
    test(boo("null")?.fldB?.metA("null") === undefined)

    test(boo("null")?.metB("not null")?.fldA === undefined)
    test(boo("null")?.metB("not null")?.fldAn === undefined)
    test(boo("null")?.metB("not null")?.metA("xyz") === undefined)
    test(boo("null")?.metB("not null")?.metA("null") === undefined)

    test(boo("null")?.fldBn?.fldA === undefined)
    test(boo("null")?.fldBn?.fldAn === undefined)
    test(boo("null")?.fldBn?.metA("xyz") === undefined)
    test(boo("null")?.fldBn?.metA("null") === undefined)

    test(boo("null")?.metB("null")?.fldA === undefined)
    test(boo("null")?.metB("null")?.fldAn === undefined)
    test(boo("null")?.metB("null")?.metA("xyz") === undefined)
    test(boo("null")?.metB("null")?.metA("null") === undefined)
}

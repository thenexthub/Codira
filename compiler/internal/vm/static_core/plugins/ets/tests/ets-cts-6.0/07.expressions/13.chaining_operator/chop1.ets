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
    fldA: string|undefined = "abc"
    fldAn: string|undefined = undefined
    metA(p: string): string|undefined {
        return p == "undefined" ? undefined : p
    }
}

class B {
    fldB: A|undefined = new A()
    fldBn: A|undefined = undefined
    metB(p: string): A|undefined {
        return p == "undefined" ? undefined : new A()
    }
}

function boo(p: string): B|undefined {
    return p == "undefined" ? undefined : new B()
}

function main() {
    test(boo("not undefined")?.fldB?.fldA === "abc")
    test(boo("not undefined")?.fldB?.fldAn === undefined)
    test(boo("not undefined")?.fldB?.metA("xyz") === "xyz")
    test(boo("not undefined")?.fldB?.metA("undefined") === undefined)

    test(boo("not undefined")?.metB("not undefined")?.fldA === "abc")
    test(boo("not undefined")?.metB("not undefined")?.fldAn === undefined)
    test(boo("not undefined")?.metB("not undefined")?.metA("xyz") === "xyz")
    test(boo("not undefined")?.metB("not undefined")?.metA("undefined") === undefined)

    test(boo("not undefined")?.fldBn?.fldA === undefined)
    test(boo("not undefined")?.fldBn?.fldAn === undefined)
    test(boo("not undefined")?.fldBn?.metA("xyz") === undefined)
    test(boo("not undefined")?.fldBn?.metA("undefined") === undefined)

    test(boo("not undefined")?.metB("undefined")?.fldA === undefined)
    test(boo("not undefined")?.metB("undefined")?.fldAn === undefined)
    test(boo("not undefined")?.metB("undefined")?.metA("xyz") === undefined)
    test(boo("not undefined")?.metB("undefined")?.metA("undefined") === undefined)

    test(boo("undefined")?.fldB?.fldA === undefined)
    test(boo("undefined")?.fldB?.fldAn === undefined)
    test(boo("undefined")?.fldB?.metA("xyz") === undefined)
    test(boo("undefined")?.fldB?.metA("undefined") === undefined)

    test(boo("undefined")?.metB("not undefined")?.fldA === undefined)
    test(boo("undefined")?.metB("not undefined")?.fldAn === undefined)
    test(boo("undefined")?.metB("not undefined")?.metA("xyz") === undefined)
    test(boo("undefined")?.metB("not undefined")?.metA("undefined") === undefined)

    test(boo("undefined")?.fldBn?.fldA === undefined)
    test(boo("undefined")?.fldBn?.fldAn === undefined)
    test(boo("undefined")?.fldBn?.metA("xyz") === undefined)
    test(boo("undefined")?.fldBn?.metA("undefined") === undefined)

    test(boo("undefined")?.metB("undefined")?.fldA === undefined)
    test(boo("undefined")?.metB("undefined")?.fldAn === undefined)
    test(boo("undefined")?.metB("undefined")?.metA("xyz") === undefined)
    test(boo("undefined")?.metB("undefined")?.metA("undefined") === undefined)
}

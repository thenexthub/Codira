/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    If the object to the left of ('?.') is undefined or null, then the evaluation of the entire
    surrounding expression is dropped, and undefined is used as the result of the expression.
---*/

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
    arktest.assertEQ(boo("not null")?.fldB?.fldA , "abc")
    arktest.assertEQ(boo("not null")?.fldB?.fldAn , null)
    arktest.assertEQ(boo("not null")?.fldB?.metA("xyz") , "xyz")
    arktest.assertEQ(boo("not null")?.fldB?.metA("null") , null)

    arktest.assertEQ(boo("not null")?.metB("not null")?.fldA , "abc")
    arktest.assertEQ(boo("not null")?.metB("not null")?.fldAn , null)
    arktest.assertEQ(boo("not null")?.metB("not null")?.metA("xyz") , "xyz")
    arktest.assertEQ(boo("not null")?.metB("not null")?.metA("null") , null)

    arktest.assertEQ(boo("not null")?.fldBn?.fldA , undefined)
    arktest.assertEQ(boo("not null")?.fldBn?.fldAn , undefined)
    arktest.assertEQ(boo("not null")?.fldBn?.metA("xyz") , undefined)
    arktest.assertEQ(boo("not null")?.fldBn?.metA("null") , undefined)

    arktest.assertEQ(boo("not null")?.metB("null")?.fldA , undefined)
    arktest.assertEQ(boo("not null")?.metB("null")?.fldAn , undefined)
    arktest.assertEQ(boo("not null")?.metB("null")?.metA("xyz") , undefined)
    arktest.assertEQ(boo("not null")?.metB("null")?.metA("null") , undefined)

    arktest.assertEQ(boo("null")?.fldB?.fldA , undefined)
    arktest.assertEQ(boo("null")?.fldB?.fldAn , undefined)
    arktest.assertEQ(boo("null")?.fldB?.metA("xyz") , undefined)
    arktest.assertEQ(boo("null")?.fldB?.metA("null") , undefined)

    arktest.assertEQ(boo("null")?.metB("not null")?.fldA , undefined)
    arktest.assertEQ(boo("null")?.metB("not null")?.fldAn , undefined)
    arktest.assertEQ(boo("null")?.metB("not null")?.metA("xyz") , undefined)
    arktest.assertEQ(boo("null")?.metB("not null")?.metA("null") , undefined)

    arktest.assertEQ(boo("null")?.fldBn?.fldA , undefined)
    arktest.assertEQ(boo("null")?.fldBn?.fldAn , undefined)
    arktest.assertEQ(boo("null")?.fldBn?.metA("xyz") , undefined)
    arktest.assertEQ(boo("null")?.fldBn?.metA("null") , undefined)

    arktest.assertEQ(boo("null")?.metB("null")?.fldA , undefined)
    arktest.assertEQ(boo("null")?.metB("null")?.fldAn , undefined)
    arktest.assertEQ(boo("null")?.metB("null")?.metA("xyz") , undefined)
    arktest.assertEQ(boo("null")?.metB("null")?.metA("null") , undefined)
}
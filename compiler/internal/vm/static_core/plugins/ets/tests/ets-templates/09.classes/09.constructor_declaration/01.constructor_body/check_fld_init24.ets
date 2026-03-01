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
desc: Ensure that all fields are initialized in the specified scenario
---*/

function foo(p: number): string {
    return '' + p
}

class A {
    f00: null = null
    f01: byte = 1
    f02: short = 2
    f03: int = 3
    f04: long = 4
    f05: float = 1.5f
    f06: double = 2.5
    f07: boolean = true
    f08: string = foo(3.14)
    f09: bigint = 111111111111111111111111n
    f10: Array<Object> = [new Object, 'b', 333n]
    f11: FixedArray<int> = [42, 43, 44]
    f12: [string, boolean|int] = ['12', 12]
    f13: Record<int, string> = { 1: '1', 2: '2' }
    f14: (p: number) => string = foo

    constructor(a: string) {}

    constructor(a: string, b: int) {
        this(a)
    }
}
class B extends A {
    constructor(a: string) { // primary to secondary
        // 1. Optional arbitrary code that uses neither this nor super.
        if (a == '1') {
            throw new Error('1')
        }
        // 2. Mandatory call to a superconstructor
        super(a, 1)
        // 3. Mandatory execution of field initializers (if any) in the order they appear in
        // a class body implicitly added by the compiler.
        // 4. Optional arbitrary code that avoids usage of non-initialized fields.
        if (a == '2') {
            throw new Error('2')
        }
        // 5. Optional code that ensures all object fields to be initialized.
        arktest.assertEQ(this.f00, null)
        arktest.assertEQ(this.f01, 1)
        arktest.assertEQ(this.f02, 2)
        arktest.assertEQ(this.f03, 3)
        arktest.assertEQ(this.f04, 4)
        arktest.assertEQ(this.f05, 1.5f)
        arktest.assertEQ(this.f06, 2.5)
        arktest.assertEQ(this.f07, true)
        arktest.assertEQ(this.f08, '3.14')
        arktest.assertEQ(this.f09, 111111111111111111111111n)
        arktest.assertEQ(this.f10.length, 3)
        arktest.assertTrue(this.f10[0] instanceof Object)
        arktest.assertEQ(this.f10[1], 'b')
        arktest.assertEQ(this.f10[2], 333n)
        arktest.assertEQ(this.f11.length, 3)
        arktest.assertEQ(this.f11[0], 42)
        arktest.assertEQ(this.f11[1], 43)
        arktest.assertEQ(this.f11[2], 44)
        arktest.assertEQ(this.f12[0], '12')
        arktest.assertEQ(this.f12[1], 12)
        arktest.assertEQ(this.f13[1], '1')
        arktest.assertEQ(this.f13[2], '2')
        arktest.assertEQ(this.f14(2.0), '2')
        // 6. Optional arbitrary code.
        if (a == '3') {
            throw new Error('3')
        }
    }

    constructor(a: string, b: int) { // primary to secondary
        // 1. Optional arbitrary code that uses neither this nor super.
        if (a == '4') {
            throw new Error('4')
        }
        // 2. Mandatory call to a superconstructor
        super(a, 1)
        // 3. Mandatory execution of field initializers (if any) in the order they appear in
        // a class body implicitly added by the compiler.
        // 4. Optional arbitrary code that avoids usage of non-initialized fields.
        if (a == '5') {
            throw new Error('5')
        }
        // 5. Optional code that ensures all object fields to be initialized.
        arktest.assertEQ(this.f00, null)
        arktest.assertEQ(this.f01, 1)
        arktest.assertEQ(this.f02, 2)
        arktest.assertEQ(this.f03, 3)
        arktest.assertEQ(this.f04, 4)
        arktest.assertEQ(this.f05, 1.5f)
        arktest.assertEQ(this.f06, 2.5)
        arktest.assertEQ(this.f07, true)
        arktest.assertEQ(this.f08, '3.14')
        arktest.assertEQ(this.f09, 111111111111111111111111n)
        arktest.assertEQ(this.f10.length, 3)
        arktest.assertTrue(this.f10[0] instanceof Object)
        arktest.assertEQ(this.f10[1], 'b')
        arktest.assertEQ(this.f10[2], 333n)
        arktest.assertEQ(this.f11.length, 3)
        arktest.assertEQ(this.f11[0], 42)
        arktest.assertEQ(this.f11[1], 43)
        arktest.assertEQ(this.f11[2], 44)
        arktest.assertEQ(this.f12[0], '12')
        arktest.assertEQ(this.f12[1], 12)
        arktest.assertEQ(this.f13[1], '1')
        arktest.assertEQ(this.f13[2], '2')
        arktest.assertEQ(this.f14(2.0), '2')
        // 6. Optional arbitrary code.
        if (a == '6') {
            throw new Error('6')
        }
    }
}

function main() {
    // call 1st primary constructor
    let b: B = new B('x')
    arktest.expectError(() => { b = new B('1') }, '1')
    arktest.expectError(() => { b = new B('2') }, '2')
    arktest.expectError(() => { b = new B('3') }, '3')

    // call 2nd primary constructor
    b = new B('x', 1)
    arktest.expectError(() => { b = new B('4', 1) }, '4')
    arktest.expectError(() => { b = new B('5', 1) }, '5')
    arktest.expectError(() => { b = new B('6', 1) }, '6')
}

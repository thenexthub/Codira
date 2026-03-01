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

function istrue(v: Any) { return !!v }

function erase<T>(v: T): Any {
    return v;
}

class Klass {
    public f: number;

    constructor(f:number) {
        this.f = f;
    }
}

function foo(n: number): Klass | undefined {
    if (n == 0) {
        return undefined;
    }
    return new Klass(n);
}

enum Color {
    Red,
    Green,
    Blue = 2147483647,
    Yellow = -2147483648,
}

enum Color64 {
    Red,
    Green,
    Blue = 9223372036854775807,
    Yellow = -9223372036854775808,
}

enum ColorString {
    Red = '',
    Green = 'Green'
}

function testNormal() {
    arktest.assertTrue(!istrue(new Byte(0 as byte)));
    arktest.assertTrue(istrue(new Byte(1 as byte)));
    arktest.assertTrue(!istrue(new Short(0 as short)));
    arktest.assertTrue(istrue(new Short(1 as short)));
    arktest.assertTrue(!istrue(new Int(0)));
    arktest.assertTrue(istrue(new Int(1)));
    arktest.assertTrue(!istrue(new Long(0)));
    arktest.assertTrue(istrue(new Long(1)));
    arktest.assertTrue(!istrue(new Float(0.0)));
    arktest.assertTrue(istrue(new Float(0.5)));
    arktest.assertTrue(!istrue(new Double(0.0)));
    arktest.assertTrue(istrue(new Double(0.5)));
    arktest.assertTrue(!istrue(new Char(c'\0')));
    arktest.assertTrue(istrue(new Char(c'0')));
    arktest.assertTrue(!istrue(''));
    arktest.assertTrue(istrue('Hello'));
    arktest.assertTrue(!istrue(false));
    arktest.assertTrue(istrue(true));
    arktest.assertTrue(!istrue(null));
    arktest.assertTrue(!istrue(undefined));
    arktest.assertTrue(!istrue(NaN));
    arktest.assertTrue(!istrue(new bigint(0)));
    arktest.assertTrue(istrue(new bigint(1)));
    arktest.assertTrue(!istrue(foo(0)));
    arktest.assertTrue(istrue(foo(1)));
    arktest.assertTrue(!istrue(Color.Red));
    arktest.assertTrue(istrue(Color.Green));
    arktest.assertTrue(istrue(Color.Blue));
    arktest.assertTrue(istrue(Color.Yellow));
    arktest.assertTrue(!istrue(Color64.Red));
    arktest.assertTrue(istrue(Color64.Green));
    arktest.assertTrue(istrue(Color64.Blue));
    arktest.assertTrue(istrue(Color64.Yellow));
    arktest.assertTrue(!istrue(ColorString.Red));
    arktest.assertTrue(istrue(ColorString.Green));
    let a: bigint | null = 0n;
    arktest.assertTrue(istrue(!a));
}

function testErase() {
    arktest.assertTrue(!istrue(erase(new Byte(0 as byte))));
    arktest.assertTrue(istrue(erase(new Byte(1 as byte))));
    arktest.assertTrue(!istrue(erase(new Short(0 as short))));
    arktest.assertTrue(istrue(erase(new Short(1 as short))));
    arktest.assertTrue(!istrue(erase(new Int(0))));
    arktest.assertTrue(istrue(erase(new Int(1))));
    arktest.assertTrue(!istrue(erase(new Long(0))));
    arktest.assertTrue(istrue(erase(new Long(1))));
    arktest.assertTrue(!istrue(erase(new Float(0.0))));
    arktest.assertTrue(istrue(erase(new Float(0.5))));
    arktest.assertTrue(!istrue(erase(new Double(0.0))));
    arktest.assertTrue(istrue(erase(new Double(0.5))));
    arktest.assertTrue(!istrue(erase(new Char(c'\0'))));
    arktest.assertTrue(istrue(erase(new Char(c'0'))));
    arktest.assertTrue(!istrue(erase('')));
    arktest.assertTrue(istrue(erase('Hello')));
    arktest.assertTrue(!istrue(erase(false)));
    arktest.assertTrue(istrue(erase(true)));
    arktest.assertTrue(!istrue(erase(null)));
    arktest.assertTrue(!istrue(erase(undefined)));
    arktest.assertTrue(!istrue(erase(NaN)));
    arktest.assertTrue(!istrue(erase(new bigint(0))));
    arktest.assertTrue(istrue(erase(new bigint(1))));
    arktest.assertTrue(!istrue(erase(foo(0))));
    arktest.assertTrue(istrue(erase(foo(1))));
    arktest.assertTrue(!istrue(erase(Color.Red)));
    arktest.assertTrue(istrue(erase(Color.Green)));
    arktest.assertTrue(istrue(erase(Color.Blue)));
    arktest.assertTrue(istrue(erase(Color.Yellow)));
    arktest.assertTrue(!istrue(erase(Color64.Red)));
    arktest.assertTrue(istrue(erase(Color64.Green)));
    arktest.assertTrue(istrue(erase(Color64.Blue)));
    arktest.assertTrue(istrue(erase(Color64.Yellow)));
    arktest.assertTrue(!istrue(erase(ColorString.Red)));
    arktest.assertTrue(istrue(erase(ColorString.Green)));
    let a: bigint | null = 0n;
    arktest.assertTrue(istrue(erase(!a)));
}

function testConditions() {
    if (istrue(0)) {
        arktest.assertTrue(false);
    } else {
        arktest.assertTrue(true);
    }

    if (istrue(1)) {
        arktest.assertTrue(true);
    } else {
        arktest.assertTrue(false);
    }
}


function testFunction() {
    if (istrue((() => { let x: number = 0; return x })())) {
        arktest.assertTrue(false);
    } else {
        arktest.assertTrue(true);
    }
}

function testMemoryAllocationsWithIsTrue() {
    const totalAllocations = 10000;
    let allocations: Array<Object> = new Array<Object>(totalAllocations);
    let count = 0;

    for (let i = 0; i < totalAllocations; i++) {
        allocations[i] = new ArrayBuffer(1024);
        if (istrue(erase(new Int(1)))) {
            count++
        }
    }
    arktest.assertEQ(count, totalAllocations);
}

function main() {
    testNormal();
    testErase();
    testConditions();
    testFunction();
    testMemoryAllocationsWithIsTrue();
}

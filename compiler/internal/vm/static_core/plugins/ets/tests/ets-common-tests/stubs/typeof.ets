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

function erase<T>(v: T): Any {
    return v;
}

function foo(v: int) {
    return v
};

function foo2(v: int | object) {
    return v
};

function testNumerics() {
    arktest.assertEQ(typeof (() => { let v: byte = 1; return v })(), "byte");
    arktest.assertEQ(typeof (() => { let v: short = 1; return v })(), "short");
    arktest.assertEQ(typeof (() => { let v: int = 1; return v })(), "int");
    arktest.assertEQ(typeof (() => { let v: long = 123456789123; return v })(), "long");
    arktest.assertEQ(typeof (() => { let v: float = 1.1f; return v })(), "float");
    arktest.assertEQ(typeof (() => { let v: double = 1.1; return v })(), "number");
    arktest.assertEQ(typeof (() => { let v: char = c'a'; return v })(), "char");
}

function testNumerics2() {
    let a1: byte = 1;
    let a2: short = 1;
    let a3: int = 1;
    let a4: long = 123456789123;
    let a5: float = 1.1f;
    let a6: double = 1.1;
    let a7: char = c'a';
    arktest.assertEQ(typeof erase(a1), "byte");
    arktest.assertEQ(typeof erase(a2), "short");
    arktest.assertEQ(typeof erase(a3), "int");
    arktest.assertEQ(typeof erase(a4), "long");
    arktest.assertEQ(typeof erase(a5), "float");
    arktest.assertEQ(typeof erase(a6), "number");
    arktest.assertEQ(typeof erase(a7), "char");
}

function testString() {
    arktest.assertEQ(typeof (() => { let v: string = "Hello World"; return v })(), "string");
    arktest.assertEQ(typeof (() => { let v: String = "Hello World"; return v })(), "string");
    arktest.assertEQ(typeof erase(new string("Hello World")), "string");
    arktest.assertEQ(typeof erase(new String("Hello World")), "string");
}

function testBoolean() {
    arktest.assertEQ(typeof (() => { let v: boolean = true; return v })(), "boolean");
    arktest.assertEQ(typeof (() => { let v: Boolean = false; return v })(), "boolean");
    arktest.assertEQ(typeof erase(true), "boolean");
    arktest.assertEQ(typeof erase(false), "boolean");
}

function testBigInt() {
    arktest.assertEQ(typeof (() => { let v: BigInt = new BigInt(10000000000000); return v })(), "bigint");
    arktest.assertEQ(typeof (() => { let v: bigint = new bigint(99999999999999); return v })(), "bigint");
    arktest.assertEQ(typeof erase(new BigInt(10000000000000)), "bigint");
    arktest.assertEQ(typeof erase(new bigint(99999999999999)), "bigint");
}

function testObject() {
    let obj1: Object = {};
    let obj2: Object = [1, 2, 3];
    let obj3: Object = () => { console.log("Hello World") };
    let obj4: Object = new Date();
    let obj5: Object = 123;

    arktest.assertEQ(typeof obj1, "object");
    arktest.assertEQ(typeof obj2, "object");
    arktest.assertEQ(typeof obj3, "function");
    arktest.assertEQ(typeof obj4, "object");
    arktest.assertEQ(typeof obj5, "int");

    arktest.assertEQ(typeof erase(obj1), "object");
    arktest.assertEQ(typeof erase(obj2), "object");
    arktest.assertEQ(typeof erase(obj3), "function");
    arktest.assertEQ(typeof erase(obj4), "object");
    arktest.assertEQ(typeof erase(obj5), "int");
}

let f1: () => void = () => {
    console.log("Hello World");
}

function normalFuction() {
    console.log("Hello World");
}

function testFunction() {
    let f2 = normalFuction;
    arktest.assertEQ(typeof f1, "function");
    arktest.assertEQ(typeof f2, "function");

    arktest.assertEQ(typeof erase(f1), "function");
    arktest.assertEQ(typeof erase(f2), "function");
}

function testUndefined() {
    let v = undefined;
    arktest.assertEQ(typeof v, "undefined");
    arktest.assertEQ(typeof erase(v), "undefined");
}

function testNull() {
    let v = null;
    arktest.assertEQ(typeof v, "object");
    arktest.assertEQ(typeof erase(v), "object");
}

enum Direction {
    Up = 1,
    Down,
    left,
    Right
}

enum Status {
    Active = "ACTIVE",
    Inactive = "INACTIVE",
    Pending = "PENDING"
}

function testEnum() {
    let dir: Direction = Direction.Up;
    let status: Status = Status.Active;
    arktest.assertEQ(typeof dir, "int");
    arktest.assertEQ(typeof status, "string");
    // The enum is wrapped as an object type when calling erase
    arktest.assertEQ(typeof erase(dir), "int");
    arktest.assertEQ(typeof erase(status), "string");
    arktest.assertEQ(typeof foo(dir), "int");
    // The union type will convert all its members into object
    arktest.assertEQ(typeof foo2(dir), "int");
}

let v1: string | number | boolean = "abc";
let v2: string | number | null | undefined = 123;

function testUnion() {
    arktest.assertEQ(typeof v1, "string");
    arktest.assertEQ(typeof v2, "number");
    arktest.assertEQ(typeof erase(v1), "string");
    arktest.assertEQ(typeof erase(v2), "number");
}

class A implements Function {
    unsafeCall(...r: FixedArray<Any>): Any { return null }
    get name() {
        return "A";
    }
}
class B extends Numeric {
    public toByte(): byte { return 1 };
    public toInt(): int { return 1 };
    public toShort(): short { return 1 };
    public toLong(): long { return 1 };
    public toFloat(): float { return 1.1f };
    public toDouble(): double { return 1.1 };

    public byteValue(): byte { return this.toByte() };
    public intValue(): int { return this.toInt() };
    public shortValue(): short { return this.toShort() };
    public longValue(): long { return this.toLong() };
    public floatValue(): float { return this.toFloat() };
    public doubleValue(): double { return this.toDouble() };
}

function testClass() {
    let a = new A();
    let b = new B();
    arktest.assertEQ(typeof a, "function");
    arktest.assertEQ(typeof b, "object");
    arktest.assertEQ(typeof erase(a), "function");
    arktest.assertEQ(typeof erase(b), "object");
}

interface User {
    id: number;
}

function testInterface() {
    let u: User = {
        id: 1
    }
    arktest.assertEQ(typeof u, "object");
    arktest.assertEQ(typeof erase(u), "object");
}

function main() {
    testNumerics();
    testNumerics2();
    testString();
    testBoolean();
    testBigInt();
    testObject();
    testFunction();
    testUndefined();
    testNull();
    testEnum();
    testUnion();
    testClass();
    testInterface();
}

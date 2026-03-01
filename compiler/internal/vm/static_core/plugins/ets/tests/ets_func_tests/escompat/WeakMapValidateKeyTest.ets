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

/**
 * Test that WeakMap correctly rejects primitive types as keys
 * and only accepts objects, as per ECMAScript specification.
 */

class TestObject {
    value: int;
    constructor(val: int) {
        this.value = val;
    }
}

function testWeakMapWithObjectKey(): void {
    const map = new WeakMap<Object, string>();
    const key = new TestObject(42);
    
    // Should work fine with object keys - no exception expected
    arktest.expectNoThrow(() => {
        map.set(key, "value");
    });
    
    arktest.assertTrue(map.has(key), "WeakMap should accept object keys");
    arktest.assertEQ(map.get(key), "value", "WeakMap should return correct value for object key");
}

function testWeakMapRejectsBooleanKey(): void {
    const map = new WeakMap<Object, string>();
    
    arktest.expectThrow(
        () => { map.set(new Boolean(true), "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsNumberKeys(): void {
    const map = new WeakMap<Object, string>();
    
    // Test Int key rejection
    arktest.expectThrow(
        () => { map.set(new Int(42), "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    // Test Long key rejection
    arktest.expectThrow(
        () => { map.set(new Long(100), "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    // Test Double key rejection
    arktest.expectThrow(
        () => { map.set(new Double(3.14), "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsStringKey(): void {
    const map = new WeakMap<Object, string>();
    
    arktest.expectThrow(
        () => { map.set(new String("key"), "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsMoreBoxedTypes(): void {
    const map = new WeakMap<Object, string>();
    
    // Test Byte key rejection
    arktest.expectThrow(
        () => { map.set(new Byte(1), "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    // Test Short key rejection
    arktest.expectThrow(
        () => { map.set(new Short(100), "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    // Test Float key rejection
    arktest.expectThrow(
        () => { map.set(new Float(2.5), "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    // Test Char key rejection
    arktest.expectThrow(
        () => { map.set(new Char(c'A'), "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsBigInt(): void {
    const map = new WeakMap<Object, string>();
    
    // BigInt should also be rejected as it's a value type
    arktest.expectThrow(
        () => { map.set(new BigInt(12345), "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapAcceptsMultipleObjectTypes(): void {
    const map = new WeakMap<Object, Int>();
    
    // Test with custom class
    const obj1 = new TestObject(1);
    arktest.expectNoThrow(() => { map.set(obj1, new Int(10)); });
    
    // Test with Array
    const arr = new Array<int>(1, 2, 3);
    arktest.expectNoThrow(() => { map.set(arr, new Int(20)); });
    
    // Verify values are stored correctly
    arktest.assertEQ(map.get(obj1)!.toInt(), 10);
    arktest.assertEQ(map.get(arr)!.toInt(), 20);
}

function testWeakMapRejectsLiteralIntegerKey(): void {
    const map = new WeakMap<Object, string>();
    
    // Test literal integer as key - should be rejected
    // Note: In ETS, literals are auto-boxed when used as Object type
    arktest.expectThrow(
        () => { map.set(1234, "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(0, "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(-999, "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsLiteralDoubleKey(): void {
    const map = new WeakMap<Object, string>();
    
    // Test literal double/float as key - should be rejected
    arktest.expectThrow(
        () => { map.set(1.2, "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(3.14159, "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(-0.5, "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsLiteralStringKey(): void {
    const map = new WeakMap<Object, string>();
    
    // Test literal string as key - should be rejected
    arktest.expectThrow(
        () => { map.set("abc", "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set("test string", "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set("", "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsLiteralBooleanKey(): void {
    const map = new WeakMap<Object, string>();
    
    // Test literal boolean as key - should be rejected
    arktest.expectThrow(
        () => { map.set(true, "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(false, "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsLiteralCharKey(): void {
    const map = new WeakMap<Object, string>();
    
    // Test literal char as key - should be rejected
    arktest.expectThrow(
        () => { map.set(c'A', "value"); },
        (e: Error) => e instanceof TypeError
    );
    
    arktest.expectThrow(
        () => { map.set(c'Z', "value"); },
        (e: Error) => e instanceof TypeError
    );
}

function testWeakMapRejectsAllLiteralPrimitives(): void {
    const map = new WeakMap<Object, string>();
    
    // Test various literal primitives in one comprehensive test
    const literalValues: Object[] = [
        42,           // int literal
        1234,         // int literal
        1.5,          // double literal
        true,         // boolean literal
        false,        // boolean literal
        "text",       // string literal
        c'X'          // char literal
    ];
    
    for (let i = 0; i < literalValues.length; i++) {
        arktest.expectThrow(
            () => { map.set(literalValues[i], "value"); },
            (e: Error) => e instanceof TypeError
        );
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("WeakMapValidateKeyTest");
    
    // Test with boxed primitive types
    suite.addTest("Valid object keys", testWeakMapWithObjectKey);
    suite.addTest("Reject Boolean key", testWeakMapRejectsBooleanKey);
    suite.addTest("Reject number keys (Int/Long/Double)", testWeakMapRejectsNumberKeys);
    suite.addTest("Reject String key", testWeakMapRejectsStringKey);
    suite.addTest("Reject other boxed types (Byte/Short/Float/Char)", testWeakMapRejectsMoreBoxedTypes);
    suite.addTest("Reject BigInt key", testWeakMapRejectsBigInt);
    suite.addTest("Accept multiple object types", testWeakMapAcceptsMultipleObjectTypes);
    
    // Test with literal primitive values (natural values)
    suite.addTest("Reject literal integer key (1234)", testWeakMapRejectsLiteralIntegerKey);
    suite.addTest("Reject literal double key (1.2)", testWeakMapRejectsLiteralDoubleKey);
    suite.addTest("Reject literal string key ('abc')", testWeakMapRejectsLiteralStringKey);
    suite.addTest("Reject literal boolean key (true/false)", testWeakMapRejectsLiteralBooleanKey);
    suite.addTest("Reject literal char key ('A')", testWeakMapRejectsLiteralCharKey);
    suite.addTest("Reject all literal primitives (comprehensive)", testWeakMapRejectsAllLiteralPrimitives);
    
    return suite.run();
}


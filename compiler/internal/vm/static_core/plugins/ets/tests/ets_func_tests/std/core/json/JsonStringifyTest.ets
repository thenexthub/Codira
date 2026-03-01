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

import { jsonx } from "std/core"

function main(): int {
    const suite = new arktest.ArkTestsuite("JSON.stringifyJsonElement (JsonElement)");

    // Basic type tests
    suite.addTest("Stringify primitive types", testStringifyPrimitives);
    suite.addTest("Stringify null and undefined", testStringifyNullUndefined);
    suite.addTest("Stringify boolean values", testStringifyBooleans);
    suite.addTest("Stringify numbers", testStringifyNumbers);
    suite.addTest("Stringify strings", testStringifyStrings);

    // String escaping tests
    suite.addTest("Stringify string escaping", testStringifyStringEscaping);
    suite.addTest("Stringify control characters", testStringifyControlCharacters);
    suite.addTest("Stringify unicode escaping", testStringifyUnicodeEscaping);

    // Container type tests
    suite.addTest("Stringify empty containers", testStringifyEmptyContainers);
    suite.addTest("Stringify arrays", testStringifyArrays);
    suite.addTest("Stringify objects", testStringifyObjects);
    suite.addTest("Stringify nested structures", testStringifyNestedStructures);

    // Formatting tests
    suite.addTest("Stringify with numeric space", testStringifyWithNumericSpace);
    suite.addTest("Stringify with string space", testStringifyWithStringSpace);
    suite.addTest("Stringify space edge cases", testStringifySpaceEdgeCases);
    suite.addTest("Stringify complex formatting", testStringifyComplexFormatting);

    // Edge cases and complex scenarios
    suite.addTest("Stringify mixed type arrays", testStringifyMixedArrays);
    suite.addTest("Stringify complex nested objects", testStringifyComplexObjects);
    suite.addTest("Stringify deeply nested structures", testStringifyDeeplyNested);

    return suite.run();
}

function testStringifyPrimitives(): void {
    // Test basic primitive types
    const stringElem = jsonx.JsonElement.createString("hello");
    arktest.assertEQ(JSON.stringifyJsonElement(stringElem), '"hello"', "String should be quoted");

    const intElem = jsonx.JsonElement.createInteger(42);
    arktest.assertEQ(JSON.stringifyJsonElement(intElem), "42", "Integer should be stringified without quotes");

    const doubleElem = jsonx.JsonElement.createDouble(3.14);
    arktest.assertEQ(JSON.stringifyJsonElement(doubleElem), "3.14", "Double should be stringified without quotes");

    const trueElem = jsonx.JsonElement.createBoolean(true);
    arktest.assertEQ(JSON.stringifyJsonElement(trueElem), "true", "Boolean true should be stringified as 'true'");

    const falseElem = jsonx.JsonElement.createBoolean(false);
    arktest.assertEQ(JSON.stringifyJsonElement(falseElem), "false", "Boolean false should be stringified as 'false'");
}

function testStringifyNullUndefined(): void {
    const nullElem = jsonx.JsonElement.createNull();
    arktest.assertEQ(JSON.stringifyJsonElement(nullElem), "null", "Null should be stringified as 'null'");

    const undefinedElem = jsonx.JsonElement.createUndefined();
    arktest.assertEQ(JSON.stringifyJsonElement(undefinedElem), "undefined", "Undefined should be stringified as 'undefined'");
}

function testStringifyBooleans(): void {
    const trueElem = jsonx.JsonElement.createBoolean(true);
    const falseElem = jsonx.JsonElement.createBoolean(false);

    arktest.assertEQ(JSON.stringifyJsonElement(trueElem), "true", "True boolean should stringify correctly");
    arktest.assertEQ(JSON.stringifyJsonElement(falseElem), "false", "False boolean should stringify correctly");

    // Test in arrays
    const arrayElem = jsonx.JsonElement.createArray([trueElem, falseElem]);
    arktest.assertEQ(JSON.stringifyJsonElement(arrayElem), "[true,false]", "Boolean array should stringify correctly");
}

function testStringifyNumbers(): void {
    // Test various number formats
    const zero = jsonx.JsonElement.createInteger(0);
    arktest.assertEQ(JSON.stringifyJsonElement(zero), "0", "Zero should stringify correctly");

    const negative = jsonx.JsonElement.createInteger(-42);
    arktest.assertEQ(JSON.stringifyJsonElement(negative), "-42", "Negative integer should stringify correctly");

    const largeInt = jsonx.JsonElement.createInteger(1000000);
    arktest.assertEQ(JSON.stringifyJsonElement(largeInt), "1000000", "Large integer should stringify correctly");

    const smallDouble = jsonx.JsonElement.createDouble(0.001);
    arktest.assertEQ(JSON.stringifyJsonElement(smallDouble), "0.001", "Small double should stringify correctly");

    const negativeDouble = jsonx.JsonElement.createDouble(-3.14159);
    arktest.assertEQ(JSON.stringifyJsonElement(negativeDouble), "-3.14159", "Negative double should stringify correctly");
}

function testStringifyStrings(): void {
    const emptyStr = jsonx.JsonElement.createString("");
    arktest.assertEQ(JSON.stringifyJsonElement(emptyStr), '""', "Empty string should be quoted");

    const simpleStr = jsonx.JsonElement.createString("hello world");
    arktest.assertEQ(JSON.stringifyJsonElement(simpleStr), '"hello world"', "Simple string should be quoted");

    const numberStr = jsonx.JsonElement.createString("123");
    arktest.assertEQ(JSON.stringifyJsonElement(numberStr), '"123"', "Numeric string should be quoted");

    const spaceStr = jsonx.JsonElement.createString("   ");
    arktest.assertEQ(JSON.stringifyJsonElement(spaceStr), '"   "', "Whitespace string should be quoted");
}

function testStringifyStringEscaping(): void {
    // Test special character escaping
    const quoteStr = jsonx.JsonElement.createString('He said "Hello"');
    arktest.assertEQ(JSON.stringifyJsonElement(quoteStr), '"He said \\"Hello\\""', "Quotes should be escaped");

    const backslashStr = jsonx.JsonElement.createString("path\\to\\file");
    arktest.assertEQ(JSON.stringifyJsonElement(backslashStr), '"path\\\\to\\\\file"', "Backslashes should be escaped");

    const mixedStr = jsonx.JsonElement.createString('Say "Hi" and use \\ char');
    arktest.assertEQ(JSON.stringifyJsonElement(mixedStr), '"Say \\"Hi\\" and use \\\\ char"', "Mixed escaping should work");
}

function testStringifyControlCharacters(): void {
    // Test control character escaping
    const newlineStr = jsonx.JsonElement.createString("line1\nline2");
    arktest.assertEQ(JSON.stringifyJsonElement(newlineStr), '"line1\\nline2"', "Newline should be escaped");

    const tabStr = jsonx.JsonElement.createString("col1\tcol2");
    arktest.assertEQ(JSON.stringifyJsonElement(tabStr), '"col1\\tcol2"', "Tab should be escaped");

    const carriageReturnStr = jsonx.JsonElement.createString("text\rmore");
    arktest.assertEQ(JSON.stringifyJsonElement(carriageReturnStr), '"text\\rmore"', "Carriage return should be escaped");

    const backspaceStr = jsonx.JsonElement.createString("back\bspace");
    arktest.assertEQ(JSON.stringifyJsonElement(backspaceStr), '"back\\bspace"', "Backspace should be escaped");

    const formfeedStr = jsonx.JsonElement.createString("form\ffeed");
    arktest.assertEQ(JSON.stringifyJsonElement(formfeedStr), '"form\\ffeed"', "Form feed should be escaped");
}

function testStringifyUnicodeEscaping(): void {
    // Test unicode control character escaping (characters with code < 32)
    const controlChar = jsonx.JsonElement.createString("test\u0001end");
    arktest.assertEQ(JSON.stringifyJsonElement(controlChar), '"test\\u0001end"', "Control character should be unicode escaped");

    const nullChar = jsonx.JsonElement.createString("null\u0000char");
    arktest.assertEQ(JSON.stringifyJsonElement(nullChar), '"null\\u0000char"', "Null character should be unicode escaped");

    const deleteChar = jsonx.JsonElement.createString("del\u001fchar");
    arktest.assertEQ(JSON.stringifyJsonElement(deleteChar), '"del\\u001fchar"', "Delete character should be unicode escaped");
}

function testStringifyEmptyContainers(): void {
    const emptyArray = jsonx.JsonElement.createArray([]);
    arktest.assertEQ(JSON.stringifyJsonElement(emptyArray), "[]", "Empty array should stringify as []");

    const emptyObject = jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>());
    arktest.assertEQ(JSON.stringifyJsonElement(emptyObject), "{}", "Empty object should stringify as {}");
}

function testStringifyArrays(): void {
    // Single element array
    const singleArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("item")
    ]);
    arktest.assertEQ(JSON.stringifyJsonElement(singleArray), '["item"]', "Single element array should stringify correctly");

    // Multi-element array
    const multiArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("first"),
        jsonx.JsonElement.createInteger(42),
        jsonx.JsonElement.createBoolean(true)
    ]);
    arktest.assertEQ(JSON.stringifyJsonElement(multiArray), '["first",42,true]', "Multi-element array should stringify correctly");

    // Array with null
    const nullArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("before"),
        jsonx.JsonElement.createNull(),
        jsonx.JsonElement.createString("after")
    ]);
    arktest.assertEQ(JSON.stringifyJsonElement(nullArray), '["before",null,"after"]', "Array with null should stringify correctly");
}

function testStringifyObjects(): void {
    // Single property object
    const map1 = new Map<string, jsonx.JsonElement>();
    map1.set("name", jsonx.JsonElement.createString("John"));
    const singleObj = jsonx.JsonElement.createObject(map1);
    arktest.assertEQ(JSON.stringifyJsonElement(singleObj), '{"name":"John"}', "Single property object should stringify correctly");

    // Multi-property object
    const map2 = new Map<string, jsonx.JsonElement>();
    map2.set("name", jsonx.JsonElement.createString("Alice"));
    map2.set("age", jsonx.JsonElement.createInteger(30));
    map2.set("active", jsonx.JsonElement.createBoolean(true));
    const multiObj = jsonx.JsonElement.createObject(map2);
    const result = JSON.stringifyJsonElement(multiObj);

    // Since Map iteration order might vary, check that all parts are present
    arktest.assertTrue(result.includes('"name":"Alice"'), "Object should contain name property");
    arktest.assertTrue(result.includes('"age":30'), "Object should contain age property");
    arktest.assertTrue(result.includes('"active":true'), "Object should contain active property");
    arktest.assertTrue(result.startsWith('{') && result.endsWith('}'), "Object should be wrapped in braces");
    arktest.assertFalse(result.includes("\n"), "Object should not contain newlines");
    arktest.assertEQ(result.length, 39, "Object should be 39 characters long");

    const root = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>)
    const unionElmt = new jsonx.JsonElement()
    unionElmt.setUndefined()
    root.setElement("union", unionElmt)
    arktest.assertEQ(JSON.stringifyJsonElement(root), "{}", "Undefined object should not stringify")
}

function testStringifyNestedStructures(): void {
    // Array containing objects
    const innerObj1 = new Map<string, jsonx.JsonElement>();
    innerObj1.set("id", jsonx.JsonElement.createInteger(1));
    innerObj1.set("name", jsonx.JsonElement.createString("Item1"));

    const innerObj2 = new Map<string, jsonx.JsonElement>();
    innerObj2.set("id", jsonx.JsonElement.createInteger(2));
    innerObj2.set("name", jsonx.JsonElement.createString("Item2"));

    const arrayWithObjects = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createObject(innerObj1),
        jsonx.JsonElement.createObject(innerObj2)
    ]);

    const result1 = JSON.stringifyJsonElement(arrayWithObjects);
    arktest.assertTrue(result1.startsWith('[') && result1.endsWith(']'), "Array should be wrapped in brackets");
    arktest.assertTrue(result1.includes('"id":1'), "Array should contain first object data");
    arktest.assertTrue(result1.includes('"id":2'), "Array should contain second object data");

    // Object containing arrays
    const scores = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createInteger(85),
        jsonx.JsonElement.createInteger(92),
        jsonx.JsonElement.createInteger(78)
    ]);

    const outerMap = new Map<string, jsonx.JsonElement>();
    outerMap.set("name", jsonx.JsonElement.createString("Student"));
    outerMap.set("scores", scores);
    const objWithArray = jsonx.JsonElement.createObject(outerMap);

    const result2 = JSON.stringifyJsonElement(objWithArray);
    arktest.assertTrue(result2.includes('"scores":[85,92,78]'), "Object should contain array property");
}

function testStringifyWithNumericSpace(): void {
    const obj = new Map<string, jsonx.JsonElement>();
    obj.set("a", jsonx.JsonElement.createInteger(1));
    obj.set("b", jsonx.JsonElement.createInteger(2));
    const elem = jsonx.JsonElement.createObject(obj);

    // Test with space = 2
    const result2 = JSON.stringifyJsonElement(elem, undefined, 2);
    arktest.assertTrue(result2.includes('\n'), "Formatted JSON should contain newlines");
    arktest.assertTrue(result2.includes('  '), "Formatted JSON should contain 2-space indentation");

    // Test with space = 0 (no formatting)
    const result0 = JSON.stringifyJsonElement(elem, undefined, 0);
    arktest.assertFalse(result0.includes('\n'), "Space=0 should not add newlines");

    // Test with space = 4
    const result4 = JSON.stringifyJsonElement(elem, undefined, 4);
    arktest.assertTrue(result4.includes('    '), "Space=4 should use 4-space indentation");

    // Test space limit (max 10)
    const result15 = JSON.stringifyJsonElement(elem, undefined, 15);
    const result10 = JSON.stringifyJsonElement(elem, undefined, 10);
    arktest.assertEQ(result15, result10, "Space > 10 should be clamped to 10");

    // Test negative space (should be 0)
    const resultNeg = JSON.stringifyJsonElement(elem, undefined, -5);
    arktest.assertEQ(resultNeg, result0, "Negative space should be treated as 0");
}

function testStringifyWithStringSpace(): void {
    const arr = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createInteger(1),
        jsonx.JsonElement.createInteger(2)
    ]);

    // Test with tab character
    const resultTab = JSON.stringifyJsonElement(arr, undefined, "\t");
    arktest.assertTrue(resultTab.includes('\t'), "String space should use tab character");

    // Test with custom string
    const resultCustom = JSON.stringifyJsonElement(arr, undefined, "🔥");
    arktest.assertTrue(resultCustom.includes('🔥'), "String space should use custom string");

    // Test string length limit (max 10 characters)
    const longString = "1234567890abcdef";
    const resultLong = JSON.stringifyJsonElement(arr, undefined, longString);
    const resultTruncated = JSON.stringifyJsonElement(arr, undefined, "1234567890");
    arktest.assertEQ(resultLong, resultTruncated, "String space > 10 chars should be truncated");

    // Test empty string (no formatting)
    const resultEmpty = JSON.stringifyJsonElement(arr, undefined, "");
    arktest.assertFalse(resultEmpty.includes('\n'), "Empty string space should not add formatting");
}

function testStringifySpaceEdgeCases(): void {
    const simple = jsonx.JsonElement.createArray([jsonx.JsonElement.createInteger(1)]);

    // Test undefined space (no formatting)
    const resultUndef = JSON.stringifyJsonElement(simple, undefined, undefined);
    arktest.assertFalse(resultUndef.includes('\n'), "Undefined space should not add formatting");

    // Test single space
    const resultOne = JSON.stringifyJsonElement(simple, undefined, 1);
    arktest.assertTrue(resultOne.includes(' '), "Single space should add minimal indentation");
    arktest.assertTrue(!resultOne.includes('  '), "Single space should not add double space");
}

function testStringifyComplexFormatting(): void {
    // Create a complex nested structure
    const address = new Map<string, jsonx.JsonElement>();
    address.set("street", jsonx.JsonElement.createString("123 Main St"));
    address.set("city", jsonx.JsonElement.createString("Anytown"));
    address.set("zip", jsonx.JsonElement.createInteger(12345));

    const person = new Map<string, jsonx.JsonElement>();
    person.set("name", jsonx.JsonElement.createString("John Doe"));
    person.set("age", jsonx.JsonElement.createInteger(30));
    person.set("address", jsonx.JsonElement.createObject(address));
    person.set("hobbies", jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("reading"),
        jsonx.JsonElement.createString("coding")
    ]));

    const personElem = jsonx.JsonElement.createObject(person);

    // Test formatted output
    const formatted = JSON.stringifyJsonElement(personElem, undefined, 2);
    arktest.assertTrue(formatted.includes('\n'), "Complex structure should have newlines when formatted");

    // Count indentation levels - should have multiple levels of indentation
    const lines = formatted.split('\n');
    let hasLevelTwo = false;    // 4 spaces
    let hasLevelThree = false;  // 6 spaces

    for (const line of lines) {
        // Count leading spaces
        let spaceCount = 0;
        for (let i = 0; i < line.length; i++) {
            if (line.charAt(i) === c' ') {
                spaceCount++;
            } else {
                break;
            }
        }
        if (spaceCount === 2) {
            hasLevelTwo = true;
        }
        if (spaceCount === 4) {
            hasLevelThree = true;
        }
    }

    arktest.assertTrue(hasLevelTwo, "Should have 2-level indentation (2 spaces)");
    arktest.assertTrue(hasLevelThree, "Should have 3-level indentation (4 spaces) for nested objects");
}

function testStringifyMixedArrays(): void {
    // Array with all different types
    const mixedArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("text"),
        jsonx.JsonElement.createInteger(42),
        jsonx.JsonElement.createDouble(3.14),
        jsonx.JsonElement.createBoolean(true),
        jsonx.JsonElement.createBoolean(false),
        jsonx.JsonElement.createNull(),
        jsonx.JsonElement.createArray([jsonx.JsonElement.createString("nested")]),
        jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>())
    ]);

    const result = JSON.stringifyJsonElement(mixedArray);
    arktest.assertTrue(result.includes('"text"'), "Should contain string element");
    arktest.assertTrue(result.includes('42'), "Should contain integer element");
    arktest.assertTrue(result.includes('3.14'), "Should contain double element");
    arktest.assertTrue(result.includes('true'), "Should contain true element");
    arktest.assertTrue(result.includes('false'), "Should contain false element");
    arktest.assertTrue(result.includes('null'), "Should contain null element");
    arktest.assertTrue(result.includes('["nested"]'), "Should contain nested array");
    arktest.assertTrue(result.includes('{}'), "Should contain empty object");
}

function testStringifyComplexObjects(): void {
    // Create object with various property types
    const config = new Map<string, jsonx.JsonElement>();
    config.set("version", jsonx.JsonElement.createString("1.0.0"));
    config.set("maxRetries", jsonx.JsonElement.createInteger(3));
    config.set("timeout", jsonx.JsonElement.createDouble(30.5));
    config.set("enabled", jsonx.JsonElement.createBoolean(true));
    config.set("fallback", jsonx.JsonElement.createNull());

    // Add nested array
    const servers = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("server1.example.com"),
        jsonx.JsonElement.createString("server2.example.com")
    ]);
    config.set("servers", servers);

    // Add nested object
    const credentials = new Map<string, jsonx.JsonElement>();
    credentials.set("username", jsonx.JsonElement.createString("admin"));
    credentials.set("passwordHash", jsonx.JsonElement.createString("abc123"));
    config.set("auth", jsonx.JsonElement.createObject(credentials));

    const configElem = jsonx.JsonElement.createObject(config);
    const result = JSON.stringifyJsonElement(configElem);

    // Verify all components are present
    arktest.assertTrue(result.includes('"version":"1.0.0"'), "Should contain version string");
    arktest.assertTrue(result.includes('"maxRetries":3'), "Should contain maxRetries integer");
    arktest.assertTrue(result.includes('"timeout":30.5'), "Should contain timeout double");
    arktest.assertTrue(result.includes('"enabled":true'), "Should contain enabled boolean");
    arktest.assertTrue(result.includes('"fallback":null'), "Should contain fallback null");
    arktest.assertTrue(result.includes('"servers":'), "Should contain servers array");
    arktest.assertTrue(result.includes('"auth":'), "Should contain auth object");
}

function testStringifyDeeplyNested(): void {
    // Create deeply nested structure (3 levels)
    const level3 = new Map<string, jsonx.JsonElement>();
    level3.set("deep", jsonx.JsonElement.createString("value"));
    level3.set("number", jsonx.JsonElement.createInteger(123));

    const level2 = new Map<string, jsonx.JsonElement>();
    level2.set("nested", jsonx.JsonElement.createObject(level3));
    level2.set("array", jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("item1"),
        jsonx.JsonElement.createString("item2")
    ]));

    const level1 = new Map<string, jsonx.JsonElement>();
    level1.set("root", jsonx.JsonElement.createString("level1"));
    level1.set("child", jsonx.JsonElement.createObject(level2));

    const deepElem = jsonx.JsonElement.createObject(level1);

    // Test without formatting
    const compact = JSON.stringifyJsonElement(deepElem);
    arktest.assertFalse(compact.includes('\n'), "Compact stringify should not have newlines");
    arktest.assertTrue(compact.includes('"deep":"value"'), "Should preserve deeply nested values");

    // Test with formatting
    const formatted = JSON.stringifyJsonElement(deepElem, undefined, 2);
    arktest.assertTrue(formatted.includes('\n'), "Formatted stringify should have newlines");

    // Verify proper nesting levels in formatted output
    const lines = formatted.split('\n');
    let maxIndentLevel = 0;
    for (const line of lines) {
        let indentLevel = 0;
        for (let i = 0; i < line.length; i += 2) {
            if (line.substring(i, i + 2) === '  ') {
                indentLevel++;
            } else {
                break;
            }
        }
        maxIndentLevel = Math.max(maxIndentLevel, indentLevel).toInt();
    }

    arktest.assertTrue(maxIndentLevel >= 3, "Should have at least 3 levels of indentation for deep nesting");
}

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
    const suite = new arktest.ArkTestsuite("json.JSON.parse");

    // Basic type parsing tests
    suite.addTest("Parse primitive types", testParsePrimitives);
    suite.addTest("Parse null", testParseNull);
    suite.addTest("Parse boolean values", testParseBooleans);
    suite.addTest("Parse numbers", testParseNumbers);
    suite.addTest("Parse strings", testParseStrings);

    // String content tests
    suite.addTest("Parse string escaping", testParseStringEscaping);
    suite.addTest("Parse control characters", testParseControlCharacters);
    suite.addTest("Parse unicode escaping", testParseUnicodeEscaping);

    // Container type tests
    suite.addTest("Parse empty containers", testParseEmptyContainers);
    suite.addTest("Parse arrays", testParseArrays);
    suite.addTest("Parse objects", testParseObjects);
    suite.addTest("Parse nested structures", testParseNestedStructures);

    // Complex scenarios
    suite.addTest("Parse mixed type arrays", testParseMixedArrays);
    suite.addTest("Parse complex objects", testParseComplexObjects);
    suite.addTest("Parse deeply nested structures", testParseDeeplyNested);

    // Whitespace handling
    suite.addTest("Parse with whitespace", testParseWithWhitespace);

    // Error handling tests
    suite.addTest("Parse error handling", testParseErrorHandling);

    // Round-trip consistency tests
    suite.addTest("Round-trip consistency", testRoundTripConsistency);

    // Reviver function tests
    suite.addTest("Parse with reviver function", testParseWithReviver);

    return suite.run();
}

function testParsePrimitives(): void {
    // Test basic primitive types using round-trip approach
    const stringElem = jsonx.JsonElement.createString("hello");
    const stringJson = JSON.stringifyJsonElement(stringElem);
    const parsedString = JSON.parseJsonElement(stringJson);
    arktest.assertEQ(parsedString.jsonType, jsonx.JsonType.JsonString, "Parsed string should have correct type");
    arktest.assertEQ(parsedString.asString(), "hello", "Parsed string should have correct value");

    const intElem = jsonx.JsonElement.createInteger(42);
    const intJson = JSON.stringifyJsonElement(intElem);
    const parsedInt = JSON.parseJsonElement(intJson);
    arktest.assertEQ(parsedInt.jsonType, jsonx.JsonType.JsonNumber, "Parsed integer should have correct type");
    arktest.assertEQ(parsedInt.asInteger(), 42, "Parsed integer should have correct value");

    const doubleElem = jsonx.JsonElement.createDouble(3.14);
    const doubleJson = JSON.stringifyJsonElement(doubleElem);
    const parsedDouble = JSON.parseJsonElement(doubleJson);
    arktest.assertEQ(parsedDouble.jsonType, jsonx.JsonType.JsonNumber, "Parsed double should have correct type");
    arktest.assertEQ(parsedDouble.asDouble(), 3.14, "Parsed double should have correct value");
}

function testParseNull(): void {
    const nullElem = jsonx.JsonElement.createNull();
    const nullJson = JSON.stringifyJsonElement(nullElem);
    const parsedNull = JSON.parseJsonElement(nullJson);
    arktest.assertEQ(parsedNull.jsonType, jsonx.JsonType.JsonNull, "Parsed null should have correct type");
}

function testParseBooleans(): void {
    const trueElem = jsonx.JsonElement.createBoolean(true);
    const trueJson = JSON.stringifyJsonElement(trueElem);
    const parsedTrue = JSON.parseJsonElement(trueJson);
    arktest.assertEQ(parsedTrue.jsonType, jsonx.JsonType.JsonTrue, "Parsed true should have correct type");
    arktest.assertTrue(parsedTrue.asBoolean(), "Parsed true should have correct value");

    const falseElem = jsonx.JsonElement.createBoolean(false);
    const falseJson = JSON.stringifyJsonElement(falseElem);
    const parsedFalse = JSON.parseJsonElement(falseJson);
    arktest.assertEQ(parsedFalse.jsonType, jsonx.JsonType.JsonFalse, "Parsed false should have correct type");
    arktest.assertFalse(parsedFalse.asBoolean(), "Parsed false should have correct value");
}

function testParseNumbers(): void {
    // Test various number formats
    const zero = jsonx.JsonElement.createInteger(0);
    const zeroJson = JSON.stringifyJsonElement(zero);
    const parsedZero = JSON.parseJsonElement(zeroJson);
    arktest.assertEQ(parsedZero.asInteger(), 0, "Parsed zero should be correct");

    const negative = jsonx.JsonElement.createInteger(-42);
    const negativeJson = JSON.stringifyJsonElement(negative);
    const parsedNegative = JSON.parseJsonElement(negativeJson);
    arktest.assertEQ(parsedNegative.asInteger(), -42, "Parsed negative integer should be correct");

    const largeInt = jsonx.JsonElement.createInteger(1000000);
    const largeJson = JSON.stringifyJsonElement(largeInt);
    const parsedLarge = JSON.parseJsonElement(largeJson);
    arktest.assertEQ(parsedLarge.asInteger(), 1000000, "Parsed large integer should be correct");

    const smallDouble = jsonx.JsonElement.createDouble(0.001);
    const smallJson = JSON.stringifyJsonElement(smallDouble);
    const parsedSmall = JSON.parseJsonElement(smallJson);
    arktest.assertEQ(parsedSmall.asDouble(), 0.001, "Parsed small double should be correct");

    const negativeDouble = jsonx.JsonElement.createDouble(-3.14159);
    const negDoubleJson = JSON.stringifyJsonElement(negativeDouble);
    const parsedNegDouble = JSON.parseJsonElement(negDoubleJson);
    arktest.assertEQ(parsedNegDouble.asDouble(), -3.14159, "Parsed negative double should be correct");
}

function testParseStrings(): void {
    const emptyStr = jsonx.JsonElement.createString("");
    const emptyJson = JSON.stringifyJsonElement(emptyStr);
    const parsedEmpty = JSON.parseJsonElement(emptyJson);
    arktest.assertEQ(parsedEmpty.asString(), "", "Parsed empty string should be correct");

    const simpleStr = jsonx.JsonElement.createString("hello world");
    const simpleJson = JSON.stringifyJsonElement(simpleStr);
    const parsedSimple = JSON.parseJsonElement(simpleJson);
    arktest.assertEQ(parsedSimple.asString(), "hello world", "Parsed simple string should be correct");

    const numberStr = jsonx.JsonElement.createString("123");
    const numberStrJson = JSON.stringifyJsonElement(numberStr);
    const parsedNumberStr = JSON.parseJsonElement(numberStrJson);
    arktest.assertEQ(parsedNumberStr.asString(), "123", "Parsed numeric string should be correct");

    const spaceStr = jsonx.JsonElement.createString("   ");
    const spaceJson = JSON.stringifyJsonElement(spaceStr);
    const parsedSpace = JSON.parseJsonElement(spaceJson);
    arktest.assertEQ(parsedSpace.asString(), "   ", "Parsed whitespace string should be correct");
}

function testParseStringEscaping(): void {
    // Test special character escaping round-trip
    const quoteStr = jsonx.JsonElement.createString('He said "Hello"');
    const quoteJson = JSON.stringifyJsonElement(quoteStr);
    const parsedQuote = JSON.parseJsonElement(quoteJson);
    arktest.assertEQ(parsedQuote.asString(), 'He said "Hello"', "Parsed quoted string should be correct");

    const backslashStr = jsonx.JsonElement.createString("path\\to\\file");
    const backslashJson = JSON.stringifyJsonElement(backslashStr);
    const parsedBackslash = JSON.parseJsonElement(backslashJson);
    arktest.assertEQ(parsedBackslash.asString(), "path\\to\\file", "Parsed backslash string should be correct");

    const mixedStr = jsonx.JsonElement.createString('Say "Hi" and use \\ char');
    const mixedJson = JSON.stringifyJsonElement(mixedStr);
    const parsedMixed = JSON.parseJsonElement(mixedJson);
    arktest.assertEQ(parsedMixed.asString(), 'Say "Hi" and use \\ char', "Parsed mixed escaping should be correct");
}

function testParseControlCharacters(): void {
    // Test control character escaping round-trip
    const newlineStr = jsonx.JsonElement.createString("line1\nline2");
    const newlineJson = JSON.stringifyJsonElement(newlineStr);
    const parsedNewline = JSON.parseJsonElement(newlineJson);
    arktest.assertEQ(parsedNewline.asString(), "line1\nline2", "Parsed newline string should be correct");

    const tabStr = jsonx.JsonElement.createString("col1\tcol2");
    const tabJson = JSON.stringifyJsonElement(tabStr);
    const parsedTab = JSON.parseJsonElement(tabJson);
    arktest.assertEQ(parsedTab.asString(), "col1\tcol2", "Parsed tab string should be correct");

    const carriageReturnStr = jsonx.JsonElement.createString("text\rmore");
    const crJson = JSON.stringifyJsonElement(carriageReturnStr);
    const parsedCr = JSON.parseJsonElement(crJson);
    arktest.assertEQ(parsedCr.asString(), "text\rmore", "Parsed carriage return string should be correct");

    const backspaceStr = jsonx.JsonElement.createString("back\bspace");
    const bsJson = JSON.stringifyJsonElement(backspaceStr);
    const parsedBs = JSON.parseJsonElement(bsJson);
    arktest.assertEQ(parsedBs.asString(), "back\bspace", "Parsed backspace string should be correct");

    const formfeedStr = jsonx.JsonElement.createString("form\ffeed");
    const ffJson = JSON.stringifyJsonElement(formfeedStr);
    const parsedFf = JSON.parseJsonElement(ffJson);
    arktest.assertEQ(parsedFf.asString(), "form\ffeed", "Parsed form feed string should be correct");
}

function testParseUnicodeEscaping(): void {
    // Test unicode control character escaping round-trip
    const controlChar = jsonx.JsonElement.createString("test\u0001end");
    const controlJson = JSON.stringifyJsonElement(controlChar);
    const parsedControl = JSON.parseJsonElement(controlJson);
    arktest.assertEQ(parsedControl.asString(), "test\u0001end", "Parsed unicode control character should be correct");

    const nullChar = jsonx.JsonElement.createString("null\u0000char");
    const nullCharJson = JSON.stringifyJsonElement(nullChar);
    const parsedNullChar = JSON.parseJsonElement(nullCharJson);
    arktest.assertEQ(parsedNullChar.asString(), "null\u0000char", "Parsed null character should be correct");
}

function testParseEmptyContainers(): void {
    const emptyArray = jsonx.JsonElement.createArray([]);
    const emptyArrayJson = JSON.stringifyJsonElement(emptyArray);
    const parsedEmptyArray = JSON.parseJsonElement(emptyArrayJson);
    arktest.assertEQ(parsedEmptyArray.jsonType, jsonx.JsonType.JsonArray, "Parsed empty array should have correct type");
    arktest.assertEQ(parsedEmptyArray.asArray().length, 0, "Parsed empty array should be empty");

    const emptyObject = jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>());
    const emptyObjectJson = JSON.stringifyJsonElement(emptyObject);
    const parsedEmptyObject = JSON.parseJsonElement(emptyObjectJson);
    arktest.assertEQ(parsedEmptyObject.jsonType, jsonx.JsonType.JsonObject, "Parsed empty object should have correct type");
}

function testParseArrays(): void {
    // Single element array
    const singleArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("item")
    ]);
    const singleJson = JSON.stringifyJsonElement(singleArray);
    const parsedSingle = JSON.parseJsonElement(singleJson);
    arktest.assertEQ(parsedSingle.jsonType, jsonx.JsonType.JsonArray, "Parsed single array should have correct type");
    arktest.assertEQ(parsedSingle.asArray().length, 1, "Parsed single array should have correct length");
    arktest.assertEQ(parsedSingle.asArray()[0].asString(), "item", "Parsed single array element should be correct");

    // Multi-element array
    const multiArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("first"),
        jsonx.JsonElement.createInteger(42),
        jsonx.JsonElement.createBoolean(true)
    ]);
    const multiJson = JSON.stringifyJsonElement(multiArray);
    const parsedMulti = JSON.parseJsonElement(multiJson);
    arktest.assertEQ(parsedMulti.asArray().length, 3, "Parsed multi array should have correct length");
    arktest.assertEQ(parsedMulti.asArray()[0].asString(), "first", "First element should be correct");
    arktest.assertEQ(parsedMulti.asArray()[1].asInteger(), 42, "Second element should be correct");
    arktest.assertTrue(parsedMulti.asArray()[2].asBoolean(), "Third element should be correct");

    // Array with null
    const nullArray = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createString("before"),
        jsonx.JsonElement.createNull(),
        jsonx.JsonElement.createString("after")
    ]);
    const nullArrayJson = JSON.stringifyJsonElement(nullArray);
    const parsedNullArray = JSON.parseJsonElement(nullArrayJson);
    arktest.assertEQ(parsedNullArray.asArray().length, 3, "Array with null should have correct length");
    arktest.assertEQ(parsedNullArray.asArray()[1].jsonType, jsonx.JsonType.JsonNull, "Null element should be parsed correctly");
}

function testParseObjects(): void {
    // Single property object
    const map1 = new Map<string, jsonx.JsonElement>();
    map1.set("name", jsonx.JsonElement.createString("John"));
    const singleObj = jsonx.JsonElement.createObject(map1);
    const singleJson = JSON.stringifyJsonElement(singleObj);
    const parsedSingle = JSON.parseJsonElement(singleJson);
    arktest.assertEQ(parsedSingle.jsonType, jsonx.JsonType.JsonObject, "Parsed single object should have correct type");
    arktest.assertEQ(parsedSingle.getString("name"), "John", "Parsed single object property should be correct");

    // Multi-property object
    const map2 = new Map<string, jsonx.JsonElement>();
    map2.set("name", jsonx.JsonElement.createString("Alice"));
    map2.set("age", jsonx.JsonElement.createInteger(30));
    map2.set("active", jsonx.JsonElement.createBoolean(true));
    const multiObj = jsonx.JsonElement.createObject(map2);
    const multiJson = JSON.stringifyJsonElement(multiObj);
    const parsedMulti = JSON.parseJsonElement(multiJson);

    arktest.assertEQ(parsedMulti.getString("name"), "Alice", "Parsed object name should be correct");
    arktest.assertEQ(parsedMulti.getInteger("age"), 30, "Parsed object age should be correct");
    arktest.assertTrue(parsedMulti.getBoolean("active"), "Parsed object active should be correct");
}

function testParseNestedStructures(): void {
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
    const arrayJson = JSON.stringifyJsonElement(arrayWithObjects);
    const parsedArray = JSON.parseJsonElement(arrayJson);

    arktest.assertEQ(parsedArray.jsonType, jsonx.JsonType.JsonArray, "Parsed nested array should have correct type");
    arktest.assertEQ(parsedArray.asArray().length, 2, "Parsed nested array should have correct length");
    arktest.assertEQ(parsedArray.asArray()[0].getInteger("id"), 1, "First nested object should be correct");
    arktest.assertEQ(parsedArray.asArray()[1].getString("name"), "Item2", "Second nested object should be correct");

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
    const objJson = JSON.stringifyJsonElement(objWithArray);
    const parsedObj = JSON.parseJsonElement(objJson);

    arktest.assertEQ(parsedObj.getString("name"), "Student", "Object with array name should be correct");
    arktest.assertEQ(parsedObj.getArray("scores").length, 3, "Object with array scores should have correct length");
    arktest.assertEQ(parsedObj.getArray("scores")[1].asInteger(), 92, "Object with array scores should be correct");
}

function testParseMixedArrays(): void {
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
    const mixedJson = JSON.stringifyJsonElement(mixedArray);
    const parsedMixed = JSON.parseJsonElement(mixedJson);

    arktest.assertEQ(parsedMixed.asArray().length, 8, "Mixed array should have correct length");
    arktest.assertEQ(parsedMixed.asArray()[0].asString(), "text", "String element should be correct");
    arktest.assertEQ(parsedMixed.asArray()[1].asInteger(), 42, "Integer element should be correct");
    arktest.assertEQ(parsedMixed.asArray()[2].asDouble(), 3.14, "Double element should be correct");
    arktest.assertTrue(parsedMixed.asArray()[3].asBoolean(), "True element should be correct");
    arktest.assertFalse(parsedMixed.asArray()[4].asBoolean(), "False element should be correct");
    arktest.assertEQ(parsedMixed.asArray()[5].jsonType, jsonx.JsonType.JsonNull, "Null element should be correct");
    arktest.assertEQ(parsedMixed.asArray()[6].jsonType, jsonx.JsonType.JsonArray, "Nested array should be correct");
    arktest.assertEQ(parsedMixed.asArray()[7].jsonType, jsonx.JsonType.JsonObject, "Empty object should be correct");
}

function testParseComplexObjects(): void {
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
    const configJson = JSON.stringifyJsonElement(configElem);
    const parsedConfig = JSON.parseJsonElement(configJson);

    // Verify all components
    arktest.assertEQ(parsedConfig.getString("version"), "1.0.0", "Version should be correct");
    arktest.assertEQ(parsedConfig.getInteger("maxRetries"), 3, "MaxRetries should be correct");
    arktest.assertEQ(parsedConfig.getDouble("timeout"), 30.5, "Timeout should be correct");
    arktest.assertTrue(parsedConfig.getBoolean("enabled"), "Enabled should be correct");
    arktest.assertEQ(parsedConfig.getElement("fallback").jsonType, jsonx.JsonType.JsonNull, "Fallback should be null");
    arktest.assertEQ(parsedConfig.getArray("servers").length, 2, "Servers array should be correct");
    arktest.assertEQ(parsedConfig.getElement("auth").getString("username"), "admin", "Auth object should be correct");
}

function testParseDeeplyNested(): void {
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
    const deepJson = JSON.stringifyJsonElement(deepElem);
    const parsedDeep = JSON.parseJsonElement(deepJson);

    arktest.assertEQ(parsedDeep.getString("root"), "level1", "Root level should be correct");
    arktest.assertEQ(parsedDeep.getElement("child").getElement("nested").getString("deep"), "value");
    arktest.assertEQ(parsedDeep.getElement("child").getElement("nested").getInteger("number"), 123);
    arktest.assertEQ(parsedDeep.getElement("child").getArray("array").length, 2);
}

function testParseWithWhitespace(): void {
    // Test that parser handles whitespace correctly
    const simpleObj = new Map<string, jsonx.JsonElement>();
    simpleObj.set("name", jsonx.JsonElement.createString("test"));
    simpleObj.set("value", jsonx.JsonElement.createInteger(42));
    const elem = jsonx.JsonElement.createObject(simpleObj);

    // Test with formatted JSON (with spaces and newlines)
    const formattedJson = JSON.stringifyJsonElement(elem, undefined, 2);
    const parsedFormatted = JSON.parseJsonElement(formattedJson);
    arktest.assertEQ(parsedFormatted.getString("name"), "test");
    arktest.assertEQ(parsedFormatted.getInteger("value"), 42);

    // Test with extra whitespace manually added
    const compactJson = JSON.stringifyJsonElement(elem);
    const extraWhitespace = "  \n\t " + compactJson + " \n\t  ";
    const parsedWhitespace = JSON.parseJsonElement(extraWhitespace);
    arktest.assertEQ(parsedWhitespace.getString("name"), "test");
}

function checkCorrectError(func: () => void) {
    try {
        func()
        throw new AssertionError("no error")
    } catch(e) {
        if (!(e instanceof JsonParseError)) {
            throw new AssertionError("got Error of different type")
        }
    }
}

function testParseErrorHandling(): void {
    // Test invalid JSON strings
    checkCorrectError((): void => {
        JSON.parseJsonElement("{")
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement("}")
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement("[")
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement("]")
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement('[{}, {]}')
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement('["", "",,]')
    });
    /// NOTE: need to add check case for trailing comma (only one) in array

    checkCorrectError((): void => {
        JSON.parseJsonElement('{"name":}')
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement('{"name":"unclosed string}')
    });

    checkCorrectError((): void => {
        JSON.parseJsonElement("invalid")
    });
}

function testRoundTripConsistency(): void {
    // Test that stringify(parse(stringify(elem))) === stringify(elem)
    const complexElem = createComplexTestElement();

    const json1 = JSON.stringifyJsonElement(complexElem);
    const parsed = JSON.parseJsonElement(json1);
    const json2 = JSON.stringifyJsonElement(parsed);

    // The JSONs should be functionally equivalent
    const reparsed = JSON.parseJsonElement(json2);
    verifyComplexElementEquality(complexElem, reparsed);

    let json3 = `
{
    "version": "2.0",
    "timeout": 15.5,
    "metadata": null,
    "users": [
        {
            "id": 1,
            "name": "Alice",
            "active": true
        },
        {
            "id": 2,
            "name": "Bob",
            "active": false
        }
    ]
}
`
    const parsed3 = JSON.parseJsonElement(json3);
    verifyComplexElementEquality(complexElem, parsed3);
}

function testParseWithReviver(): void {
    // Test parse with reviver function
    const originalElem = new Map<string, jsonx.JsonElement>();
    originalElem.set("count", jsonx.JsonElement.createInteger(10));
    originalElem.set("name", jsonx.JsonElement.createString("test"));
    const elem = jsonx.JsonElement.createObject(originalElem);

    const json = JSON.stringifyJsonElement(elem);

    // Reviver that doubles numeric values
    const reviverResult = JSON.parseJsonElement(json, (key: string, value: jsonx.JsonElement): jsonx.JsonElement => {
        if (key === "count" && value.jsonType === jsonx.JsonType.JsonNumber) {
            return jsonx.JsonElement.createInteger(value.asInteger() * 2);
        }
        return value;
    });

    arktest.assertEQ(reviverResult.getInteger("count"), 20, "Reviver should have doubled the count");
    arktest.assertEQ(reviverResult.getString("name"), "test", "Reviver should not affect other values");
}

function createComplexTestElement(): jsonx.JsonElement {
    // Helper function to create a complex test element
    const users = jsonx.JsonElement.createArray([
        jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>([
            ["id", jsonx.JsonElement.createInteger(1)],
            ["name", jsonx.JsonElement.createString("Alice")],
            ["active", jsonx.JsonElement.createBoolean(true)]
        ])),
        jsonx.JsonElement.createObject(new Map<string, jsonx.JsonElement>([
            ["id", jsonx.JsonElement.createInteger(2)],
            ["name", jsonx.JsonElement.createString("Bob")],
            ["active", jsonx.JsonElement.createBoolean(false)]
        ]))
    ]);

    const config = new Map<string, jsonx.JsonElement>();
    config.set("version", jsonx.JsonElement.createString("2.0"));
    config.set("timeout", jsonx.JsonElement.createDouble(15.5));
    config.set("users", users);
    config.set("metadata", jsonx.JsonElement.createNull());

    return jsonx.JsonElement.createObject(config);
}

function verifyComplexElementEquality(elem1: jsonx.JsonElement, elem2: jsonx.JsonElement): void {
    // Helper function to verify two complex elements are equal
    arktest.assertEQ(elem1.jsonType, elem2.jsonType, "Element types should match");

    if (elem1.jsonType === jsonx.JsonType.JsonObject) {
        arktest.assertEQ(elem1.getString("version"), elem2.getString("version"), "Version should match");
        arktest.assertEQ(elem1.getDouble("timeout"), elem2.getDouble("timeout"), "Timeout should match");
        arktest.assertEQ(elem1.getArray("users").length, elem2.getArray("users").length, "Users array length should match");
        arktest.assertEQ(elem1.getElement("metadata").jsonType, elem2.getElement("metadata").jsonType, "Metadata type should match");

        // Verify first user
        const user1_1 = elem1.getArray("users")[0];
        const user2_1 = elem2.getArray("users")[0];
        arktest.assertEQ(user1_1.getInteger("id"), user2_1.getInteger("id"), "First user ID should match");
        arktest.assertEQ(user1_1.getString("name"), user2_1.getString("name"), "First user name should match");
        arktest.assertEQ(user1_1.getBoolean("active"), user2_1.getBoolean("active"), "First user active should match");
    }
}

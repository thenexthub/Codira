/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import { jsonx } from "std/core"

function main(): int {
    const suite = new arktest.ArkTestsuite("json.JsonElement");

    suite.addTest("JsonElement creation", testJsonElementCreation);
    suite.addTest("JsonElement type checking", testJsonElementTypeChecking);
    suite.addTest("JsonElement type conversion", testJsonElementTypeConversion);
    suite.addTest("JsonElement object operations", testJsonElementObjectOperations);
    suite.addTest("JsonElement array operations", testJsonElementArrayOperations);

    suite.addTest("JsonElement error handling (get)", testJsonElementErrorHandlingGet);
    suite.addTest("JsonElement error handling (iterator)", testJsonElementErrorHandlingIterator);
    suite.addTest("JsonElement error handling (as)", testJsonElementErrorHandlingAs);

    suite.addTest("JsonElement copy constructor", testJsonElementCopyConstructor);
    suite.addTest("JsonElement complex structures", testJsonElementComplexStructures);
    suite.addTest("JsonElement type mutation", testJsonElementTypeMutation);
    suite.addTest("JsonElement factory methods", testJsonElementFactoryMethods);

    suite.addTest('JsonElement parseLong test', testJsonElementParseLongAndInt);
    suite.addTest('JsonElement createInterge asLong test', testJsonElementAsLong);
    suite.addTest('JsonElement test long to int', testLongToInt);
    suite.addTest('JsonElement test long to double', testLongToDouble);

    return suite.run();
}

function testJsonElementCreation(): void {
    // default type - undefined
    const undefinedElem = new jsonx.JsonElement()
    const undefinedWithStatic = jsonx.JsonElement.createUndefined()
    arktest.assertEQ(undefinedElem.jsonType, jsonx.JsonType.JsonUndefined, "Undefined element should have JsonUndefined type");
    arktest.assertEQ(undefinedWithStatic.jsonType, jsonx.JsonType.JsonUndefined, "Undefined element should have JsonUndefined type");

    // Create primitive elements
    const stringElem = new jsonx.JsonElement();
    stringElem.setString("Hello, JSON!");
    arktest.assertEQ(stringElem.asString(), "Hello, JSON!", "String value should match what was set");

    const numberElem = new jsonx.JsonElement();
    numberElem.setDouble(123.45);
    arktest.assertEQ(numberElem.asDouble(), 123.45, "Double value should match what was set");

    const longElem = new jsonx.JsonElement();
    longElem.setLong(123456789);
    arktest.assertEQ(longElem.asLong(), 123456789, 'Double value should match what was set');

    const integerElem = new jsonx.JsonElement();
    integerElem.setInteger(42);
    arktest.assertEQ(integerElem.asInteger(), 42, "Integer value should match what was set");

    const trueElem = new jsonx.JsonElement();
    trueElem.setBoolean(true);
    arktest.assertTrue(trueElem.asBoolean(), "Boolean true value should match what was set");

    const falseElem = new jsonx.JsonElement();
    falseElem.setBoolean(false);
    arktest.assertFalse(falseElem.asBoolean(), "Boolean false value should match what was set");

    // Create null element
    const nullElem = new jsonx.JsonElement();
    nullElem.setNull();
    arktest.assertEQ(nullElem.jsonType, jsonx.JsonType.JsonNull, "Null element should have JsonNull type");
    let testNull = nullElem.tryAsNull();
    arktest.assertEQ(testNull,null);

    // Create array element
    const arrayElem = new jsonx.JsonElement();
    arrayElem.setArray([stringElem, numberElem, integerElem, trueElem, falseElem, nullElem]);
    arktest.assertEQ(arrayElem.asArray().length, 6, "Array should contain 6 elements");

    // Create object element with setElement
    const objectElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    objectElem.setElement("name", jsonx.JsonElement.createString("John Doe"));
    objectElem.setElement("age", jsonx.JsonElement.createInteger(30));
    objectElem.setElement("isActive", jsonx.JsonElement.createBoolean(true));
    objectElem.setElement("height", jsonx.JsonElement.createDouble(1.85));
    objectElem.setElement("money",jsonx.JsonElement.createLong(9223372036854775807))

    // Verify object properties
    arktest.assertEQ(objectElem.getString("name"), "John Doe", "Object property 'name' should be set correctly");
    arktest.assertEQ(objectElem.getInteger("age"), 30, "Object property 'age' should be set correctly");
    arktest.assertTrue(objectElem.getBoolean("isActive"), "Object property 'isActive' should be set correctly");
    arktest.assertEQ(objectElem.getDouble("height"), 1.85, "Object property 'height' should be set correctly");
    arktest.assertEQ(objectElem.getLong("money"), 9223372036854775807, "Object property 'money' should be set correctly");
}

function testJsonElementTypeChecking(): void {
    // Test type checking with jsonType property
    const stringElem = new jsonx.JsonElement();
    stringElem.setString("Hello");
    arktest.assertEQ(stringElem.jsonType, jsonx.JsonType.JsonString, "Element with string should have JsonString type");

    const doubleElem = new jsonx.JsonElement();
    doubleElem.setDouble(3.14);
    arktest.assertEQ(doubleElem.jsonType, jsonx.JsonType.JsonNumber, "Element with double should have JsonNumber type");

    const integerElem = new jsonx.JsonElement();
    integerElem.setInteger(42);
    arktest.assertEQ(integerElem.jsonType, jsonx.JsonType.JsonNumber, "Element with integer should have JsonNumber type");

    const longElem = new jsonx.JsonElement();
    longElem.setLong(-9223372036854775808);
    arktest.assertEQ(integerElem.jsonType, jsonx.JsonType.JsonNumber, "Element with Long should have JsonNumber type");

    const trueElem = new jsonx.JsonElement();
    trueElem.setBoolean(true);
    arktest.assertEQ(trueElem.jsonType, jsonx.JsonType.JsonTrue, "Element with true should have JsonTrue type");

    const falseElem = new jsonx.JsonElement();
    falseElem.setBoolean(false);
    arktest.assertEQ(falseElem.jsonType, jsonx.JsonType.JsonFalse, "Element with false should have JsonFalse type");

    const nullElem = new jsonx.JsonElement();
    arktest.assertEQ(nullElem.jsonType, jsonx.JsonType.JsonUndefined, "Element with no value should have JsonUndefined type");
    nullElem.setNull();
    arktest.assertEQ(nullElem.jsonType, jsonx.JsonType.JsonNull, "Element with no value should have JsonNull type");

    const arrayElem = new jsonx.JsonElement();
    arrayElem.setArray([]);
    arktest.assertEQ(arrayElem.jsonType, jsonx.JsonType.JsonArray, "Element with array should have JsonArray type");

    const objectElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    objectElem.setElement("key", stringElem);
    arktest.assertEQ(objectElem.jsonType, jsonx.JsonType.JsonObject, "Element with object should have JsonObject type");
}

function testJsonElementTypeConversion(): void {
    // Test strict value conversion
    const stringElem = new jsonx.JsonElement();
    stringElem.setString("Hello, world!");
    arktest.assertEQ(stringElem.asString(), "Hello, world!", "asString() should return string value");

    const doubleElem = new jsonx.JsonElement();
    doubleElem.setDouble(3.14159);
    arktest.assertEQ(doubleElem.asDouble(), 3.14159, "asDouble() should return double value");

    const integerElem = new jsonx.JsonElement();
    integerElem.setInteger(42);
    arktest.assertEQ(integerElem.asInteger(), 42, "asInteger() should return integer value");

    const longElem = new jsonx.JsonElement();
    longElem.setLong(-9223372036854775808);
    arktest.assertEQ(longElem.asLong(), -9223372036854775808, "asLong() should return long value");

    const boolElem = new jsonx.JsonElement();
    boolElem.setBoolean(true);
    arktest.assertTrue(boolElem.asBoolean(), "asBoolean() should return boolean value");

    // Test optional conversions (try methods)
    const stringValue = stringElem.tryAsString();
    arktest.assertEQ(stringValue, "Hello, world!", "tryAsString() should return string value for string element");

    const invalidDoubleValue = stringElem.tryAsDouble();
    arktest.assertEQ(invalidDoubleValue, undefined, "tryAsDouble() on string element should return undefined");

    const invalidIntegerValue = stringElem.tryAsInteger();
    arktest.assertEQ(invalidIntegerValue, undefined, "tryAsInteger() on string element should return undefined");

    const invalidBooleanValue = stringElem.tryAsBoolean();
    arktest.assertEQ(invalidBooleanValue, undefined, "tryAsBoolean() on string element should return undefined");

    const invalidLongValue = stringElem.tryAsLong();
    arktest.assertEQ(invalidLongValue, undefined, "tryAsLong() on string element should return undefined");

    const invalidNullValue = stringElem.tryAsNull();
    arktest.assertEQ(invalidNullValue, undefined, "tryAsNull() on string element should return undefined");
}

function testJsonElementObjectOperations(): void {
    // Create an object with properties
    const objectElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    objectElem.setElement("name", jsonx.JsonElement.createString("John Doe"));
    objectElem.setElement("age", jsonx.JsonElement.createInteger(30));
    objectElem.setElement("isActive", jsonx.JsonElement.createBoolean(true));

    // Test getElement
    const nameElem = objectElem.getElement("name");
    arktest.assertEQ(nameElem.asString(), "John Doe", "getElement() should return correct element");

    // Test property access methods
    arktest.assertEQ(objectElem.getString("name"), "John Doe", "getString() should return string property");
    arktest.assertEQ(objectElem.getInteger("age"), 30, "getInteger() should return integer property");
    arktest.assertTrue(objectElem.getBoolean("isActive"), "getBoolean() should return boolean property");

    // Test tryGetElement for existing property
    const ageElem = objectElem.tryGetElement("age");
    arktest.assertNE(ageElem, undefined, "tryGetElement() should find existing property");
    arktest.assertEQ(ageElem?.asInteger(), 30, "Element from tryGetElement() should have correct value");

    // Test tryGetElement for non-existing property
    const nonExistentElem = objectElem.tryGetElement("nonExistent");
    arktest.assertEQ(nonExistentElem, undefined, "tryGetElement() should return undefined for non-existent property");

    // Test optional property access methods with fallback
    arktest.assertEQ(objectElem.tryGetString("name"), "John Doe", "tryGetString() should return string for existing property");
    arktest.assertEQ(objectElem.tryGetString("nonExistent", "Default"), "Default", "tryGetString() should return fallback for non-existent property");
    arktest.assertEQ(objectElem.tryGetInteger("age"), 30, "tryGetInteger() should return integer for existing property");
    arktest.assertEQ(objectElem.tryGetInteger("nonExistent", 100), 100, "tryGetInteger() should return fallback for non-existent property");
    arktest.assertTrue(objectElem.tryGetBoolean("isActive"), "tryGetBoolean() should return boolean for existing property");
    arktest.assertFalse(objectElem.tryGetBoolean("nonExistent"), "tryGetBoolean() should return false fallback for non-existent property");

    // Test property modification
    objectElem.setElement("age", jsonx.JsonElement.createInteger(31));
    arktest.assertEQ(objectElem.getInteger("age"), 31, "Property should be updated after setElement()");

    // Test property removal
    const removed = objectElem.removeElement("age");
    arktest.assertTrue(removed, "removeElement() should return true for existing property");

    const removedAgain = objectElem.removeElement("age");
    arktest.assertFalse(removedAgain, "removeElement() should return false for already removed property");

    // Test trying to get removed property
    const removedElem = objectElem.tryGetElement("age");
    arktest.assertEQ(removedElem, undefined, "Removed property should not be accessible");

    // Test iterator
    let propertyCount = 0;
    let it = objectElem.$_iterator();
    while (true) {
        const v = it.next();
        if (v.done) {
            break;
        }
        const key = v.value![0];
        arktest.assertTrue(key === "name" || key === "isActive", "Iterator should return expected keys");
        propertyCount++;
    }
    arktest.assertEQ(propertyCount, 2, "Iterator should visit 2 properties after removal");
}

function testJsonElementArrayOperations(): void {
    // Create array element
    const arrayElem = new jsonx.JsonElement();
    const items = [
        jsonx.JsonElement.createString("item1"),
        jsonx.JsonElement.createInteger(42),
        jsonx.JsonElement.createBoolean(true)
    ];
    arrayElem.setArray(items);

    // Test array access
    const arr = arrayElem.asArray();
    arktest.assertEQ(arr.length, 3, "Array should have 3 elements");
    arktest.assertEQ(arr[0].asString(), "item1", "First array element should be correct");
    arktest.assertEQ(arr[1].asInteger(), 42, "Second array element should be correct");
    arktest.assertTrue(arr[2].asBoolean(), "Third array element should be correct");

    // Test $_get for array indexing
    const firstElement = arrayElem.$_get(0);
    arktest.assertEQ(firstElement.asString(), "item1", "$_get should return correct element at index");

    const secondElement = arrayElem.$_get(1);
    arktest.assertEQ(secondElement.asInteger(), 42, "$_get should return correct element at index");
}

function expectJsonError(func: () => void): void {
    arktest.expectThrow(func, (e: Error) => { return (e instanceof jsonx.JsonError) });
}
function expectJsonTypeError(func: () => void): void {
    arktest.expectThrow(func, (e: Error) => { return (e instanceof jsonx.JsonTypeError) });
}
function testJsonElementErrorHandlingGet(): void {
    // forall e, i: int: e.jsonKind != Array => e[i] throws
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            u[1]
        }
    );
    // forall e, i: int:
    //   e.jsonKind == Array
    //   i < 0 || i > |e| - 1
    //      => e[i] throws RangeError
    arktest.expectThrow(
        (): void => {
            let u = jsonx.JsonElement.createArray([]);
            u[0]
        },
        (e: Error) => {
            return (e instanceof RangeError)
        }
    );

    // get[s]
    // forall e, k: string:
    //   e.jsonKind != Object
    //      => e[k] throws JsonTypeError
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            u["key"]
        }
    );
    // the same for getElement
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            u.getElement("key")
        }
    );

    // forall e, k: string:
    //   k not in e.maybeObjectValues.Keys
    //      => e[k] throws RangeError
    arktest.expectThrow(
        (): void => {
            let u = jsonx.JsonElement.createObject({});
            u["key"]
        },
        (e: Error) => {
            return (e instanceof RangeError)
        }
    );
    // the same for getElement
    arktest.expectThrow(
        (): void => {
            let u = jsonx.JsonElement.createObject({});
            u.getElement("key")
        },
        (e: Error) => {
            return (e instanceof RangeError)
        }
    );
}

function testJsonElementErrorHandlingIterator(): void {
    // forall e: JsonElement:
    //   e.jsonKind != Object
    //      => e.$_iterator() throws JsonTypeError
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            u.$_iterator()
        }
    );
    // the same for Array
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createArray([]);
            u.$_iterator()
        }
    );
}

function testJsonElementErrorHandlingAs(): void {
    // forall t: JsonType, e: JsonElement:
    //   e.jsonType != t
    //   => e as t throws JsonTypeError
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            let v = u.asArray()
        }
    );
    // no implicit conversion double <=> integer
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createDouble(3.14);
            let v = u.asInteger()
        }
    );
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createBoolean(true);
            let v = u.asDouble()
        }
    );
    expectJsonTypeError(
        (): void => {
            let u = jsonx.JsonElement.createString("test");
            let v = u.asBoolean()
        }
    );
    // ...
}

function testJsonElementCopyConstructor(): void {
    // Create original element with nested structure
    const original = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    original.setElement("name", jsonx.JsonElement.createString("John"));
    original.setElement("age", jsonx.JsonElement.createInteger(30));
    original.setElement("scores", jsonx.JsonElement.createArray([
        jsonx.JsonElement.createInteger(85),
        jsonx.JsonElement.createInteger(92),
        jsonx.JsonElement.createInteger(78),
        jsonx.JsonElement.createLong(89)
    ]));

    // Create copy using copy constructor
    const copy = new jsonx.JsonElement(original);
    copy.setElement("name", jsonx.JsonElement.createString("Jane"));

    arktest.assertNE(copy.getElement("name").asString(),
             original.getElement("name").asString(),
             "Copy should have different name");
    for(let i = 0; i < 4; i++) {
        arktest.assertNE(copy.getElement("scores").asArray()[i],original.getElement("scores").asArray()[i])
    }
}

function testJsonElementComplexStructures(): void {
    // Create a complex nested structure
    const userElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);

    // Add basic properties
    userElem.setElement("id", jsonx.JsonElement.createInteger(1001));
    userElem.setElement("name", jsonx.JsonElement.createString("Alice Johnson"));
    userElem.setElement("active", jsonx.JsonElement.createBoolean(true));

    // Add nested address object
    const addressElem = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    addressElem.setElement("street", jsonx.JsonElement.createString("123 Main St"));
    addressElem.setElement("city", jsonx.JsonElement.createString("Anytown"));
    addressElem.setElement("zipCode", jsonx.JsonElement.createInteger(54321));
    userElem.setElement("address", addressElem);

    // Add array of tags
    const tagsArray = [
        jsonx.JsonElement.createString("developer"),
        jsonx.JsonElement.createString("javascript"),
        jsonx.JsonElement.createString("typescript")
    ];
    userElem.setElement("tags", jsonx.JsonElement.createArray(tagsArray));

    // Add array of scores
    const scoresArray = [
        jsonx.JsonElement.createInteger(85),
        jsonx.JsonElement.createInteger(92),
        jsonx.JsonElement.createInteger(78)
    ];
    userElem.setElement("scores", jsonx.JsonElement.createArray(scoresArray));

    // Add nested contacts array with objects
    const contact1 = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    contact1.setElement("type", jsonx.JsonElement.createString("email"));
    contact1.setElement("value", jsonx.JsonElement.createString("alice@example.com"));

    const contact2 = new jsonx.JsonElement({} as Record<string, jsonx.JsonElement>);
    contact2.setElement("type", jsonx.JsonElement.createString("phone"));
    contact2.setElement("value", jsonx.JsonElement.createString("555-123-4567"));

    userElem.setElement("contacts", jsonx.JsonElement.createArray([contact1, contact2]));

    // Verify structure
    arktest.assertEQ(userElem.getInteger("id"), 1001, "Complex object should have correct integer property");
    arktest.assertEQ(userElem.getString("name"), "Alice Johnson", "Complex object should have correct string property");
    arktest.assertTrue(userElem.getBoolean("active"), "Complex object should have correct boolean property");

    // Verify nested object
    const address = userElem.getElement("address");
    arktest.assertEQ(address.getString("street"), "123 Main St", "Nested object should have correct street property");
    arktest.assertEQ(address.getString("city"), "Anytown", "Nested object should have correct city property");
    arktest.assertEQ(address.getInteger("zipCode"), 54321, "Nested object should have correct zipCode property");

    // Verify arrays
    const tags = userElem.getArray("tags");
    arktest.assertEQ(tags.length, 3, "Tags array should have 3 elements");
    arktest.assertEQ(tags[0].asString(), "developer", "Tags array should have correct elements");

    const scores = userElem.getArray("scores");
    arktest.assertEQ(scores.length, 3, "Scores array should have 3 elements");
    arktest.assertEQ(scores[1].asInteger(), 92, "Scores array should have correct elements");

    // Verify nested array of objects
    const contacts = userElem.getArray("contacts");
    arktest.assertEQ(contacts.length, 2, "Contacts array should have 2 elements");
    arktest.assertEQ(contacts[0].getString("type"), "email", "Contact object should have correct type");
    arktest.assertEQ(contacts[0].getString("value"), "alice@example.com", "Contact object should have correct value");
    arktest.assertEQ(contacts[1].getString("type"), "phone", "Contact object should have correct type");
    arktest.assertEQ(contacts[1].getString("value"), "555-123-4567", "Contact object should have correct value");
}

function testJsonElementTypeMutation(): void {
    // Test ability to change element type
    const elem = new jsonx.JsonElement();

    // Start with string
    elem.setString("Hello");
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonString, "Element should be string type initially");
    arktest.assertEQ(elem.asString(), "Hello", "Element should have correct string value");

    // Change to double
    elem.setDouble(3.14);
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonNumber, "Element should be number type after setDouble");
    arktest.assertEQ(elem.asDouble(), 3.14, "Element should have correct double value");

    // Change to integer
    elem.setInteger(42);
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonNumber, "Element should be number type after setInteger");
    arktest.assertEQ(elem.asInteger(), 42, "Element should have correct integer value");

    // Change to boolean true
    elem.setBoolean(true);
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonTrue, "Element should be true type after setBoolean(true)");
    arktest.assertTrue(elem.asBoolean(), "Element should have correct boolean value");

    // Change to boolean false
    elem.setBoolean(false);
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonFalse, "Element should be false type after setBoolean(false)");
    arktest.assertFalse(elem.asBoolean(), "Element should have correct boolean value");

    // Change to array
    elem.setArray([
        jsonx.JsonElement.createString("item")
    ]);
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonArray, "Element should be array type after setArray");
    arktest.assertEQ(elem.asArray().length, 1, "Element should have correct array length");

    // Set to null
    elem.setNull();
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonNull, "Element should be null type after setNull");
    elem.setUndefined()
    arktest.assertEQ(elem.jsonType, jsonx.JsonType.JsonUndefined, "Element should be undefined type after setUndefined");

    // Test implicit setNull when changing types
    const elemB = new jsonx.JsonElement();
    elemB.setString("Hello");
    elemB.setDouble(3.14); // This should implicitly call setNull first
    arktest.assertEQ(elemB.jsonType, jsonx.JsonType.JsonNumber, "Element should change to number type");
}

function testJsonElementFactoryMethods(): void {
    // Test static creation methods

    // Create string
    const stringElem = jsonx.JsonElement.createString("Hello, Factory!");
    arktest.assertEQ(stringElem.jsonType, jsonx.JsonType.JsonString, "Factory method should create string element");
    arktest.assertEQ(stringElem.asString(), "Hello, Factory!", "Factory method should set correct string value");

    // Create double
    const doubleElem = jsonx.JsonElement.createDouble(3.14159);
    arktest.assertEQ(doubleElem.jsonType, jsonx.JsonType.JsonNumber, "Factory method should create number element");
    arktest.assertEQ(doubleElem.asDouble(), 3.14159, "Factory method should set correct double value");

    // Create integer
    const integerElem = jsonx.JsonElement.createInteger(42);
    arktest.assertEQ(integerElem.jsonType, jsonx.JsonType.JsonNumber, "Factory method should create number element");
    arktest.assertEQ(integerElem.asInteger(), 42, "Factory method should set correct integer value");

    // Create boolean
    const trueElem = jsonx.JsonElement.createBoolean(true);
    arktest.assertEQ(trueElem.jsonType, jsonx.JsonType.JsonTrue, "Factory method should create true element");
    arktest.assertTrue(trueElem.asBoolean(), "Factory method should set correct boolean value");

    const falseElem = jsonx.JsonElement.createBoolean(false);
    arktest.assertEQ(falseElem.jsonType, jsonx.JsonType.JsonFalse, "Factory method should create false element");
    arktest.assertFalse(falseElem.asBoolean(), "Factory method should set correct boolean value");

    // Create null
    const nullElem = jsonx.JsonElement.createNull();
    arktest.assertEQ(nullElem.jsonType, jsonx.JsonType.JsonNull, "Factory method should create undefined element");

    // Create array
    const arrayElements = [
        jsonx.JsonElement.createString("item1"),
        jsonx.JsonElement.createInteger(42),
        jsonx.JsonElement.createBoolean(true)
    ];
    const arrayElem = jsonx.JsonElement.createArray(arrayElements);
    arktest.assertEQ(arrayElem.jsonType, jsonx.JsonType.JsonArray, "Factory method should create array element");
    arktest.assertEQ(arrayElem.asArray().length, 3, "Factory method should create array with correct length");
    arktest.assertEQ(arrayElem.asArray()[0].asString(), "item1", "Factory method should preserve array element values");

    // Create object
    const map = new Map<string, jsonx.JsonElement>();
    map.set("name", jsonx.JsonElement.createString("John Doe"));
    map.set("age", jsonx.JsonElement.createInteger(30));

    const objectElem = jsonx.JsonElement.createObject(map);
    arktest.assertEQ(objectElem.jsonType, jsonx.JsonType.JsonObject, "Factory method should create object element");
    arktest.assertEQ(objectElem.getString("name"), "John Doe", "Factory method should preserve object property values");
    arktest.assertEQ(objectElem.getInteger("age"), 30, "Factory method should preserve object property values");
}

function testJsonElementSetElementChangeType(): void {
    const root = new jsonx.JsonElement();
    const prop = new jsonx.JsonElement();
    prop.setDouble(10);
    root.setElement("prop", prop);
    arktest.assertEQ(root.getDouble("prop"), 10, "SetElement should automatically change type");
}

function testJsonElementNumberCasts(): void {
    let usernameInteger = '{"username": 1234}';
    let jsonElement = JSON.parseJsonElement(usernameInteger);
    arktest.assertEQ(jsonElement.getDouble("username"),  1234.0, "GetDouble should return correct double value");
    arktest.assertEQ(jsonElement.getInteger("username"), 1234, "GetInteger should return correct integer value");

    let usernameDouble = '{"username": 1234.5678}';
    let jsonElementDouble = JSON.parseJsonElement(usernameDouble);

    arktest.expectThrow(
        (): void => {
            let u = jsonElementDouble.getInteger("username");
        },
        (e: Error) => {
            return (e instanceof jsonx.JsonTypeError)
        }
    )
}

function testJsonElementParseLongAndInt(): void {
    let str = '9223372036854775807'
    let longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 9223372036854775807)

    str = '-9223372036854775808'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), -9223372036854775808)

    str = '9223372036854775808'
    arktest.expectError((): void => { JSON.parseJsonElement(str) }, new FormatError('Value exceeds integer limits'))

    str = '-9223372036854775809'
    arktest.expectError((): void => { JSON.parseJsonElement(str) }, new FormatError('Value exceeds integer limits'))

    str = '0'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 0)
    arktest.assertEQ(longElem.asInteger(), 0)

    str = '1234567890123456789'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 1234567890123456789)

    str = '-987654321098765432'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), -987654321098765432)

    str = '1'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 1)
    arktest.assertEQ(longElem.asInteger(), 1)

    str = '42'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 42)
    arktest.assertEQ(longElem.asInteger(), 42)
}

function testJsonElementAsLong(): void {
    const intElem = jsonx.JsonElement.createInteger(30)
    let longValue = intElem.asLong()
    arktest.assertEQ(longValue, 30)

    const doubleElem = jsonx.JsonElement.createDouble(1234.5)
    let maybeLong = doubleElem.tryAsLong()
    arktest.assertEQ(maybeLong, undefined)
}

function testLongToInt(): void {
    let str = '2147483647'
    let longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asInteger(), 2147483647)

    str = '2147483648'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), 2147483648)
    arktest.assertEQ(longElem.tryAsInteger(), undefined)
    arktest.expectError((): void => { longElem.asInteger() }, new RangeError(`'2147483648' is not a int`))

    str = '-2147483648'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asInteger(), -2147483648)

    str = '-2147483649'
    longElem = JSON.parseJsonElement(str)
    arktest.assertEQ(longElem.asLong(), -2147483649)
    arktest.assertEQ(longElem.tryAsInteger(), undefined)
    arktest.expectError((): void => { longElem.asInteger() }, new RangeError(`'-2147483649' is not a int`))
}

function testLongToDouble(): void {
    let str = '9007199254740991'
    let doubleElem = JSON.parseJsonElement(str)
    arktest.assertEQ(doubleElem.asDouble(), 9007199254740991)

    str = '9007199254740992'
    doubleElem = JSON.parseJsonElement(str)
    arktest.assertEQ(doubleElem.asLong(), 9007199254740992)
    arktest.assertEQ(doubleElem.tryAsDouble(), undefined)
    arktest.expectError((): void => { doubleElem.asDouble() }, new RangeError(`'9007199254740992' is not a double`))

    str = '-9007199254740991'
    doubleElem = JSON.parseJsonElement(str)
    arktest.assertEQ(doubleElem.asDouble(), -9007199254740991)

    str = '-9007199254740992'
    doubleElem = JSON.parseJsonElement(str)
    arktest.assertEQ(doubleElem.asLong(), -9007199254740992)
    arktest.assertEQ(doubleElem.tryAsDouble(), undefined)
    arktest.expectError((): void => { doubleElem.asDouble() }, new RangeError(`'-9007199254740992' is not a double`))
}
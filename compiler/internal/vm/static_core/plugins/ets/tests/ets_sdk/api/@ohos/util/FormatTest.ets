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

import {util} from "@ohos.util";

const success: int = 0;
const fail: int = 1;

function check(result: int, message: string): int {
    if (result == 0) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function main(): int {
    let failures: int = 0
    failures += check(testUtilFormat(), "Test util.format()")
    failures += check(testFormatLessArgsThanInFormat(), "Test util.format() with less arguments count than in format string")
    failures += check(testFormatIncorrectArgs(), "Test util.format() with incorrect argument flags")
    failures += check(testFormatNoMatchArgs(), "Test util.format() with no matching flags & arguments")
    failures += check(testFormatMoreArgsThanInUtilFormat(), "Test util.format() with more args than in format string")
    failures += check(testFormatJSON(), "Test util.format() JSON")
    failures += check(testFormatEscapePercent(), "Test util.format() for %%")
    failures += check(testEmptyFormatWithArgs(), "Test util.format() with args & empty format string")
    failures += check(testNonEmptyFormatWithArgs(), "Test util.format() with args & non-empty format string")
    failures += check(testFormatWithoutArgs(), "Test util.format() without args")
    failures += check(testFormatObject(), "Test util.format() Object as util2.inspect() with options { showHidden: true, showProxy: true }")
    failures += check(testFormatObject2(), "Test util.format() Object as util2.inspect() without options")
    failures += check(testFormatArrayObect(), "Test util.format() for Array object")
    failures += check(testFunctionsArrayObject(), "Test util.format() for Array of functions object")
    failures += check(testFunctionAsObject(), "Test util.format() for Function object")
    failures += check(testFormatCircularObject(), "Test util.format() Object with circular reference")
    failures += check(testArrayOfCircularObjects(), "Test util.format() for Array with circular reference objects")
    failures += check(testApiV1Tests(), "Test util.format() from api v1")
    return failures
}

function testUtilFormat(): int {

    const s : string = "computer"
    const i: int = 35
    const fp : float = 1.7320534f

    arktest.assertEQ(util.format("String: %s", s), "String: computer")
    arktest.assertEQ(util.format("%s", s), "computer")
    arktest.assertEQ(util.format("%s String", s), "computer String")
    arktest.assertEQ(util.format("%s ", s), "computer ")
    arktest.assertEQ(util.format(" %s", s), " computer")
    arktest.assertEQ(util.format("%%s", s), "%s computer")
    arktest.assertEQ(util.format("%%s%", s), "%s% computer")
    arktest.assertEQ(util.format("s%", s), "s% computer")
    arktest.assertEQ(util.format("%s%", s), "computer%")

    arktest.assertEQ(util.format("   Integer:   %d\n", i), "   Integer:   35\n")
    arktest.assertEQ(util.format("   Real:      %f\n", fp), "   Real:      1.7320534\n")

    arktest.assertEQ(util.format("String: %s int %d !!! %d %f %d", s, 1, 2, 3, 4), "String: computer int 1 !!! 2 3 4")  // String: computer int 1 !!! 2 3.0 4 ???

    return success
}

function testFormatLessArgsThanInFormat(): int {
    let res = util.format("example %d of invalid %f format string", 1)
    arktest.assertEQ(res, "example 1 of invalid %f format string")
    return success
}

function testFormatIncorrectArgs(): int {
    let res = util.format("example %X of invalid format string", 1)
    arktest.assertEQ(res, "example %X of invalid format string 1")
    return success
}

function testFormatNoMatchArgs(): int {
    {
        // NOTE(ekaterinazaytseva): converting string to int -> NaN
        let res = util.format("example %s of invalid %d format string %d", "string", 1, "string instead int")
        arktest.assertEQ(res, "example string of invalid 1 format string NaN")
    }

    {
        let res = util.format("example %s of invalid %d format string %o", "string", 1, "string as object")
        arktest.assertEQ(res, "example string of invalid 1 format string 'string as object'")
    }
    return success
}

function testFormatMoreArgsThanInUtilFormat(): int {
    let res = util.format("Text: %s", "ddd", 44, "one more time text")
    arktest.assertEQ(res, "Text: ddd 44 one more time text")
    return success
}

class TestObject1 {
    x: int = 5
    y: int = 6
}

function testFormatJSON(): int {
    // simple JSON
    arktest.assertEQ(util.format("JSON : %j", new TestObject1()), "JSON : {\"x\":5,\"y\":6}")

    // JSON with circular reference
    let jsonWithCircularRef : string = "{\"name\":\"Kris\", \"father\":{\"name\":\"Bill\",\"wife\":{\"name\":\"Karen\"}},\"mother\":{\"$ref\":\"#father.wife\"}}"
    let expected = "JSON : \"{\\\"name\\\":\\\"Kris\\\", \\\"father\\\":{\\\"name\\\":\\\"Bill\\\",\\\"wife\\\":{\\\"name\\\":\\\"Karen\\\"}},\\\"mother\\\":{\\\"$ref\\\":\\\"#father.wife\\\"}}\"";
    let res = util.format("JSON : %j", jsonWithCircularRef)
    arktest.assertEQ(res, expected)
    return success
}

class TestObject2 {
    strField : string
    intField : int
    floatField : float
    boolField : boolean = false
    constructor(strF: string, intF: int, floatF: float) {
        this.strField = strF
        this.intField = intF
        this.floatField = floatF
    }
}
function testFormatObject(): int {

    let res = util.format("Object: %o", new TestObject2("k28", 14, 0.1f))
    let expected = "Object: { strField: 'k28',\nintField: 14,\nfloatField: 0.1,\nboolField: false }"
    arktest.assertEQ(res, expected)
    return success
}

function testFormatObject2(): int {
    let res = util.format("Object: %O", new TestObject2("k28", 14, 0.1f))
    let expected = "Object: { strField: 'k28',\nintField: 14,\nfloatField: 0.1,\nboolField: false }"
    arktest.assertEQ(res, expected)
    return success
}

class TestObject3 {
    prop: string = ""
    reference : Object = 0
}
function testFormatCircularObject(): int {

    const circularObject = new TestObject3()
    circularObject.prop = "Circular Reference Example"
    circularObject.reference = circularObject;

    let expected = "{ prop: 'Circular Reference Example',\nreference: [Circular]\n }"

    let res = util.format("%o", circularObject)
    arktest.assertEQ(res, expected)

    res = util.format("%O", circularObject)
    arktest.assertEQ(res, expected)

    return success
}

function testArrayOfCircularObjects() : int {
    const circularObject = new TestObject3()
    circularObject.prop = "Circular Reference Example"
    circularObject.reference = circularObject

    let arr = new Array<TestObject3>(2)
    arr[0] = circularObject
    arr[1] = circularObject

    let expected = "[ { prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, \n{ prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, [length]: 2 ]"
    let expected1 = "[ { prop: 'Circular Reference Example',\n  reference: [Circular]\n   }, \n{ prop: 'Circular Reference Example',\n  reference: [Circular]\n   } ]"

    let res = util.format("%o", arr)
    arktest.assertEQ(res, expected)

    res = util.format("%O", arr)
    arktest.assertEQ(res, expected1)

    return success
}

function testEmptyFormatWithArgs(): int {
    let res = util.format("", 1, 2, 3, "test")
    arktest.assertEQ(res, "1 2 3 test")
    return success
}

function testNonEmptyFormatWithArgs(): int {
    let res = util.format("Some important text!", 1, 2, 3, "test")
    arktest.assertEQ(res, "Some important text! 1 2 3 test")
    return success
}

function testFormatWithoutArgs(): int {
    let res = util.format("%d %s  lalala %d %d")
    arktest.assertEQ(res, "%d %s  lalala %d %d")
    return success
}

function testFormatEscapePercent(): int {
    let res = util.format("%%  lalala %%%%")
    arktest.assertEQ(res, "%  lalala %%")
    return success
}

class TestObject4 {
    f1: string = ""
    k1: int = 8
    t : boolean = false
}
function testFormatArrayObect(): int {

    {
        let arr =  new Array<TestObject4>()

        arktest.assertEQ(util.format('Array: %o', arr), "Array: [ [length]: 0 ]")
        arktest.assertEQ(util.format('Array: %O', arr), "Array: [  ]")
    }
    {
        let arr =  new Array<TestObject4>(1)
        arr[0] = new TestObject4()

        let expected = "Array: [ { f1: '',\n  k1: 8,\n  t: false }, [length]: 1 ]"
        let expected1 = "Array: [ { f1: '',\n  k1: 8,\n  t: false } ]"

        arktest.assertEQ(util.format('Array: %o', arr), expected)
        arktest.assertEQ(util.format('Array: %O', arr), expected1)
    }

    return success
}

function testFunctionAsObject() : int {

    let func = (arg: string):string => { return "XX" + arg}

    let expected = "Function: { [Function: null]  [length]: 0,  [name] :'null',  [prototype]: null { [constructor]: [Circular] } }"
    let expected1 = "Function: { [Function: null] }"

    arktest.assertEQ(util.format('Function: %o', func), expected)
    arktest.assertEQ(util.format('Function: %O', func), expected1)

    return success
}

function testFunctionsArrayObject() : int {
    let lambda = (arg: string):string => { return "XX" + arg}

    let arr = new Array<(arg: string)=>string>(1)
    arr[0] = lambda

    let expected = "Array of functions: [ { [Function: null]  [length]: 0,  [name] :'null',  [prototype]: null { [constructor]: [Circular] } }, [length]: 1 ]"
    let expected1 = "Array of functions: [ [Function: null] ]"

    arktest.assertEQ(util.format('Array of functions: %o', arr), expected)
    arktest.assertEQ(util.format('Array of functions: %O', arr), expected1)

    return success
}

class Person {
    name: string = 'John'
    age: int  = 20
}

class Address {
    city: string = 'New York'
    country: string = 'USA'
}
class UtilPersontype {
    name: string = 'John'
    age: int  = 20
    address: Address = {}
};
interface utilAddresstype {
    city: string;
    country: string;
}
interface utilPersontype {
    name: string;
    age: number;
    address: utilAddresstype;
}

function testApiV1Tests() : int {

    let name = 'John';
    let age = 20;
    let formattedString = util.format('My name is %s and I am %s years old', name, age);
    arktest.assertEQ(formattedString, "My name is John and I am 20 years old");

    let num = 10.5;
    formattedString = util.format('The number is %d', num);
    arktest.assertEQ(formattedString, "The number is 10.5");

    num = 100.5;
    formattedString = util.format('The number is %i', num);
    arktest.assertEQ(formattedString, "The number is 100");

    const pi = 3.141592653;
    formattedString = util.format('The value of pi is %f', pi);
    arktest.assertEQ(formattedString, "The value of pi is 3.141592653");

    const person: UtilPersontype = {
        name: 'John',
        age: 20,
        address: {
            city: 'New York',
            country: 'USA'
        }
    };
    let expected1 = "Formatted object using %O: { name: 'John',\n\age: 20,\n\address:\n\{ city: 'New York',\n\  country: 'USA' } }"
    let expected2 = "Formatted object using %o: { name: 'John',\n\age: 20,\n\address:\n\{ city: 'New York',\n\  country: 'USA' } }"

    arktest.assertEQ(util.format('Formatted object using %%O: %O', person), expected1);
    arktest.assertEQ(util.format('Formatted object using %%o: %o', person), expected2);

    const percentage = 80;
    let arg = 'homework';
    formattedString = util.format('John finished %d%% of the %s', percentage, arg);
    arktest.assertEQ(formattedString, "John finished 80% of the homework")

    return success
}

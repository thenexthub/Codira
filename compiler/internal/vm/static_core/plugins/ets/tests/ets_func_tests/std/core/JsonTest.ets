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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return RESULT_SUCCESS
    }
    console.println("FAILED: " + name)
    return RESULT_FAILURE
}

function testNull(): int {
    let nullType = Type.of(null)
    return test(JSON.parse<Object>("null", nullType) == null, "Null")
}

function testBoolean(): int {
    let booleanType = Type.of(new Boolean() as Object)
    return test((JSON.parse<Object>("true", booleanType) as Boolean) == true, "Boolean(true)") +
          test((JSON.parse<Object>("false", booleanType) as Boolean) == false, "Boolean(false)")
}

function testDouble(): int {
    let doubleType = Type.of(new Double() as Object)
    return test((JSON.parse<Object>("0", doubleType) as Double) == 0, "Double(0)") +
        test((JSON.parse<Object>("1", doubleType) as Double) == 1, "Double(1)") +
        test((JSON.parse<Object>("-1", doubleType) as Double) == -1, "Double(-1)") +
        test((JSON.parse<Object>("2.72182", doubleType) as Double) == 2.72182, "Double(2.72182)") +
        test((JSON.parse<Object>("-2.72182", doubleType) as Double) == -2.72182, "Double(-2.72182)")
}

function testStringWithEscape01(): int {
    let stringType = Type.of(new String() as Object)
    let s = 'a\\\b\\c\\\\d'
    let s1 = '"a\\\\\\b\\\\c\\\\\\\\d"'
    let msg = 'String(a\\\b\\c\\\\d)'
    return test(JSON.stringify(s) == s1, msg) +
        test(JSON.parse<Object>(s1, stringType) as String == s, msg)
}

function testStringWithEscape02(): int {
    let stringType = Type.of(new String() as Object)
    let s = '\"-\\-\/-\b-\f-\n-\r-\t'
    let s1 = '"\\"-\\\\-/-\\b-\\f-\\n-\\r-\\t"'
    let msg = 'String(\"-\\-\/-\b-\f-\n-\r-\t)'
    return test(JSON.stringify(s) == s1, msg) +
        test(JSON.parse<Object>(s1, stringType) as String == s, msg)
}

function testString(): int {
    let stringType = Type.of(new String() as Object)
    return test(JSON.parse<Object>("\"Gracias\"", stringType) as String == "Gracias", "String(Gracias)") +
        test(JSON.parse<Object>("\"\"", stringType) as String == "", "String()")
}

function testString02(): int {
    let stringType = Type.of(new String() as Object)
    let s = 'a/b//c///d////f'
    let s1 = '"a/b//c///d////f"'
    let msg = 'String(a/b//c///d////f)'
    return test(JSON.stringify(s) == s1, msg) +
        test(JSON.parse<Object>(s1, stringType) as String == s, msg)
}

function testString03(): int {
    let stringType = Type.of(new String() as Object)
    let s = 'https://p3-sign'
    let s1 = '"https://p3-sign"'
    let msg = 'String(https://p3-sign)'
    return test(JSON.stringify(s) == s1, msg) +
        test(JSON.parse<Object>(s1, stringType) as String == s, msg)
}

function fixedArrayEquals<E>(a: FixedArray<E>, b: FixedArray<E>): boolean {
    if (a.length != b.length) {
        return false
    }
    for (let i = 0; i < a.length; i++) {
        if (a[i] != b[i]) {
            return false
        }
    }
    return true
}

type OptBool = Boolean | undefined

function testArrayBooleanParse(): int {
    let failures = 0
    const emptyArray: FixedArray<OptBool> = []
    const expectedArray: FixedArray<OptBool> = [false, true]
    const booleanType = Class.of(emptyArray)
    const parsedArray = JSON.parse<FixedArray<OptBool>>("[false, true]", booleanType)!
    failures += test(fixedArrayEquals(parsedArray, expectedArray), "[false, true]")
    failures += test(fixedArrayEquals(JSON.parse<FixedArray<OptBool>>("[]", booleanType)!, emptyArray), "[]")
    return failures
}

function testArrayBooleanStringify(): int {
    let boxedBools = [new Boolean(true), new Boolean(false)]
    let bools = [true, false]

    return test(JSON.stringify(boxedBools) == JSON.stringify(bools), "[new Boolean(true), new Boolean(false)]")
}

type OptDouble = Double | undefined

function testArrayDouble(): int {
    let failures = 0
    const emptyArray: FixedArray<OptDouble> = []
    const testArray: FixedArray<OptDouble> = [0, -1, 1, 2.72182, -2.72182]
    const doubleType = Class.of(emptyArray)
    const parsedArray = JSON.parse<FixedArray<OptDouble>>("[0, -1, 1, 2.72182, -2.72182]", doubleType)!
    failures += test(fixedArrayEquals(parsedArray, testArray), "[0, -1, 1, 2.72182, -2.72182]")
    failures += test(fixedArrayEquals(JSON.parse<FixedArray<OptDouble>>("[]", doubleType)!, emptyArray), "[]")
    return failures
}

function testArrayString(): int {
    let failures = 0
    const emptyArray: FixedArray<String> = []
    const stringType = Class.of(emptyArray)
    const strArrayJson = '["White", "Rabbit", "was running away", "from Alice"]'
    const testArray: FixedArray<String> = ["White", "Rabbit", "was running away", "from Alice"]
    failures += test(fixedArrayEquals(JSON.parse<FixedArray<String>>(strArrayJson, stringType)!, testArray), strArrayJson)
    failures += test(fixedArrayEquals(JSON.parse<FixedArray<String>>("[]", stringType)!, emptyArray), "[]")
    return failures
}

function testObject(): int {
    let objectType = Type.of(new Object())
    return test(JSON.stringify(JSON.parse<Object>("{}", objectType) as Object) == "{}", "Object")
}

export class User {}

class UserWithFields extends User {
    id: Double = 0.0
    g: Boolean = false
    name: String = ""
}

class SuperUser extends UserWithFields {
    users: FixedArray<UserWithFields> = []
}

function testStringifyParse(): int {
    let classType = Type.of(new SuperUser() as Object)
    let str = "{\"id\":10,\"g\":true,\"name\":\"Name\",\"users\":[{\"id\":12,\"g\":true,\"name\":\"Name1\"},{\"id\":13,\"g\":false,\"name\":\"Name2\"}]}";
    return test(JSON.stringify(JSON.parse<SuperUser>(str, classType) as SuperUser) == str, "SuperUser")
}

class NoConstructor {
    x : double
    constructor(x : double) {
        this.x = x
    }
}

class ForbiddenPrimitives {
    x : int
}

class CycleA {
    // x : CycleB = new CycleB()
    x : CycleB | null = null
}

class CycleB {
    // x : CycleA = new CycleA()
    x : CycleA | null = null
}

class ManyFields {
    undef1 = undefined

    boxBool = new Boolean(true)
    boxBools = [new Boolean(true)]

    boxByte = new Byte(1 as byte)
    boxBytes = [new Byte(1 as byte)]

    boxShort = new Short(1 as short)
    boxShorts = [new Short(1 as short)]

    boxInt = new Int(1)
    boxInts = [new Int(1), new Int(1)]

    undef2 = undefined

    boxLong = new Long(1)
    boxLongs = [new Long(1)]

    boxFloat = new Float(1.1)

    // NOTE(cheezzario) without explicit type, reflection returns this field value as FixedArray<double> and 1.1 becomes 1.100000023841858
    boxFloats: FixedArray<Float> = [new Float(1.1)]

    boxDouble = new Double(1.1)
    boxDoubles = [new Double(1.1)]

    nan = NaN
    posinf = Infinity
    neginf = -Infinity

    nl: string | null = null

    static constant = "constant"

    lambda = () => "lambda"

    undef3 = undefined
}

class TimeWithToJson {
    hours: int
    minutes: int
    seconds: int

    constructor(hours: int, minutes: int, seconds: int) {
        this.hours = hours
        this.minutes = minutes
        this.seconds = seconds
    }

    toJSON(): string {
        return `${this.hours}:${this.minutes}:${this.seconds}`
    }
}

class TimeWithToJsonChild extends TimeWithToJson {
    offset: int

    constructor(time: TimeWithToJson, offset: int) {
        super(time.hours, time.minutes, time.seconds)
        this.offset = offset
    }
}

function testObjectFieldsStringify(): int {
    const expectedJson = `{"boxBool":true,"boxBools":[true],"boxByte":1,"boxBytes":[1],` +
            `"boxShort":1,"boxShorts":[1],"boxInt":1,"boxInts":[1,1],"boxLong":1,"boxLongs":[1],` +
            `"boxFloat":1.1,"boxFloats":[1.1],"boxDouble":1.1,"boxDoubles":[1.1],` +
            `"nan":null,"posinf":null,"neginf":null,"nl":null}`
    const actualJson = JSON.stringify(new ManyFields())

    let result = test(expectedJson == actualJson, "object with fields")
    if (result != RESULT_SUCCESS) {
        console.log(`expected "${expectedJson}" but was "${actualJson}"`)
    }

    return result
}

function testStringifyArray(): int {
    let result = test("[null]" == JSON.stringify([null]), "array of nulls")

    result += test("[null]" == JSON.stringify([undefined]), "array of 'undefine's")

    return result
}

function testStringifyObjectOfDerivedClass(): int {
    const expectedJson = `{"hours":10,"minutes":20,"seconds":30,"offset":3}`
    const actualJson = JSON.stringify(new TimeWithToJsonChild(new TimeWithToJson(10, 20, 30), 3))

    let result = test(expectedJson == actualJson, "object of derived class")
    
    if (result != RESULT_SUCCESS) {
        console.log(`expected ${expectedJson} but was ${actualJson}`)
    }

    return result
}

class TestNode {
    value: string
    next: Any

    constructor(value: string, next: Any = null) {
        this.value = value
        this.next = next
    }
}

function testStringifyObjectWithReferenceCycle(): int {
    const nodeA = new TestNode("A")
    const nodeB = new TestNode("B", nodeA)

    nodeA.next = nodeB

    let testSuccess = false
    try {
        const json = JSON.stringify(nodeA)
    } catch (ex) {
        if (ex instanceof TypeError) {
            testSuccess = true
        }
    }

    return test(testSuccess, "object with reference cycle")
}

function testStringifyArrayWithReferenceCycle(): int {
    const selfRefArray: FixedArray<Object> = [new Object()]
    selfRefArray[0] = selfRefArray

    let testSuccess = false
    try {
        const json = JSON.stringify(selfRefArray)
    } catch (ex) {
        if (ex instanceof TypeError) {
            testSuccess = true
        }
    }

    return test(testSuccess, "array with reference cycle")
}

function testStringifyLambda(): int {
    const lambda = () => "hello"

    let testResult = test("undefined" == JSON.stringify(lambda), "standalone lambda")
    testResult += test("[null]" == JSON.stringify([lambda]), "array of lambdas")

    return testResult
}

function testStringifyTuple(): int {
    const tuple: [string, string] = ["hello", "world"]

    let testResult = test('["hello","world"]' == JSON.stringify(tuple), "standalone tuple")
    testResult += test('[["hello","world"]]' == JSON.stringify([tuple]), "array of tuples")

    return testResult
}

function hello(): string {
    return "hello"
}

function testStringifyFunction(): int {
    const fn: () => string = hello

    let testResult = test("undefined" == JSON.stringify(fn), "standalone function")
    testResult += test("[null]" == JSON.stringify([fn]), "array of functions")

    return testResult
}

type Option<T> = T | undefined

class WithOption {
    opt: Option<string> = "hello"
}

function testStringifyUnion(): int {
    const option: Option<string> = "hello"

    let testResult = test('"hello"' == JSON.stringify(option), "standalone union")
    testResult += test('["hello"]' == JSON.stringify([option]), "array of unions")

    testResult += test('{"opt":"hello"}' == JSON.stringify(new WithOption()), "class with union typed field")

    return testResult
}

interface Fn<A, R> {
    apply(a: A): R
}

class IdentityFn<A> implements Fn<A, A> {
    apply(a: A): A {
        return a
    }
}

class WithFn {
    fn: Fn<number, number> = new IdentityFn<number>()
}

function testStringifyInterface(): int {
    const identityNumber: Fn<number, number> = new IdentityFn<number>()
    let testResult = test("{}" == JSON.stringify(identityNumber), "standalone interface")

    testResult += test('{"fn":{}}' == JSON.stringify(new WithFn()), "class with interface typed field")

    return testResult
}

enum TrafficLightColor {
    Red, Yellow, Green
}

class TrafficLight {
    color: int = TrafficLightColor.Yellow
}

function testStringifyEnum(): int {
    return test('{"color":1}' == JSON.stringify(new TrafficLight()), "enum field")
}

function testStringifyNullUndefined(): int {
    let testResult = test("null" == JSON.stringify(null), "standalone null")

    testResult += test("undefined" == JSON.stringify(undefined), "standalone undefined")

    return testResult
}

function testStringifyUnicodeStrings(): int {
    let testResult = test('"\\ud83d"' == JSON.stringify("\ud83d"), "unpaired high surrogate")
    
    testResult += test('"\\t\\ud83d"' == JSON.stringify("\t\ud83d"), "unpaired high surrogate with prefix")
    testResult += test('"\\ud83d\\t"' == JSON.stringify("\ud83d\t"), "unpaired high surrogate with suffix")
    testResult += test('"\\udc99"' == JSON.stringify("\udc99"), "unpaired low surrogate")
    testResult += test('"\\t\\udc99"' == JSON.stringify("\t\udc99"), "unpaired low surrogate with prefix")
    testResult += test('"\\udc99\\t"' == JSON.stringify("\udc99\t"), "unpaired low surrogate with suffix")
    testResult += test('"\\udc99\\ud83d"' == JSON.stringify("\udc99\ud83d"), "reversed surrogate pair")
    testResult += test('"\ud83d\udc99"' == JSON.stringify("\ud83d\udc99"), "valid surrogate pair")

    testResult += test(JSON.stringify("\uD834") == '"\\ud834"', 'JSON.stringify("\\uD834")')
    testResult += test(JSON.stringify("\uDF06") == '"\\udf06"', 'JSON.stringify("\\uDF06")')
    testResult += test(JSON.stringify ("\uD834\uDF06") == '"𝌆"', 'JSON.stringify("\\uD834\\uDF06")')
    testResult += test(JSON.stringify("\uD834\uD834\uDF06\uD834") == '"\\ud834𝌆\\ud834"', 'JSON.stringify("\\uD834\\uD834\\uDF06\\uD834")')
    testResult += test(JSON.stringify("\uDF06\uD834\uDF06\uD834") == '"\\udf06𝌆\\ud834"', 'JSON.stringify("\\uDF06\\uD834\\uDF06\\uD834")')
    testResult += test(JSON.stringify("\uDF06\uD834\uDF06\uD834") == '"\\udf06𝌆\\ud834"', 'JSON.stringify("\\uDF06\\uD834\\uDF06\\uD834")')
    testResult += test(JSON.stringify("\uDF06\uD834\uDF06\uDF06") == '"\\udf06𝌆\\udf06"', 'JSON.stringify("\\uDF06\\uD834\\uDF06\\uDF06")')
    testResult += test(JSON.stringify("\uDF06\uD834") == '"\\udf06\\ud834"', 'JSON.stringify("\\uDF06\\uD834")')
    testResult += test(JSON.stringify("\uD834\uDF06\uD834\uD834") == '"𝌆\\ud834\\ud834"', 'JSON.stringify("\\uD834\\uDF06\\uD834\\uD834")')
    testResult += test(JSON.stringify("\uD834\uDF06\uD834\uDF06") == '"𝌆𝌆"', 'JSON.stringify("\\uD834\\uDF06\\uD834\\uDF06")')
    testResult += test(JSON.stringify("\uDF06\uDF06\uD834\uD834") == '"\\udf06\\udf06\\ud834\\ud834"', 'JSON.stringify("\\uDF06\\uDF06\\uD834\\uD834")')
    testResult += test(JSON.stringify ("\uDF06\uDF06\uD834\uDF06") == '"\\udf06\\udf06𝌆"', 'JSON.stringify("\\uDF06\\uDF06\\uD834\\uDF06")')

    return testResult
}

class A {
    integerValue: int = 0;
    longValue: long = 1;
    shortValue: short = 1;
    floatValue: float = 1.2f;
    byteValue: byte = 1;
}

function testJSONParse(): int {
    let testResult: int = 0
    let text:string = `{\"integerValue\":12, \"longValue\": 2, \"appname\": \"test\", \"shortValue\":3, \"floatValue\":1.5, \"byteValue\": 7}`
    let classType = Type.of(new A())
    let parsingResult = JSON.parse<A>(text, classType) as A
    arktest.assertEQ(parsingResult.integerValue, 12)
    arktest.assertEQ(parsingResult.longValue, 2)
    arktest.assertEQ(parsingResult.shortValue, 3)
    arktest.assertEQ(parsingResult.floatValue, 1.5f)
    arktest.assertEQ(parsingResult.byteValue, 7)
    return testResult
}

function main(): int {
    let failures = 0
    failures += testNull()
    failures += testBoolean()
    failures += testDouble()
    failures += testStringWithEscape01()
    failures += testStringWithEscape02()
    failures += testString()
    failures += testString02()
    failures += testString03()
    failures += testArrayBooleanParse()
    failures += testArrayBooleanStringify()
    failures += testArrayDouble()
    failures += testArrayString()
    failures += testObject()
    failures += testStringifyParse()
    failures += testObjectFieldsStringify()
    failures += testStringifyArray()
    failures += testStringifyObjectOfDerivedClass()
    failures += testStringifyObjectWithReferenceCycle()
    failures += testStringifyArrayWithReferenceCycle()

    failures += testStringifyLambda()
    failures += testStringifyTuple()
    failures += testStringifyFunction()
    failures += testStringifyUnion()
    failures += testStringifyInterface()
    failures += testStringifyEnum()
    failures += testStringifyNullUndefined()
    failures += testStringifyUnicodeStrings()
    failures += testJSONParse()

    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}

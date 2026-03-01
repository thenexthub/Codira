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

function main(): int {
    const suite = new arktest.ArkTestsuite("JSON.stringify")

    suite.addTest("Stringify Date with negative number in constructor", stringifyNegativeDate)
    suite.addTest("Stringify object literal - simple cases", stringifyObjectLiteralSimpleCases)
    suite.addTest("Stringify object literal - corner cases", stringifyObjectLiteralCornerCases)

    // Typed array tests
    suite.addTest("Stringify typed arrays", testStringifyTypedArray1);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray2);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray3);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray4);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray5);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray6);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray7);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray8);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray9);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray10);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray11);
    suite.addTest("Stringify typed arrays", testStringifyTypedArray12);
    suite.addTest("Stringify Interface", testStringifyInterface01);
    suite.addTest("Stringify Interface", testStringifyInterface02);
    suite.addTest("Stringify Interface", testStringifyInterface03);
    suite.addTest("Stringify Interface", testStringifyInterface04);
    suite.addTest("Stringify Interface", testStringifyInterface05);

    return suite.run()
}

function stringifyNegativeDate() {
    let d = new Date(-271822)
    arktest.assertEQ(JSON.stringify(d), "\"1969-12-31T23:55:28.178Z\"")
}

class TestClass {
    value: string;
    constructor(v: string) { this.value = v }
}

interface TestInterface {
    s: string
    d: double
    a: Any
}

class TestInterfaceImpl implements TestInterface {
    s: string = ""
    d: double = 0
    a: Any = undefined
}

function checkJSONObject(jObj: JSONObject, expectedValues: Map<string, string>) {
    const flds = jObj.getFields()
    arktest.assertEQ(flds.size, expectedValues.size)
    for (const entry of expectedValues.entries()) {
        arktest.assertEQ(flds.get(entry[0])?.toString(), entry[1])
    }
}

function checkJSONArray(jsonArr: JSONArray, expectedValues: Map<string, string>[]) {
    const jsonArrValues = jsonArr.values
    arktest.assertEQ(jsonArrValues.length, expectedValues.length)
    for (let i = 0; i < jsonArrValues.length; i++) {
        checkJSONObject(jsonArrValues[i] as JSONObject, expectedValues[i])
    }
}

function stringifyObjectLiteralSimpleCases(): void {
    let obj: TestInterface = {s: "str", d: -1.5, a: undefined}
    arktest.assertTrue(reflect.isLiteralInitializedInterface(obj))

    // Expect {"d":-1.5,"s":"str"}
    let jsonStr = JSON.stringify(obj)
    let expected: Map<string, string> = new Map<string,string>([
        ["d", "-1.5"],
        ["s", "str"],
    ])
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    obj.a = new TestClass("val")
    // Expect {"a":{"value":"val"},"d":-1.5,"s":"str"}
    jsonStr = JSON.stringify(obj)
    expected.set("a", "{value:val}")
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    let array: TestInterface[] = [{s: "", d: 0.0, a: undefined}, {s: "s", d: 1.5, a: new TestClass("val")}]
    // Expect [{"d":0,"s":""},{"a":{"value":"val"},"d":1.5,"s":"s"}]
    jsonStr = JSON.stringify(array)
    expected.set("d", "1.5")
    expected.set("s", "s")
    const expectedArray: Map<string,string>[] = [new Map<string,string>([["d", "0"], ["s", ""]]), expected]
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, expectedArray)

    let fixedArray: FixedArray<TestInterface> = [{s: "", d: 0.0, a: undefined}, {s: "s", d: 1.5, a: new TestClass("val")}]
    // Expect [{"d":0,"s":""},{"a":{"value":"val"},"d":1.5,"s":"s"}]
    jsonStr = JSON.stringify(fixedArray)
    checkJSONArray(JSONParser.parse(jsonStr) as JSONArray, expectedArray)

    let record: Record<string, TestInterface[]> = new Record<string, TestInterface[]>()
    record["filler"] = array
    // Expect {"filler":[{"d":0,"s":""},{"a":{"value":"val"},"d":-1.5,"s":"str"}]}
    jsonStr = JSON.stringify(record)
    const jsonObj = JSONParser.parse(jsonStr) as JSONObject
    const fields = jsonObj.getFields()
    arktest.assertEQ(fields.size, 1)
    checkJSONArray(fields.get("filler") as JSONArray, expectedArray)
}

function stringifyObjectLiteralCornerCases(): void {
    const testObj = new TestClass("val")
    let objLit: TestInterface = {s: "str", d: -1.5, a: testObj}
    arktest.assertTrue(reflect.isLiteralInitializedInterface(objLit))

    // Expect {"a":{"value":"val"},"d":-1.5,"s":"str"}
    let jsonStr = JSON.stringify(objLit)
    let expected: Map<string, string> = new Map<string,string>([
        ["a", "{value:val}"],
        ["d", "-1.5"],
        ["s", "str"],
    ])
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check that replacer is applied
    // Expect {"a":123,"d":-1.5,"s":"str"}
    jsonStr = JSON.stringify(objLit, (key: String, value: Any): Any => { return (key == "a") ? 123 : value })
    expected.set("a", "123")
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check that fields filter is not applied to object literal itself, but is applied to all nested objects
    // Expect {"s":"str"}
    jsonStr = JSON.stringify(objLit, ["s"])
    expected.delete('a')
    expected.delete('d')
    checkJSONObject(JSONParser.parse(jsonStr) as JSONObject, expected)

    // Check cyclic dependencies are detected
    objLit.a = objLit;
    arktest.expectThrow(() => { JSON.stringify(objLit) }, (e: Error) => e instanceof TypeError)
    let array: TestInterface[] = [objLit]
    arktest.expectThrow(() => { JSON.stringify(array) }, (e: Error) => e instanceof TypeError)
    let fixedArray: FixedArray<TestInterface> = [objLit]
    arktest.expectThrow(() => { JSON.stringify(fixedArray) }, (e: Error) => e instanceof TypeError)
    objLit.a = testObj

}

interface GeneratedObjectLiteralInterface_1 {
    id: number;
    values: Uint32Array
}

interface GeneratedObjectLiteralInterface_2 {
    name: string;
    buffer: Int8Array;
}

interface GeneratedObjectLiteralInterface_3 {
    count: number;
    data: Uint8Array;
}

interface GeneratedObjectLiteralInterface_4 {
    flags: boolean;
    values: Int16Array;
}

interface GeneratedObjectLiteralInterface_5 {
    flags: boolean;
    values: Float32Array;
}

interface GeneratedObjectLiteralInterface_6 {
    flags: boolean;
    values: Float64Array;
}

interface GeneratedObjectLiteralInterface_7 {
    name: string;
    buffer: Uint16Array;
}

interface GeneratedObjectLiteralInterface_8 {
    count: number;
    data: Int32Array;
}

interface GeneratedObjectLiteralInterface_9 {
    flags: boolean;
    values: Uint8ClampedArray;
}

interface GeneratedObjectLiteralInterface_10 {
    flags: boolean;
    values: BigInt64Array;
}

interface GeneratedObjectLiteralInterface_11 {
    flags: boolean;
    values: BigUint64Array;
}

interface GeneratedObjectLiteralInterface_12 {
    flags: boolean;
    values: ArrayLike<Object>;
}

function testStringifyTypedArray1(): void {
    let a = new Uint32Array([10]);
    let data: GeneratedObjectLiteralInterface_1 = {
        id: 1,
        values: a
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getInteger("id"),  1);
    arktest.assertEQ(jsonElement.getElement("values").getInteger("0"),  10);
}

function testStringifyTypedArray2(): void {
    let buffer = new Int8Array([-10]);
    let data: GeneratedObjectLiteralInterface_2 = {
        name: "test",
        buffer: buffer
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getString("name"),  "test");
    arktest.assertEQ(jsonElement.getElement("buffer").getInteger("0"),  -10);
}

function testStringifyTypedArray3(): void {
    let dataArr = new Uint8Array([255]);
    let data: GeneratedObjectLiteralInterface_3 = {
        count: 5,
        data: dataArr,
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getInteger("count"),  5);
    arktest.assertEQ(jsonElement.getElement("data").getInteger("0"),  255);
}

function testStringifyTypedArray4(): void {
    let values = new Int16Array([1000]);
    let data: GeneratedObjectLiteralInterface_4 = {
        flags: true,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getElement("values").getInteger("0"),  1000);
}

function testStringifyTypedArray5(): void {
    let values = new Float32Array([1.0]);
    let data: GeneratedObjectLiteralInterface_5 = {
        flags: true,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getElement("values").getDouble("0"),  1.0);
}

function testStringifyTypedArray6(): void {
    let values = new Float64Array([3.14159]);
    let data: GeneratedObjectLiteralInterface_6 = {
        flags: true,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getElement("values").getDouble("0"),  3.14159);
}

function testStringifyTypedArray7(): void {
    let buffer = new Uint16Array([0]);
    let data: GeneratedObjectLiteralInterface_7 = {
        name: "test",
        buffer: buffer
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getString("name"),  "test");
    arktest.assertEQ(jsonElement.getElement("buffer").getInteger("0"),  0);
}

function testStringifyTypedArray8(): void {
    let dataArr = new Int32Array([255]);
    let data: GeneratedObjectLiteralInterface_8 = {
        count: 5,
        data: dataArr
    };
    let jsonStr = JSON.stringify(data)

    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getInteger("count"),  5);
    arktest.assertEQ(jsonElement.getElement("data").getInteger("0"),  255);
}

function testStringifyTypedArray9(): void {
    let values = new Uint8ClampedArray([1]);
    let data: GeneratedObjectLiteralInterface_9 = {
        flags: true,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getElement("values").getInteger("0"),  1);
}

function testStringifyTypedArray10(): void {
    let values = new BigInt64Array([200000]);
    let data: GeneratedObjectLiteralInterface_10 = {
        flags: false,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  false);
    arktest.assertEQ(jsonElement.getElement("values").getInteger("0"),  200000);
}

function testStringifyTypedArray11(): void {
    let values = new BigUint64Array([100000]);
    let data: GeneratedObjectLiteralInterface_11 = {
        flags: true,
        values: values
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getElement("values").getInteger("0"),  100000);
}
function checkArray(): void {
    
}

class A{
    public num1: int
    constructor(num1: int){
        this.num1 = num1
    }
};
function testStringifyTypedArray12(): void {
    let a = new A(1);
    let objArray: ArrayLike<Object> = [a];
    let data: GeneratedObjectLiteralInterface_12 = {
        flags: true,
        values: objArray
    };
    let jsonStr = JSON.stringify(data)
    let jsonElement = JSON.parseJsonElement(jsonStr)
    arktest.assertEQ(jsonElement.getBoolean("flags"),  true);
    arktest.assertEQ(jsonElement.getArray("values")[0].getInteger("num1"), 1);
}

interface I1 {
    str1: string
}

interface I2 {
    str1: string
    num1: int
}

interface I3 {
    str2: string
    num2: int
}

class A1 implements I1 {
    str1: string = '1'
}
function testStringifyInterface01(): void {
    let str = JSON.stringify(new A1)
    let jsonElement = JSON.parseJsonElement(str)
    arktest.assertEQ(jsonElement.getString('str1'), '1')
}

class B1 implements I2 {
    str1: string = 'str'
    num1: int = 10
}
function testStringifyInterface02(): void {
    let str = JSON.stringify(new B1)
    let jsonElement = JSON.parseJsonElement(str)
    arktest.assertEQ(jsonElement.getString('str1'), 'str')
    arktest.assertEQ(jsonElement.getInteger('num1'), 10)
}

class C2 implements I2, I3 {
    str1: string = 'yes'
    num1: int = 10
    str2: string = 'no'
    num2: int = 100
}
function testStringifyInterface03(): void {
    let str = JSON.stringify(new C2)
    let jsonElement = JSON.parseJsonElement(str)
    arktest.assertEQ(jsonElement.getString('str1'), 'yes')
    arktest.assertEQ(jsonElement.getInteger('num1'), 10)
    arktest.assertEQ(jsonElement.getString('str2'), 'no')
    arktest.assertEQ(jsonElement.getInteger('num2'), 100)
}

class D1 implements I1 {
    str1: string = 'str'
}

class D2 extends D1 {

}
function testStringifyInterface04(): void {
    let str = JSON.stringify(new D2)
    let jsonElement = JSON.parseJsonElement(str)
    arktest.assertEQ(jsonElement.getString('str1'), 'str')
}

class E1 implements I1 {
    str1: string = 'str'
}

class E2 extends E1 implements I3{
    str2: string = 'yes'
    num2: int = 1
}
function testStringifyInterface05(): void {
    let str = JSON.stringify(new E2)
    let jsonElement = JSON.parseJsonElement(str)
    arktest.assertEQ(jsonElement.getString('str1'), 'str')
    arktest.assertEQ(jsonElement.getString('str2'), 'yes')
    arktest.assertEQ(jsonElement.getInteger('num2'), 1)
}
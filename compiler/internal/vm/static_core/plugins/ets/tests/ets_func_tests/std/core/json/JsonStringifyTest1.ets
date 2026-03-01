/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

const RESULT_SUCCESS = 0
const RESULT_FAILURE = 1

const INT8_MAX: number = Byte.MAX_VALUE; //127
const INT8_MIN: number = Byte.MIN_VALUE; //-128

const UINT8_MAX: number = INT8_MAX * 2 + 1; //255
const UINT8_MIN: number = 0;

const INT16_MAX: number = Short.MAX_VALUE; // 32767
const INT16_MIN: number = Short.MIN_VALUE; // -32768

const UINT16_MAX: number = INT16_MAX * 2 + 1; //65535
const UINT16_MIN: number = 0;

const INT32_MAX: number = Int.MAX_VALUE;
const INT32_MIN: number = Int.MIN_VALUE;

const UINT32_MAX: number = INT32_MAX * 2 + 1; // 4294967295
const UINT32_MIN: number = 0;

const FLOAT32_MAX: number = Float.MAX_VALUE; // 3.4028235e+38
const FLOAT32_MIN: number = Float.MIN_VALUE; // 1.4e-45

const FLOAT64_MAX: number = Double.MAX_VALUE; // 1.7976931348623157e+308
const FLOAT64_MIN: number = Double.MIN_VALUE; // 5e-324

class TestClass {
    value: string;
    constructor(v: string) {
        this.value = v;
    }
}

function jsonStringifyArrayBuffer(): void {
    let emptyBuffer = new ArrayBuffer(0);
    arktest.assertEQ('{}', JSON.stringify(emptyBuffer));

    let largeBuffer = new ArrayBuffer(1024);
    arktest.assertEQ('{}', JSON.stringify(largeBuffer));
}

 function jsonStringifyUint8Array(): void {
    let value: Uint8Array = Uint8Array.of(UINT8_MIN - 1, UINT8_MIN, UINT8_MAX, UINT8_MAX + 1);
    arktest.assertEQ('{"0":255,"1":0,"2":255,"3":0}', JSON.stringify(value));
}

function jsonStringifyInt8Array(): void {
    let value: Int8Array = Int8Array.of(INT8_MAX,INT8_MIN,100,INT8_MIN + 1);
    arktest.assertEQ('{"0":127,"1":-128,"2":100,"3":-127}', JSON.stringify(value));
}

function jsonStringifyUint8ClampedArray(): void {
    let value: Uint8ClampedArray = Uint8ClampedArray.of(UINT8_MIN - 1, UINT8_MIN, UINT8_MAX, UINT8_MAX + 1);
    arktest.assertEQ('{"0":0,"1":0,"2":255,"3":255}', JSON.stringify(value));
}

function jsonStringifyInt16Array(): void {
    let value: Int16Array = Int16Array.of(INT16_MIN,INT16_MAX,12345,-12345);
    arktest.assertEQ('{"0":-32768,"1":32767,"2":12345,"3":-12345}', JSON.stringify(value));
}

function jsonStringifyUint16Array(): void {
    let value: Uint16Array = Uint16Array.of(UINT16_MIN,UINT16_MAX);
    arktest.assertEQ('{"0":0,"1":65535}', JSON.stringify(value));
}

function jsonStringifyInt32Array(): void {
    let value: Int32Array = Int32Array.of(INT32_MIN,INT32_MAX);
    arktest.assertEQ('{"0":-2147483648,"1":2147483647}', JSON.stringify(value));
}

function jsonStringifyUint32Array(): void {
    let uint32Array = new Uint32Array(5);
    let array = [UINT32_MIN,UINT32_MAX,INT32_MIN,INT32_MAX];
    let uint32ArrayFromArray = new Uint32Array(array);
    arktest.assertEQ('{"0":0,"1":4294967295,"2":2147483648,"3":2147483647}', JSON.stringify(uint32ArrayFromArray));
}

function jsonStringifyFloat32Array(): void {
    let value: Float32Array = Float32Array.of(FLOAT32_MIN,FLOAT32_MAX);
    arktest.assertEQ('{"0":1.401298464324817e-45,"1":3.4028234663852886e+38}', JSON.stringify(value));
}

function jsonStringifyFloat64Array(): void {
    let value: Float64Array = Float64Array.of(FLOAT64_MIN,FLOAT64_MAX);
    arktest.assertEQ('{"0":5e-324,"1":1.7976931348623157e+308}', JSON.stringify(value));
}

function jsonStringifyPromise(): void {
    let pro = new Promise<Object>((resolve: (value: Object) => void): void => {});
    arktest.assertEQ('{}', JSON.stringify(pro));
}

function jsonStringifySet(): void {
    let set: Set<string> = new Set<string>();
    set.add("foo");
    arktest.assertEQ("{}", JSON.stringify(set));
}

function jsonStringifyMapEmpty(): void {
    const map1 = new Map<string, string>();
    arktest.assertEQ("{}", JSON.stringify(map1));
}

function jsonStringifyMapWithEntries(): void {
    const map2 = new Map<string, string>();
    map2.set("key1", "value1");
    arktest.assertEQ("{}", JSON.stringify(map2));
}

function jsonStringifyRecord(): void {
    let record: Record<string | number, Any> = {}
    record["name"] = "arkts";
    record["version"] = 1;

    const fn: () => void = () => {}
    record[2] = fn

    let jsonStr = JSON.stringify(record);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    arktest.assertEQ(fields.get("name")?.toString(), "arkts");
    arktest.assertEQ(fields.get("version")?.toString(), "1");
    arktest.assertTrue(fields.get("2") === undefined)
}

function jsonStringifyRecordWithSpace(): void {
    const rec: Record<string, Any> = { 'name': 'Rec', 'version': 2 }
    const recJson = JSON.stringify(rec, undefined, ' ')
    arktest.assertEQ(recJson, '{\n "name": "Rec",\n "version": 2\n}')
}

function jsonStringifyRecordWithUndefinedValue(): void {
    let record: Record<string, Any> = new Record<string, Any>();
    record["name"] = "arkts";
    record["version"] = 1.0;
    record["ignore"] = undefined;

    let jsonStr = JSON.stringify(record);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    arktest.assertEQ(fields.get("name")?.toString(), "arkts");
    arktest.assertEQ(fields.get("version")?.toString(), "1");
    arktest.assertEQ(fields.has("ignore"), false);
}

function jsonStringifyRecordWithUndefinedValueWithSpace(): void {
    const record: Record<string, Any> = { 'name': 'arkts', 'version': 1, 'ignore': undefined }
    const recordJson = JSON.stringify(record, undefined, ' ')
    arktest.assertEQ(recordJson, '{\n "name": "arkts",\n "version": 1\n}')
}

function jsonStringifyNestedRecord(): void {
    let record: Record<string, Any> = new Record<string, Any>();
    record["name"] = "arkts";
    record["version"] = 1.0;

    const nestedRecord: Record<string, Any> = new Record<string, Any>();
    nestedRecord["inner"] = record;
    nestedRecord["desc"] = new String("nested");

    let jsonStr = JSON.stringify(nestedRecord);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    let innerObj = fields.get("inner") as JSONObject;
    let innerFields = innerObj.getFields();

    arktest.assertEQ(innerFields.get("name")?.toString(), "arkts");
    arktest.assertEQ(innerFields.get("version")?.toString(), "1");
    arktest.assertEQ(fields.get("desc")?.toString(), "nested");
}

function jsonStringifyNestedRecordWithSpace(): void {
    const record: Record<string, Any> = { 'name': 'arkts', 'version': 1}
    const nestedRecord: Record<string, Any> = { 'desc': 'nested' }
    record['nested'] = nestedRecord

    const recordJson = JSON.stringify(record, undefined, ' ')
    arktest.assertEQ(recordJson, '{\n "name": "arkts",\n "version": 1,\n "nested": {\n  "desc": "nested"\n }\n}')
}

function jsonStringifyRecordWithClassInstance(): void {
    let record: Record<string, Any> = new Record<string, Any>();
    record["TestClass"] = new TestClass('Test');

    let jsonStr = JSON.stringify(record);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    let classObj = fields.get("TestClass") as JSONObject;
    let classFields = classObj.getFields();

    arktest.assertEQ(classFields.get("value")?.toString(), "Test");
}

function jsonStringifyRecordWithClassInstanceWithSpace(): void {
    // CC-OFFNXT(G.NAM.03) implementation feature
    const record: Record<string, Any> = { 'TestClass': new TestClass('Test') }
    const recordJson = JSON.stringify(record, undefined, ' ')
    arktest.assertEQ(recordJson, '{\n "TestClass": {\n  "value": "Test"\n }\n}')
}

function jsonStringifyRecordWithArray(): void {
    let record: Record<string, Any> = new Record<string, Any>();
    record['list'] = new Array<Number>(1, 2, 3);

    let jsonStr = JSON.stringify(record);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    let arr = fields.get('list') as JSONArray;
    let elements = arr.values;

    arktest.assertEQ(elements.length, 3);
    arktest.assertEQ(elements[0]?.toString(), "1");
    arktest.assertEQ(elements[1]?.toString(), "2");
    arktest.assertEQ(elements[2]?.toString(), "3");
}

function jsonStringifyRecordWithArrayWithSpace(): void {
    const record: Record<string, Any> = { 'list': [1, 2, 3] }
    arktest.assertEQ(JSON.stringify(record, undefined, ' '), '{\n "list": [\n  1,\n  2,\n  3\n ]\n}')
}

function jsonStringifyRecordWithNull(): void {
    let record: Record<string, Any> = new Record<string, Any>();
    record['nullable'] = null;

    let jsonStr = JSON.stringify(record);
    let jsonObj = JSONParser.parse(jsonStr) as JSONObject;
    let fields = jsonObj.getFields();

    let nullableVal = fields.get('nullable');
    arktest.assertEQ(nullableVal instanceof JSONNull, true);
}

function jsonStringifyRecordWithNullWithSpace(): void {
    const record: Record<string, Any> = { 'nullable': null }
    arktest.assertEQ(JSON.stringify(record, undefined, ' '), '{\n "nullable": null\n}')
}

function jsonStringifyEmptyRecord(): void {
    const record: Record<string, Any> = {}
    arktest.assertEQ(JSON.stringify(record), '{}')
}

function jsonStringifyRecordWithNumericKey(): void {
    let recordSimpleNumeric: Record<string | number, number | string | boolean> = { "1": 1, 2: "2", 3: true }
    arktest.assertEQ(JSON.stringify(recordSimpleNumeric), '{"1":1,"2":"2","3":true}')

    let recordWithCollisions: Record<string | number, number | string | boolean> = { "1": 1, 1: "1", 2: "2", 3: true }
    arktest.assertEQ(JSON.stringify(recordWithCollisions), '{"1":1,"1":"1","2":"2","3":true}')
}

function jsonStringifyRecordWithSameValue(): void {
    let record: Record<string, Any> = {};
    let obj = new TestClass('Test');
    record['a'] = obj;
    record['b'] = obj;
    arktest.assertEQ(JSON.stringify(record), '{"a":{"value":"Test"},"b":{"value":"Test"}}');
}

function jsonStringifyRecordWithEmpty(): void {
    const record: Record<string, Any> = new Record<string, Any>()
    arktest.assertEQ(JSON.stringify(record), '{}')
}

function jsonStringifyNestedMap(): void {
    const map = new Map<string, Object>();
    let record: Record<string, Any> = new Record<string, Any>();
    record["name"] = "arkts";
    map.set("nested", record);

    arktest.assertEQ("{}", JSON.stringify(map));
}

function jsonStringifyCyclicRecord(): void {
    const cycleRecord = new Record<string, Any>();
    cycleRecord["self"] = cycleRecord;
    try {
        JSON.stringify(cycleRecord);
        throw new Error("Expected JSON.stringify to throw on cyclic structure, but it did not.");
    } catch (e) {
        arktest.assertEQ("cyclic object value", (e as Error).message);
    }
}

function jsonStringifyControlCharacter(): void {
    let res: FixedArray<String> = ['"\\u0000"', '"\\u0001"', '"\\u0002"', '"\\u0003"', '"\\u0004"', '"\\u0005"', '"\\u0006"', '"\\u0007"',
                                    '"\\b"', '"\\t"', '"\\n"', '"\\u000b"', '"\\f"', '"\\r"', '"\\u000e"', '"\\u000f"', '"\\u0010"',
                                    '"\\u0011"', '"\\u0012"', '"\\u0013"', '"\\u0014"', '"\\u0015"', '"\\u0016"', '"\\u0017"', '"\\u0018"',
                                    '"\\u0019"', '"\\u001a"', '"\\u001b"', '"\\u001c"', '"\\u001d"', '"\\u001e"', '"\\u001f"'];
    for (let i = 0; i < 32; i++) {
        arktest.assertEQ(JSON.stringify(String.fromCharCode(i)), res[i]);
    }
}

function jsonStringifyEscapeCharacter(): void {
    let arr: FixedArray<String> = ["\a", "\b", "\f", "\n", "\r", "\t", "\v", "\\", "\'", "\"", "\?", "\0"];
    let res: FixedArray<String> = ['"a"', '"\\b"', '"\\f"', '"\\n"', '"\\r"', '"\\t"', '"\\u000b"', '"\\\\"', "\"'\"", '"\\""', '"?"', '"\\u0000"'];
    for (let i = 0; i < arr.length; i++) {
        arktest.assertEQ(JSON.stringify(arr[i]), res[i]);
    }
}

function jsonStringifyTupleWithFunction(): void {
    let add = (a:number, b:number) => a + b;
    let sub = (a:number, b:number) => a - b;
    let set = new Set<Function>([add, sub]);
    let res = JSON.stringify(Array.from(set.entries()));
    arktest.assertEQ(res, '[[null,null],[null,null]]');

    let data: [int, int | undefined, String | null, Function, String] = [1, undefined, null, add, 'test'];
    res = JSON.stringify(data);
    arktest.assertEQ(res, '[1,null,null,null,"test"]');
}

function jsonStringifyTupleWithUndefinedValue(): void {
    const odds = new Set<undefined>([undefined]);
    let set = odds.entries();
    // NOTE: rewrite this comparison after #28848 solved
    arktest.assertEQ(JSON.stringify(set.next().value), "[null,null]");
}

enum EnumTest {
    e1 = 2.0,
    e2 = 1.23,
    e3 = 5.1,
}

enum EnumTestStr {
    s1 = 'abc',
    s2 = 'def',
}

class EnumClass {
    c1: EnumTest = EnumTest.e2
    c2: EnumTestStr = EnumTestStr.s1
    c3: Array<EnumTestStr | EnumTest> = [EnumTestStr.s2, EnumTestStr.s1, EnumTest.e1, EnumTest.e3]
}

function jsonStringifyEnum(): void {
    const obj = new EnumClass()
    arktest.assertEQ(JSON.stringify(obj), '{"c1":1.23,"c2":"abc","c3":["def","abc",2,5.1]}');
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Json stringify for ArrayLike tests ")
    suite.addTest('JSON.stringify for ArrayBuffer', jsonStringifyArrayBuffer);
    suite.addTest('JSON.stringify for Uint8Array', jsonStringifyUint8Array)
    suite.addTest('JSON.stringify for Int8Array', jsonStringifyInt8Array)
    suite.addTest('JSON.stringify for Uint8ClampedArray', jsonStringifyUint8ClampedArray)
    suite.addTest('JSON.stringify for Int16Array', jsonStringifyInt16Array)
    suite.addTest('JSON.stringify for Uint16Array', jsonStringifyUint16Array)
    suite.addTest('JSON.stringify for Int32Array', jsonStringifyInt32Array)
    suite.addTest('JSON.stringify for Uint32Array', jsonStringifyUint32Array)
    suite.addTest('JSON.stringify for Float32Array', jsonStringifyFloat32Array)
    suite.addTest('JSON.stringify for Float64Array', jsonStringifyFloat64Array)
    suite.addTest('JSON.stringify for Promise', jsonStringifyPromise)
    suite.addTest('JSON.stringify for Set', jsonStringifySet)
    suite.addTest('JSON.stringify for empty Map', jsonStringifyMapEmpty);
    suite.addTest('JSON.stringify for Map with entries', jsonStringifyMapWithEntries);
    suite.addTest('JSON.stringify for Record', jsonStringifyRecord);
    suite.addTest('JSON.stringify Record with space', jsonStringifyRecordWithSpace)
    suite.addTest('JSON.stringify for Record with undefined value', jsonStringifyRecordWithUndefinedValue);
    suite.addTest('JSON.stringify for Record with undefined value with space', jsonStringifyRecordWithUndefinedValueWithSpace);
    suite.addTest('JSON.stringify for nested Record', jsonStringifyNestedRecord);
    suite.addTest('JSON.stringify for nested Record with space', jsonStringifyNestedRecordWithSpace)
    suite.addTest('JSON.stringify for Record with class instance', jsonStringifyRecordWithClassInstance);
    suite.addTest('JSON.stringify for Record with class instance with space', jsonStringifyRecordWithClassInstanceWithSpace);
    suite.addTest('JSON.stringify for Record with array', jsonStringifyRecordWithArray);
    suite.addTest('JSON.stringify for Record with array with space', jsonStringifyRecordWithArrayWithSpace);
    suite.addTest('JSON.stringify for Record with null', jsonStringifyRecordWithNull);
    suite.addTest('JSON.stringify for Record with null with space', jsonStringifyRecordWithNullWithSpace);
    suite.addTest('JSON.stringify for empty Record', jsonStringifyEmptyRecord);
    suite.addTest('JSON.stringify for Record with numeric key', jsonStringifyRecordWithNumericKey);
    suite.addTest('JSON.stringify for Record with same object as value', jsonStringifyRecordWithSameValue);
    suite.addTest("JSON.stringify for Record with empty", jsonStringifyRecordWithEmpty);
    suite.addTest('JSON.stringify for nested Map', jsonStringifyNestedMap);
    suite.addTest('JSON.stringify for cyclic Record', jsonStringifyCyclicRecord);
    suite.addTest('JSON.stringify for control character', jsonStringifyControlCharacter);
    suite.addTest('JSON.stringify for escape character', jsonStringifyEscapeCharacter);
    suite.addTest('JSON.stringify for tuple with function', jsonStringifyTupleWithFunction);
    suite.addTest('JSON.stringify for tuple with undefined value', jsonStringifyTupleWithUndefinedValue);
    suite.addTest('JSON.stringify for enum', jsonStringifyEnum);

    return suite.run()
}

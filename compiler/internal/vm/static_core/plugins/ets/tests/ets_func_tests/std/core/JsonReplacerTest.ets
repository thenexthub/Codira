/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function dummyReplacer(key: string, value: Any): Any {
    return value
}

function toStringReplacer(key: string, value: Any): Any {
    if (key == '') {
        return value
    }

    return '' + value
}

function toFnReplacer(key: string, value: Any): Any {
    return key == 'id' ? () => {} : value
}

class A {
    public id: int = 1
    public name = 'A'

    constructor(id: int = 1) {
        this.id = id
    }

    toString(): string {
        return `${this.name}(${this.id})`
    }
}

class R {
    public a = new A()
}

class E {}

class N {
    public n = null
    public u = undefined
}

class C {
    public age: number = 0
    public name: string = ''
}

class O<T> implements JsonReplacer {
    private buffer: Array<T>
    private capacity: int = 20

    constructor(capacity: int) {
        this.capacity = capacity;
        this.buffer = new Array<T>(this.capacity)
        this.buffer.length = 0
    }

    pushBack(val: T): void {
        this.buffer.push(val)
    }

    jsonReplacer(): Record<String, Any> {
        const buf = this.buffer
        const len = buf.length
        let arrayObj: Record<String, Any> = {}
        for (let i = 0; i < len; i++) {
            arrayObj[String(i)] = buf[i]
        }
        return arrayObj
    }
}

class D<T> {
    public value: O<T>
    public capacity: int

    constructor(capacity: int) {
        this.capacity = capacity
        this.value = new O<T>(capacity)
    }

    pushBack(val: T): void {
        this.value.pushBack(val)
    }
}

function jsonStringifyObjectWithFunctionReplacer(): void {
    arktest.assertEQ(JSON.stringify(new A(), dummyReplacer), `{"id":1,"name":"A"}`)
    arktest.assertEQ(JSON.stringify(new A(), dummyReplacer, '\t'), `{\n\t"id": 1,\n\t"name": "A"\n}`)
    arktest.assertEQ(JSON.stringify(new A(), dummyReplacer, 1), `{\n "id": 1,\n "name": "A"\n}`)
    arktest.assertEQ(JSON.stringify(new A(), toStringReplacer), `{"id":"1","name":"A"}`)
    arktest.assertEQ(JSON.stringify(new N(), toStringReplacer), `{"n":"null","u":"undefined"}`)
    arktest.assertEQ(JSON.stringify(new A(), toFnReplacer), `{"name":"A"}`)
}

function jsonStringifyRecordWithFunctionReplacer(): void {
    const record: Record<string, Any> = { 'id': 1, 'name': 'R' }
    arktest.assertEQ(JSON.stringify(record, dummyReplacer), `{"id":1,"name":"R"}`)
    arktest.assertEQ(JSON.stringify(record, toFnReplacer), `{"name":"R"}`)
}

function arrayReverser(key: string, value: Any): Any {
    if (Array.isArray(value)) {
        return (value as Array<Any>).reverse()
    }

    const valueType = Class.ofAny(value)
    if (valueType !== undefined && valueType.isFixedArray()) {
        return Array.from<Any>(value as FixedArray<Any>).reverse()
    }

    return value
}

function jsonStringifyArrayWithFunctionReplacer(): void {
    const replacer = (key: string, value: Any): Any => {
        if (key == '' || key == '0') {
            return value
        } else {
            return undefined
        }
    }

    const array: Array<A> = Array.of(new A(), new A())
    arktest.assertEQ(JSON.stringify(array, dummyReplacer), '[{"id":1,"name":"A"},{"id":1,"name":"A"}]')
    arktest.assertEQ(JSON.stringify(array, toStringReplacer), '["A(1)","A(1)"]')
    arktest.assertEQ(JSON.stringify(array, toStringReplacer, '\t'), '[\n\t"A(1)",\n\t"A(1)"\n]')
    arktest.assertEQ(JSON.stringify(array, toStringReplacer, 1), '[\n "A(1)",\n "A(1)"\n]')
    arktest.assertEQ(JSON.stringify(array, replacer), '[{},null]')

    const ar = [new A(), new A()]
    arktest.assertEQ(JSON.stringify(ar, dummyReplacer), '[{"id":1,"name":"A"},{"id":1,"name":"A"}]')
    arktest.assertEQ(JSON.stringify(ar, toStringReplacer), '["A(1)","A(1)"]')
    arktest.assertEQ(JSON.stringify(ar, toStringReplacer, '\t'), '[\n\t"A(1)",\n\t"A(1)"\n]')
    arktest.assertEQ(JSON.stringify(ar, toStringReplacer, 1), '[\n "A(1)",\n "A(1)"\n]')
    arktest.assertEQ(JSON.stringify(ar, replacer), '[{},null]')

    const testArr: FixedArray<A> = [new A(1), new A(2)]
    arktest.assertEQ(JSON.stringify(testArr, arrayReverser), '[{"id":2,"name":"A"},{"id":1,"name":"A"}]')

    const numbers: FixedArray<number> = [1, 2, 3]
    arktest.assertEQ(JSON.stringify(numbers, dummyReplacer), '[1,2,3]')
    arktest.assertEQ(JSON.stringify(numbers, toStringReplacer), '["1","2","3"]')

    arktest.assertEQ(JSON.stringify(1, toStringReplacer), '1')
}

const EMPTY_ARRAY: FixedArray<string> = []

function jsonStringifyWithArrayReplacer(): void {
    const rec: Record<string, Any> = { 'id': 1, 'name': 'R' }
    arktest.assertEQ(JSON.stringify(new A(), EMPTY_ARRAY), '{}')
    arktest.assertEQ(JSON.stringify(rec, EMPTY_ARRAY), '{}')
    arktest.assertEQ(JSON.stringify(new A(), ['id']), '{"id":1}')
    arktest.assertEQ(JSON.stringify(rec, ['id']), '{"id":1}')
    arktest.assertEQ(JSON.stringify(new A(), ['name', 'id']), '{"name":"A","id":1}')
    arktest.assertEQ(JSON.stringify(rec, ['name', 'id']), '{"name":"R","id":1}')

    arktest.assertEQ(JSON.stringify([true], EMPTY_ARRAY), '[true]')

    const filter: FixedArray<string | number> = ['id', 0]
    arktest.assertEQ(JSON.stringify(new A(), filter), '{"id":1}')
    arktest.assertEQ(JSON.stringify(rec, filter), '{"id":1}')

    // space cases
    arktest.assertEQ(JSON.stringify(new A(), EMPTY_ARRAY, '\t'), '{}')
    arktest.assertEQ(JSON.stringify(new A(), ['id'], '\t'), '{\n\t"id": 1\n}')
    arktest.assertEQ(JSON.stringify(rec, ['id'], '\t'), '{\n\t"id": 1\n}')
    arktest.assertEQ(JSON.stringify(new R(), ['a'], '\t'), '{\n\t"a": {}\n}')
    arktest.assertEQ(JSON.stringify([new A(), new A()], EMPTY_ARRAY, '\t'), '[\n\t{},\n\t{}\n]')
    arktest.assertEQ(JSON.stringify([rec, rec], EMPTY_ARRAY, '\t'), '[\n\t{},\n\t{}\n]')
}

function jsonStringifyWithNullReplacer(): void {
    arktest.assertEQ(JSON.stringify(new A(), null), '{"id":1,"name":"A"}')
    const rec: Record<string, Any> = { 'id': 1, 'name': 'R' }
    arktest.assertEQ(JSON.stringify(rec, null), '{"id":1,"name":"R"}')
    arktest.assertEQ(JSON.stringify(new E(), null, '  '), '{}')
    const emptyRec: Record<string, Any> = {}
    arktest.assertEQ(JSON.stringify(emptyRec, null, '  '), '{}')
}

function jsonStringifyWithUndefinedReplacer(): void {
    arktest.assertEQ(JSON.stringify(new A(), undefined), '{"id":1,"name":"A"}')
    const rec: Record<string, Any> = { 'id': 1, 'name': 'R' }
    arktest.assertEQ(JSON.stringify(rec, undefined), '{"id":1,"name":"R"}')
}

function jsonStringifyWithJsonReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack('zhang san')
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: 'zhang'
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass)
    let result = '{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true}'
    arktest.assertEQ(jsonResult, result, '{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true}')
}

function jsonStringifyWithNestedReplacer(): void {
    let testClass: D<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new D<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack('zhang san')
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: 'zhang'
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass)
    let result = '{"value":{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true},"capacity":20}'
    arktest.assertEQ(jsonResult, result, '{"value":{"0":"zhang san","1":1,"2":{"age":10,"name":"zhang"},"4":null,"5":true},"capacity":20}')
}

function jsonStringifyWithSpaceReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack('zhang san')
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: 'zhang'
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass, undefined, ' ')
    let result = '{\n "0": "zhang san",\n "1": 1,\n "2": {\n  "age": 10,\n  "name": "zhang"\n },\n "4": null,\n "5": true\n}'
    arktest.assertEQ(jsonResult, result, '{\n "0": "zhang san",\n "1": 1,\n "2": {\n  "age": 10,\n  "name": "zhang"\n },\n "4": null,\n "5": true\n}')
}

function jsonStringifyWithReplacer(): void {
    let testClass: O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>
        = new O<number | string | Array<number | string | boolean> | boolean | undefined | null | Object>(20)
    testClass.pushBack('zhang san')
    testClass.pushBack(1)
    testClass.pushBack({
        age: 10, name: 'zhang'
    } as C)
    testClass.pushBack(undefined)
    testClass.pushBack(null)
    testClass.pushBack(true)
    let jsonResult = JSON.stringify(testClass, ['0'], ' ')
    let result = '{\n "0": "zhang san"\n}'
    arktest.assertEQ(jsonResult, result, '{\n "0": "zhang san"\n}')
}

function jsonStringifyStringArrayReplacer(): void {
    let arr = Array.of<TestClass1>(new TestClass1(), new TestClass1(), new TestClass1());
    let b = Array.of<string>('param1','param4')
    let ret = JSON.stringify(arr,b);
    let objArray : Array<TestClass1> = parseJsonString(ret)
    arktest.assertEQ(objArray[0].param1, '"test1"')
    arktest.assertEQ(objArray[0].param4, '"test4"')
    arktest.assertEQ(objArray[1].param1, '"test1"')
    arktest.assertEQ(objArray[1].param4, '"test4"')
    arktest.assertEQ(objArray[2].param1, '"test1"')
    arktest.assertEQ(objArray[2].param4, '"test4"')
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify')
    suite.addTest('object with function replacer', jsonStringifyObjectWithFunctionReplacer)
    suite.addTest('record with function replacer', jsonStringifyRecordWithFunctionReplacer)
    suite.addTest('array with function replacer', jsonStringifyArrayWithFunctionReplacer)
    suite.addTest('class implements JsonReplacer', jsonStringifyWithJsonReplacer)
    suite.addTest('class implements JsonReplacer nested case', jsonStringifyWithNestedReplacer)
    suite.addTest('class implements JsonReplacer with space', jsonStringifyWithSpaceReplacer)
    suite.addTest('class implements JsonReplacer with space and replacer', jsonStringifyWithReplacer)
    suite.addTest('class implements JsonReplacer with stringArray replacer', jsonStringifyStringArrayReplacer)
    suite.addTest('with array replacer', jsonStringifyWithArrayReplacer)
    suite.addTest('with "null" replacer', jsonStringifyWithNullReplacer)
    suite.addTest('with "undefined" replacer', jsonStringifyWithUndefinedReplacer)
    return suite.run()
}

class TestClass1 {
    public param1: string = 'test1';
    public param2: string = 'test2';
    public param3: string = 'test3';
    public param4: string = 'test4';
}

function parseJsonString(jsonStr: string): Array<TestClass1>  {
  let content: string = jsonStr.slice(1, -1)
  let objStrs: string[] = content.split('},{')
  let result: Array<TestClass1> = []
  objStrs.forEach((str: string, index: int, array: string[]) => {
    let objStr: string = str
    if (index === 0) {
      objStr = objStr + '}'
    }
    if (index === array.length - 1) {
      objStr = '{' + objStr;  
    }
    if (index !== 0 && index !== array.length - 1) {
      objStr = '{' + objStr + '}';  
    }
    let keyValueStr: string = objStr.slice(1, -1);
    let keyValuePairs: Array<string> = keyValueStr.split(',')
    let obj:TestClass1 = new TestClass1() ;
    keyValuePairs.forEach(pair => {
      let parts: Array<string> = pair.split(':')
      let keyPart: string = parts[0]
      let valuePart: string = parts[1]
      let key : string = keyPart.slice(1,-1)
      if (key === 'param1') {
        obj.param1 = valuePart
      } else if (key === 'param4') {
        obj.param4 = valuePart
      }
    });
    result.push(obj);
  });
  return result;
}

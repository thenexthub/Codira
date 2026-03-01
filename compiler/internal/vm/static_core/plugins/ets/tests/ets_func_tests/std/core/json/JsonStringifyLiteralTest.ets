/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

interface ObjectLiteralA {
    a: int;
    name: string;
    length: int;
    obj: ObjectLiteralB;
    obj1: ObjectLiteralD;
}

interface ObjectLiteralB {
    b: int;
    name: string;
    length: int;
    obj: ObjectLiteralC;
    obj1: ObjectLiteralD;
}

interface ObjectLiteralC {
    c: int;
    name: string;
    length: int;
    obj1: ObjectLiteralD;
}

interface ObjectLiteralD {
    d: int;
    name: string;
    length: int;
}

function stringifyFilterLiteralTest1() {
    let d: ObjectLiteralD = {d: 14, name: 'd', length: 4};
    let res = '{}';
    arktest.assertEQ(JSON.stringify(d, []), res);
    res = '{"d":14,"name":"d"}';
    arktest.assertEQ(JSON.stringify(d, ['d', 'name']), res);
}

function stringifyFilterLiteralTest2() {
    let c: ObjectLiteralC = {c: 13, name: 'c', length: 3, obj1: {d: 14, name: 'd', length: 4}};
    let res = '{"obj1":{}}';
    arktest.assertEQ(JSON.stringify(c, ['obj1']), res);
    res = '{"name":"c"}';
    arktest.assertEQ(JSON.stringify(c, ['name']), res);
    res = '{"name":"c","obj1":{"name":"d"}}';
    arktest.assertEQ(JSON.stringify(c, ['obj1', 'name']), res);
}

function stringifyFilterLiteralTest3() {
    let b: ObjectLiteralB = {
        b: 12,
        name: 'b',
        length: 2,
        obj: {
            c: 13,
            name: 'c',
            length: 3,
            obj1: {
                d: 34,
                name: 'd2',
                length: 44
            }
        },
        obj1: {
            d: 14,
            name: 'd1',
            length: 4
        }
    };
    let res = '{"obj":{}}';
    arktest.assertEQ(JSON.stringify(b, ['obj']), res);
    res = '{"obj1":{}}';
    arktest.assertEQ(JSON.stringify(b, ['obj1']), res);
    res = '{"obj":{"obj1":{}},"obj1":{}}';
    arktest.assertEQ(JSON.stringify(b, ['obj', 'obj1']), res);
    res = '{"obj":{"obj1":{"length":44},"length":3},"obj1":{"length":4},"length":2}';
    arktest.assertEQ(JSON.stringify(b, ['obj', 'obj1', 'length']), res);
    res = '{"obj":{"name":"c","obj1":{"name":"d2"}},"name":"b","obj1":{"name":"d1"}}';
    arktest.assertEQ(JSON.stringify(b, ['obj', 'obj1', 'name']), res);
}

function stringifyFilterLiteralTest4() {
    let a: ObjectLiteralA = {
        a: 11,
        name: 'a',
        length: 1,
        obj: {
            b: 12,
            name: 'b',
            length: 2,
            obj: {
                c: 13,
                name: 'c',
                length: 3,
                obj1: {
                    d: 34,
                    name: 'd2',
                    length: 44
                }
            },
            obj1: {
                d: 14,
                name: 'd1',
                length: 4
            }
        },
        obj1: {
            d: 24,
            name: 'd3',
            length: 24
        }
    };
    let res = '{"name":"a"}';
    arktest.assertEQ(JSON.stringify(a, ['name']), res);
    res = '{"obj":{"obj":{"name":"c"},"name":"b"},"name":"a"}';
    arktest.assertEQ(JSON.stringify(a, ['name', 'obj']), res);
    res = '{"name":"a","obj1":{"name":"d3"}}';
    arktest.assertEQ(JSON.stringify(a, ['name', 'obj1']), res);
    res = '{"obj":{"obj":{"name":"c","obj1":{"name":"d2"}},"name":"b","obj1":{"name":"d1"}},"name":"a","obj1":{"name":"d3"}}';
    arktest.assertEQ(JSON.stringify(a, ['name', 'obj', 'obj1']), res);

    res = '{}';
    arktest.assertEQ(JSON.stringify(a, ['d']), res);
    res = '{"obj":{"obj":{}}}';
    arktest.assertEQ(JSON.stringify(a, ['d', 'obj']), res);
    res = '{"obj1":{"d":24}}';
    arktest.assertEQ(JSON.stringify(a, ['d', 'obj1']), res);
    res = '{"obj":{"obj":{"obj1":{"d":34}},"obj1":{"d":14}},"obj1":{"d":24}}';
    arktest.assertEQ(JSON.stringify(a, ['d', 'obj', 'obj1']), res);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify');
    suite.addTest('for filter literal object 1', stringifyFilterLiteralTest1);
    suite.addTest('for filter literal object 2', stringifyFilterLiteralTest2);
    suite.addTest('for filter literal object 3', stringifyFilterLiteralTest3);
    suite.addTest('for filter literal object 4', stringifyFilterLiteralTest4);
    return suite.run();
}

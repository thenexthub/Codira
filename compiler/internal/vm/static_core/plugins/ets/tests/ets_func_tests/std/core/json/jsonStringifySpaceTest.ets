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

interface InterfaceA {
    name: string;
    length: int;
}

interface InterfaceB {
    name: string;
    length: int;
    obj?: ClassA;
}

class ClassA {
    public name: string;
    public num: number = 0;
    public length: int;
    public obj?: InterfaceA;
    public constructor(name: string, num: number, length: int) {
        this.name = name;
        this.num = num;
        this.length = length;
    }
}


function stringifySpaceTest() {
    let obj1 = new ClassA('name', 1, 2);
    let res = '{"name":"name","num":1,"length":2}';
    arktest.assertEQ(JSON.stringify(obj1), res);

    let obj2: InterfaceA = {name: 'a', length: 1};
    res = '{"name":"a","length":1}';
    arktest.assertEQ(JSON.stringify(obj2), res);
}

function stringifySpaceTest1() {
    let obj1 = new ClassA('name', 1, 2);
    let obj2: InterfaceA = {name: 'a', length: 1};
    let res = '{\n          "name": "name",\n          "num": 1,\n          "length": 2\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 10), res);
    arktest.assertEQ(JSON.stringify(obj1, null, 100), res);

    res = '{\n          "name": "a",\n          "length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 10), res);
    arktest.assertEQ(JSON.stringify(obj2, null, 100), res);

    res = '{\n     "name": "name",\n     "num": 1,\n     "length": 2\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 5), res);
    res = '{\n     "name": "a",\n     "length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 5), res);
}

function stringifySpaceTest2() {
    let obj1 = new ClassA('name', 1, 2);
    let obj2: InterfaceA = {name: 'a', length: 1};
    let res = '{\nabcdefghij"name": "name",\nabcdefghij"num": 1,\nabcdefghij"length": 2\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcdefghij'), res);
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcdefghijkkkkkkkk'), res);

    res = '{\nabcdefghij"name": "a",\nabcdefghij"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcdefghij'), res);
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcdefghijkkkkkkkk'), res);

    res = '{\nabcde"name": "name",\nabcde"num": 1,\nabcde"length": 2\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcde'), res);
    res = '{\nabcde"name": "a",\nabcde"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcde'), res);
}

function stringifySpaceTest3() {
    let obj1 = new ClassA('name', 1, 2);
    let obj2: InterfaceA = {name: 'a', length: 1};

    let res = '{\n^~!@#%&*$"name": "name",\n^~!@#%&*$"num": 1,\n^~!@#%&*$"length": 2\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, '^~!@#%&*$'), res);

    res = '{\n^~!@#%&*$"name": "a",\n^~!@#%&*$"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, '^~!@#%&*$'), res);
}

function stringifySpaceTest4() {
    let obj1 = new ClassA('name', 1, 2);
    obj1.obj = {name: 'a', length: 1};
    let obj2: InterfaceB = {name: 'a', length: 1};
    obj2.obj = new ClassA('new A', 3, 4);
    
    let res = '{\n^~!@#%&*$"name": "name",\n^~!@#%&*$"num": 1,\n^~!@#%&*$"length": 2,\n^~!@#%&*$"' +
              'obj": {\n^~!@#%&*$^~!@#%&*$"name": "a",\n^~!@#%&*$^~!@#%&*$"length": 1\n^~!@#%&*$}\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, '^~!@#%&*$'), res);

    res = '{\n^~!@#%&*$"obj": {\n^~!@#%&*$^~!@#%&*$"name": "new A",\n^~!@#%&*$^~!@#%&*$"num": 3,\n^' +
          '~!@#%&*$^~!@#%&*$"length": 4\n^~!@#%&*$},\n^~!@#%&*$"name": "a",\n^~!@#%&*$"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, '^~!@#%&*$'), res);
}

function stringifySpaceTest5() {
    let obj1 = new ClassA('name', 1, 2);
    obj1.obj = {name: 'a', length: 1};
    let obj2: InterfaceB = {name: 'a', length: 1};
    obj2.obj = new ClassA('new A', 3, 4);
    
    let res = '{\n          "name": "name",\n          "num": 1,\n          "length": 2,\n          "ob' +
              'j": {\n                    "name": "a",\n                    "length": 1\n          }\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 10), res);
    arktest.assertEQ(JSON.stringify(obj1, null, 100), res);

    res = '{\n          "obj": {\n                    "name": "new A",\n                    "num": 3,\n  ' +
          '                  "length": 4\n          },\n          "name": "a",\n          "length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 10), res);
    arktest.assertEQ(JSON.stringify(obj2, null, 100), res);

    res = '{\n     "name": "name",\n     "num": 1,\n     "length": 2,\n     "obj": {\n          "name": "a",\n          "length": 1\n     }\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 5), res);
    res = '{\n     "obj": {\n          "name": "new A",\n          "num": 3,\n          "length": 4\n     },\n     "name": "a",\n     "length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 5), res);
}

function stringifySpaceTest6() {
    let obj1 = new ClassA('name', 1, 2);
    obj1.obj = {name: 'a', length: 1};
    let obj2: InterfaceB = {name: 'a', length: 1};
    obj2.obj = new ClassA('new A', 3, 4);
    
    let res = '{\nabcdefghij"name": "name",\nabcdefghij"num": 1,\nabcdefghij"length": 2,\nabcdefghij"ob' +
              'j": {\nabcdefghijabcdefghij"name": "a",\nabcdefghijabcdefghij"length": 1\nabcdefghij}\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcdefghij'), res);
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcdefghijkkkkkkkk'), res);

    res = '{\nabcdefghij"obj": {\nabcdefghijabcdefghij"name": "new A",\nabcdefghijabcdefghij"num": 3,\nab' +
          'cdefghijabcdefghij"length": 4\nabcdefghij},\nabcdefghij"name": "a",\nabcdefghij"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcdefghij'), res);
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcdefghijkkkkkkkk'), res);

    res = '{\nabcde"name": "name",\nabcde"num": 1,\nabcde"length": 2,\nabcde"obj": {\nabcdeabcde"name": "a",\nabcdeabcde"length": 1\nabcde}\n}';
    arktest.assertEQ(JSON.stringify(obj1, null, 'abcde'), res);
    res = '{\nabcde"obj": {\nabcdeabcde"name": "new A",\nabcdeabcde"num": 3,\nabcdeabcde"length": 4\nabcde},\nabcde"name": "a",\nabcde"length": 1\n}';
    arktest.assertEQ(JSON.stringify(obj2, null, 'abcde'), res);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('JSON.stringify space tests');
    suite.addTest('for default', stringifySpaceTest);
    suite.addTest('for number', stringifySpaceTest1);
    suite.addTest('for string', stringifySpaceTest2);
    suite.addTest('for special string 1', stringifySpaceTest3);
    suite.addTest('for special string 2', stringifySpaceTest4);
    suite.addTest('for nested object 1', stringifySpaceTest5);
    suite.addTest('for nested object 2', stringifySpaceTest6);
    return suite.run();
}

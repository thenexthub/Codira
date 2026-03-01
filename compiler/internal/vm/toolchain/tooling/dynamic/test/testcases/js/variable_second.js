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

var o = {
    "foo" : function() {
        var number0 = 1;
        var string0 = "helloworld";
        var boolean0 = new Boolean(0);
        var obj0 = {
            "key0": "value0",
            "key1": 100
        };
        function function0() {
            var test = 0;
        }
        var map0 = new Map();
        var set0 = new Set();
        var undefined0 = undefined;
        let array0 = ['Apple', 'Banana']
        function* generator0() {
            let aa = 0;
            var a1 = 100;
            yield 1;
            yield 2;
            yield 3;
        }
        var regexp0 = /^\d+\.\d+$/i;
        var arraybuffer0 = new ArrayBuffer(24);
        var uint8array0 = new Uint8Array(arraybuffer0);
        const dataview0 = new DataView(arraybuffer0, 0);
        var bigint0 = BigInt(999n);
        var typedarray0 = new Uint8Array();
        var sharedarraybuffer0 = new SharedArrayBuffer(32);
        var iterator0 = string0[Symbol.iterator];
        var weakref0 = new WeakRef(obj0);

        var array1 = [];
        array1.push('banana', 'apple', 'peach');
        var array2 = ['banana', 'apple'];
        array2.push('peach');
        var array3 = ['banana', 'apple', 'peach'];
        array3.pop();
        var array4 = array3.shift();
        var array5 = new Array(number0);
        var array6 = new Array(string0);
        var array7 = new Array(boolean0);
        var array8 = new Array(obj0);
        var array9 = new Array(function0);
        var array10 = new Array(map0);
        var array11 = new Array(set0);
        var array12 = new Array(undefined0);
        var array13 = new Array(array0);
        var array14 = new Array(generator0);
        var array15 = new Array(regexp0);
        var array16 = new Array(arraybuffer0);
        var array17 = new Array(uint8array0);
        var array18 = new Array(dataview0);
        var array19 = new Array(bigint0);
        var array20 = array1;
        var array21 = array4;

        var typedarray1 = new Int8Array();
        var typedarray2 = new Uint8ClampedArray();
        var typedarray3 = new Int16Array();
        var typedarray4 = new Uint16Array();
        var typedarray5 = new Int32Array();
        var typedarray6 = new Uint32Array();
        var typedarray7 = new Float32Array();
        var typedarray8 = new Float64Array();
        var typedarray9 = new BigInt64Array();
        var typedarray10 = new BigUint64Array();
        var typedarray11 = new Uint8Array(number0);

        var iterator1 = array0[Symbol.iterator];
        var iterator2 = map0[Symbol.iterator];
        var iterator3 = set0[Symbol.iterator];
        var iterator4 = uint8array0[Symbol.iterator];
        var iterator5 = iterator0;
        var iterator6 = typedarray0[Symbol.iterator];
        var iterator7 = typedarray1[Symbol.iterator];
        var iterator8 = typedarray2[Symbol.iterator];
        var iterator9 = typedarray3[Symbol.iterator];
        var iterator10 = typedarray4[Symbol.iterator];
        var iterator11 = typedarray5[Symbol.iterator];
        var iterator12 = typedarray6[Symbol.iterator];
        var iterator13 = typedarray7[Symbol.iterator];
        var iterator14 = typedarray8[Symbol.iterator];
        var iterator15 = typedarray9[Symbol.iterator];
        var iterator16 = typedarray10[Symbol.iterator];
        var iterator17 = obj0[Symbol.iterator];
        var iterator19 = sharedarraybuffer0[Symbol.iterator];

        var weakMap0 = new WeakMap();
        var p1 = new Number(1);
        var p2 = new Number(2);
        var weakMap1 = new WeakMap([[p1, 'hello'], [p2, 'world']]);
        var weakMap2 = new WeakMap();
        weakMap2[0] = 'hello';
        var weakMap3 = new WeakMap();
        weakMap3.set(weakMap0, "weakMap0");
        var weakMap4 = new WeakMap();
        weakMap4.set(p1, 37);
        weakMap4.set(p2, 'azerty');
        var weakMap5 = new WeakMap();
        weakMap5.set(p1, undefined);

        var weakSet0 = new WeakSet();
        var weakSet1 = new WeakSet();
        weakSet1.add(p1);
        var weakSet2 = new WeakSet();
        weakSet2.add(p2);
        weakSet2.add(weakSet1);

        class Parent {
            name;
            age;
            constructor(name, age) {
                this.name = name;
                this.age = age;
            }
            print() {
                print(this.name + " " + this.age);
            }
        }

        class Child extends Parent {
            idNumber;
            constructor(name, age, idNumber) {
                super(name, age);
                this.idNumber = idNumber;
            }
            print() {
                print(this.name + " " + this.age + " " + this.idNumber);
            }
        }
        let parent = new Parent("parent", 50);
        let child = new Child("child", 15, "1234");
        parent.print();
        child.print();

        function customClass(a, b, child) {
            this._a = a;
            this._b = b;
            this.child = child;
            this.print = function() {
                print("customClass print is called");
            }
        }
        let class1 = new customClass(1, 2, child);
        class1.print();

        let class2 = {
            name: "class2"
        }
        print(class2.name);

        var nop = undefined;
    }
}

o.foo()
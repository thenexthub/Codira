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

        var set1 = new Set();
        set1.add(number0);
        var set2 = new Set(string0);
        var set3 = new Set();
        set3.add(obj0);
        var set4 = new Set(undefined0);
        var set5 = new Set(array0);
        var set6 = new Set();
        var set7 = new Set();
        var set8 = new Set();
        set8.add(generator0);
        var set9 = new Set();
        set9.add(regexp0);
        var set10 = new Set();
        set10.add(arraybuffer0);
        var set11 = new Set();
        set11.add(uint8array0);
        var set12 = new Set();
        set12.add(dataview0);
        var set13 = new Set();
        set13.add(function0);
        var set14 = set0;
        var set15 = new Set();
        set15.add(0);
        set15.add("hello");
        set15.add(obj0);
        set15.add(number0);
        var set16 = new Set();
        set16.add(map0);
        var set17 = new Set();
        set17.add(bigint0);
        var set18 = new Set();
        set18.add(boolean0);
        var set19 = new Set(set0);

        var number1 = 65535;
        var number2 = 5e-324;
        var number3 = 10 ** 10;
        var number4 = 0x1ffffffffff;
        var number5 = 0b11111111111111;
        var number6 = new Number(bigint0);
        var number7 = 123e45;
        var number8 = number0;
        var number9 = number0 + number1;
        var number10 = number0 - number1;
        var number11 = number0 * number1;
        var number12 = number0 / number1;
        var number13 = number0 % number1;
        var number14 = new Number(0);
        var number15 = new Number(1.7976931348623157e+308);
        var number16 = new Number(5e-324);
        var number17 = new Number(10 ** 10);
        var number18 = new Number(0x1ffffffffff);
        var number19 = new Number(0b11111111111111);
        var number20 = new Number(123e45);
        var number21 = new Number(number0);
        var number22 = new Number(number0 + number1);
        var number23 = new Number(number0 - number1);
        var number24 = new Number(number0 * number1);
        var number25 = new Number(number0 / number1);
        var number26 = new Number(number0 % number1);
        var number27 = 1.7976931348623157e+308;

        var string1 = "";
        var string2 = string1;
        var string3 = string1 + 'world';
        var string4 = 'hello' + 'world'
        var string5 = string4.charAt(1);
        var string6 = string1 + number0;
        var string7 = new String(string1);
        var string8 = new String(set0);
        var string9 = new String(number0);
        var string10 = new String(string0);
        var string11 = new String(obj0);
        var string12 = new String(undefined0);
        var string13 = new String(array0);
        var string14 = new String(bigint0);
        var string15 = new String(generator0);
        var string16 = new String(regexp0);
        var string17 = new String(arraybuffer0);
        var string18 = new String(uint8array0);
        var string19 = new String(dataview0);
        var string20 = new String(map0);
        var string21 = new String(function0);

        var bigint1 = BigInt(9007199254740991n);
        var bigint2 = BigInt(9007199254740991);
        var bigint3 = BigInt("9007199254740991");
        var bigint4 = BigInt("0x1fffffffffffff");
        var bigint5 = BigInt(0x1fffffffffffff);
        var bigint6 = BigInt("0b11111111111111111111111111111111111111111111111111111");
        var bigint7 = bigint0;
        var bigint8 = bigint0 + bigint1;
        var bigint9 = bigint0 - bigint1;
        var bigint10 = bigint0 * bigint1;
        var bigint11 = bigint0 / bigint1;
        var bigint12 = bigint0 % bigint1;
        var bigint13 = 10n ** 10n;
        var bigint14 = 888888888888888888888888888888888888888888888n;
        var bigint15 = BigInt(0b11111111111111);
        var bigint16 = BigInt(0);
        var bigint17 = 0n;
        var bigint18 = BigInt(123e45);
        var bigint19 = BigInt("01234567");
        var bigint20 = BigInt(number1);

        var boolean1 = new Boolean(1);
        var boolean2 = new Boolean(-1);
        var boolean3 = new Boolean(true);
        var boolean4 = new Boolean(false);
        var boolean5 = new Boolean(boolean0 + boolean1);
        var boolean6 = new Boolean(boolean0 && boolean1);
        var boolean7 = new Boolean(boolean0 || boolean1);
        var boolean8 = new Boolean(!boolean0);
        var boolean9 = true;
        var boolean10 = false;
        var boolean11 = boolean0;
        var boolean12 = boolean0 && boolean10;
        var boolean13 = boolean0 || boolean10;
        var boolean14 = !boolean10;
        var boolean15 = new Boolean("hello");
        var boolean16 = new Boolean("");
        var boolean17 = new Boolean(function0);
        var boolean18 = new Boolean(map0);
        var boolean19 = new Boolean(set0);
        var boolean20 = new Boolean(number0);
        var boolean21 = new Boolean(string0);
        var boolean22 = new Boolean(obj0);
        var boolean23 = new Boolean(undefined0);
        var boolean24 = new Boolean(array0);
        var boolean25 = new Boolean(generator0);
        var boolean26 = new Boolean(regexp0);
        var boolean27 = new Boolean(arraybuffer0);
        var boolean28 = new Boolean(uint8array0);
        var boolean29 = new Boolean(dataview0);

        var map1  = new Map(map0);
        var map2 = new Map([
            [1, 'hello'],
            [2, 'world']
        ]);
        var map3  = new Map();
        map3.set(NaN, 'NaN');
        var map4 = new Map();
        map4[0] = 'hello';
        var map5  = new Map();
        map5.set(0, 'zero');
        map5.set(1, 'one');
        map5.set(2, 'two');
        map5.set(3, 'three');
        var map6  = new Map();
        map6.set(set0, "set0");
        var map7  = new Map();
        map7.set(number0, "number0");
        var map8  = new Map();
        map8.set(string0, "string0");
        var map9  = new Map();
        map9.set(obj0, "object0");
        var map10 = new Map();
        map10.set(undefined0, "undefined0");
        var map11 = new Map();
        map11.set(array0, "array0");
        var map12 = new Map();
        map12.set(map3, "map3");
        var map13 = new Map();
        map13.set(generator0, "generator0");
        var map14 = new Map();
        map14.set(regexp0, "regexp0");
        var map15 = new Map();
        map15.set(arraybuffer0, "arraybuffer0");
        var map16 = new Map();
        map16.set(uint8array0, "uint8array0");
        var map17 = new Map();
        map17.set(dataview0, "dataview0");
        var map18 = new Map();
        map18.set(bigint10, "bigint10");
        var map19 = new Map();
        map19.set(function0, "function0");

        var object1 = {0 :'zero', 1 : 'one', 2 : 'two', 3 : 'three', 4 : 'four', 5 : 'five'};
        var object2 = object1;
        var object3 = new Object({0 :'zero', 1 : 'one', 2 : 'two', 3 : 'three', 4 : 'four', 5 : 'five'});
        var object4 = new Object(set0);
        var object5 = new Object(string0);
        var object6 = new Object(map0);
        var object7 = new Object(number0);
        var object8 = new Object(obj0);
        var object9 = new Object(undefined0);
        var object10 = new Object(array0);
        var object11 = new Object(bigint10);
        var object12 = new Object(generator0);
        var object13 = new Object(regexp0);
        var object14 = new Object(bigint0);
        var object15 = new Object(arraybuffer0);
        var object16 = new Object(uint8array0);
        var object17 = new Object(dataview0);
        var object18 = new Object(boolean10);
        var object19 = new Object(function0);

        var regExp1 = /^a/g;
        var regExp2 = new RegExp('^ab+c', 'g');
        var regExp3 = /123$/;
        var regExp4 = /\d/i;
        var regExp5 = new RegExp('^[a-zA-Z]/w{5,17}$', 'iu');
        var regExp6 = /[A-Z]/m;
        var regExp7 = new RegExp('(/d{3}-|/d{4}-)?(/d{8}|/d{7})?', 'mg');
        var regExp8 = /[a-z]/y;
        var regExp9 = /\s/u;
        var regExp10 = /a+/s;
        var regExp11 = new RegExp('(d+-)?(d{4}-?d{7}|d{3}-?d{8}|^d{7,8})(-d+)?', 's');
        var regExp12 = /a?/gy;
        var regExp13 = new RegExp('/^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$/');
        var regExp14 = /a*/igmy;
        var regExp15 = new RegExp('^[/w-]+(/.[/w-]+)*@[/w-]+(/.[/w-]+)+$', 'ig');
        var regExp16 = /a|b/iymsg;
        var regExp17 = new RegExp('^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$');
        var regExp18 = /\/<(.*)>.*<\/\/\/1>|<(.*) \/\/>/i;
        var regExp19 = new RegExp('^[1-9]*[1-9][0-9]*$', 'm');
        var regExp20 = /^[a-zA-Z]\/w{5,17}$/;
        var regExp21 = new RegExp('^[0-9a-zA-Z_]{1,}$', 'u');

        var target = {
            name: "openharmony",
            age: 3
        };
        var handler = {
            get(target, key) {
                let result = target[key];
                if (key === "age") result += "岁";
                return result;
            },
            set(target, key, value) {
                if (key === "age" && typeof value !== "number") {
                    throw Error("age字段必须为Number类型");
                }
                return Reflect.set(target, key, value);
            }
        }
        var {proxy, revoke} = Proxy.revocable(target, handler);
        var proxy1 = new Proxy(target, handler);
        var proxy2 = Proxy.revocable(target, handler);

        revoke();
        proxy2.revoke();
        
        var promiseFulfilled = new Promise((resolve, reject) => {
            let result = {
                flag: true
            }
            if (result.flag == true) {
                resolve(result);
            } else {
                reject(new Error("The promise is rejected"));
            }
        });
        var string22 = "\uD83D";
        var string23 = string22 + "\uDE42";
        var string24 = "\u{1F468}";
        var string25 = string24 + "\u200D\u{1F468}\u200D\u{1F467}\u200D\u{1F466}\u200D";
        print(string22);
        print(string23);
        print(string24);
        print(string25);

        function resolveHandler(result) {
            let a = promiseFulfilled;
            print("promise is fulfilled with result.flag " + result.flag);
        }

        function rejectHandler(error) {
            print(error.message);
        }

        promiseFulfilled.then(result => resolveHandler(result)).catch(error => rejectHandler(error));

        var nop = undefined;
    }
}

o.foo()
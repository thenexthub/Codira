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

import * as obj from "./export_variable_first.js";
import {number6} from "./export_variable_second.js";
import {string6} from "./export_variable_second.js";
import {boolean6} from "./export_variable_second.js";
import {obj6} from "./export_variable_second.js";
import {function6} from "./export_variable_second.js";
import {map6} from "./export_variable_second.js";
import {set6} from "./export_variable_second.js";
import {undefined6} from "./export_variable_second.js";
import {array6} from "./export_variable_second.js";
import {generator6} from "./export_variable_second.js";
import {regexp6} from "./export_variable_second.js";
import {arraybuffer6} from "./export_variable_second.js";
import {uint8array6} from "./export_variable_second.js";
import {dataview6} from "./export_variable_second.js";
import {bigint6} from "./export_variable_second.js";

import {number9} from "./export_variable_second.js";
import {string9} from "./export_variable_second.js";
import {boolean9} from "./export_variable_second.js";
import {obj9} from "./export_variable_second.js";
import {function9} from "./export_variable_second.js";
import {map9} from "./export_variable_second.js";
import {set9} from "./export_variable_second.js";
import {undefined9} from "./export_variable_second.js";
import {array9} from "./export_variable_second.js";
import {generator9} from "./export_variable_second.js";
import {regexp9} from "./export_variable_second.js";
import {arraybuffer9} from "./export_variable_second.js";
import {uint8array9} from "./export_variable_second.js";
import {dataview9} from "./export_variable_second.js";
import {bigint9} from "./export_variable_second.js";

// Transit introduction
import {transit1} from "./export_variable_first.js";
import {transit2} from "./export_variable_first.js";
import {transit4} from "./export_variable_first.js";

import {transit1 as transit5} from "./export_variable_first.js";
import {transit2 as transit6} from "./export_variable_first.js";
import {transit4 as transit7} from "./export_variable_first.js";

// Alias import
import {number6 as number7} from "./export_variable_second.js";
import {string6 as string7} from "./export_variable_second.js";
import {boolean6 as boolean7} from "./export_variable_second.js";
import {obj6 as obj7} from "./export_variable_second.js";
import {function6 as function7} from "./export_variable_second.js";
import {map6 as map7} from "./export_variable_second.js";
import {set6 as set7} from "./export_variable_second.js";
import {undefined6 as undefined7} from "./export_variable_second.js";
import {array6 as array7}from "./export_variable_second.js";
import {generator6 as generator7} from "./export_variable_second.js";
import {regexp6 as regexp7} from "./export_variable_second.js";
import {arraybuffer6 as arraybuffer7} from "./export_variable_second.js";
import {uint8array6 as uint8array7} from "./export_variable_second.js";
import {dataview6 as dataview7} from "./export_variable_second.js";
import {bigint6 as bigint7} from "./export_variable_second.js";

import {number9 as number10} from "./export_variable_second.js";
import {string9 as string10} from "./export_variable_second.js";
import {boolean9 as boolean10} from "./export_variable_second.js";
import {obj9 as obj10} from "./export_variable_second.js";
import {function9 as function10} from "./export_variable_second.js";
import {map9 as map10} from "./export_variable_second.js";
import {set9 as set10} from "./export_variable_second.js";
import {undefined9 as undefined10} from "./export_variable_second.js";
import {array9 as array10} from "./export_variable_second.js";
import {generator9 as generator10} from "./export_variable_second.js";
import {regexp9 as regexp10} from "./export_variable_second.js";
import {arraybuffer9 as arraybuffer10} from "./export_variable_second.js";
import {uint8array9 as uint8array10} from "./export_variable_second.js";
import {dataview9 as dataview10} from "./export_variable_second.js";
import {bigint9 as bigint10} from "./export_variable_second.js";


export var number0 = 1;
export var string0 = "helloworld";
export var boolean0 = new Boolean(0);
export var obj0 = {
	"key0": "value0",
	"key1": 100
};
export function function0() {
	var test = 0;
}
export var map0 = new Map();
export var set0 = new Set();
export var undefined0 = undefined;
export let array0 = ['Apple', 'Banana']
export function* generator0() {
	let aa = 0;
	var a1 = 100;
	yield 1;
	yield 2;
	yield 3;
}
export var regexp0 = /^\d+\.\d+$/i;
export var arraybuffer0 = new ArrayBuffer(24);
export var uint8array0 = new Uint8Array(arraybuffer0);
export const dataview0 = new DataView(arraybuffer0, 0);
export var bigint0 = BigInt(999n);
export var typedarray0 = new Uint8Array();
export var sharedarraybuffer0 = new SharedArrayBuffer(32);
export var weakref0 = new WeakRef(obj0);
export var iterator0 = string0[Symbol.iterator];
export default function(){};

var number1 = 11;
var string1 = "helloworld1";
var boolean1 = new Boolean(0);
var obj1 = {
    "key2": "value1",
    "key3": 1000
};
function function1() {
    var test = 0;
}
var map1 = new Map();
var set1 = new Set();
var undefined1 = undefined;
let array1 = ['Cherry', 'Pear']
function* generator1() {
    let bb = 0;
    var b1 = 100;
    yield 4;
    yield 5;
    yield 6;
}
var regexp1 = /^\d+\.\d+$/i;
var arraybuffer1 = new ArrayBuffer(24);
var uint8array1 = new Uint8Array(arraybuffer0);
const dataview1 = new DataView(arraybuffer0, 0);
var bigint1 = BigInt(9999n);
var typedarray1 = new Uint8Array();
var sharedarraybuffer1 = new SharedArrayBuffer(32);
var iterator1 = string1[Symbol.iterator];
var weakref1 = new WeakRef(obj1);

export {number1};
export {string1};
export {boolean1};
export {obj1};
export {function1};
export {map1};
export {set1};
export {undefined1};
export {generator1};
export {regexp1};
export {arraybuffer1};
export {uint8array1};
export {dataview1};
export {bigint1};
export {typedarray1};
export {sharedarraybuffer1};
export {iterator1};
export {weakref1};

var number2 = 111;
var string2 = "helloworld11";
var boolean2 = new Boolean(0);
var obj2 = {
    "key4": "value2",
    "key5": 10000
};
function function2() {
    var test = 0;
}
var map2 = new Map();
var set2 = new Set();
var undefined2 = undefined;
let array2 = ['Lemon', 'Durian']
function* generator2() {
    let cc = 0;
    var c1 = 1000;
    yield 7;
    yield 8;
    yield 9;
}
var regexp2 = /^\d+\.\d+$/i;
var arraybuffer2 = new ArrayBuffer(24);
var uint8array2 = new Uint8Array(arraybuffer0);
const dataview2 = new DataView(arraybuffer0, 0);
var bigint2 = BigInt(9999n);
var typedarray2 = new Uint8Array();
var sharedarraybuffer2 = new SharedArrayBuffer(32);
var iterator2 = string2[Symbol.iterator];
var weakref2 = new WeakRef(obj2);

export {number2 as number3};
export {string2 as string3};
export {boolean2 as boolean3};
export {obj2 as obj3};
export {function2 as function3};
export {map2 as map3};
export {set2 as set3};
export {undefined2 as undefined3};
export {generator2 as generator3};
export {regexp2 as regexp3};
export {arraybuffer2 as arraybuffer3};
export {uint8array2 as uint8array3};
export {dataview2 as dataview3};
export {bigint2 as bigint3};
export {typedarray2 as typedarray3};
export {sharedarraybuffer2 as sharedarraybuffer3};
export {iterator2 as iterator3};
export {weakref2 as weakref3};

export {number4} from "./export_variable_first.js";
export {string4} from "./export_variable_first.js";
export {boolean4} from "./export_variable_first.js";

export {number4 as number5} from "./export_variable_first.js";
export {string4 as string5} from "./export_variable_first.js";
export {boolean4 as boolean5} from "./export_variable_first.js";

var multipleOut1 = 1;
var multipleOut2 = 2;
var multipleOut3 = 3;
var multipleOut4 = 4;
var multipleOut5 = 5;
var multipleOut6 = 6;
var multipleOut10 = 10;
var multipleOut11 = 11;
var multipleOut12 = 12;
// Multiple outputs
export {multipleOut1, multipleOut2, multipleOut3};
export {multipleOut4 as multipleOut7, multipleOut5 as multipleOut8, multipleOut6 as multipleOut9};
export {multipleOut10, multipleOut11 as multipleOut13, multipleOut12};

export function foo() {
    var num1 = number0;
    var num2 = string0;
    var num3 = boolean0;
    var num4 = obj0;
    var num5 = obj;
    var num6 = number6;
    var num7 = string6;
    var num8 = boolean6;
    var num9 = obj6;
    var num10 = function6;
    var num11 = map6;
    var num12 = set6;
    var num13 = undefined6;
    var num14 = array6;
    var num15 = generator6;
    var num16 = regexp6;
    var num17 = arraybuffer6;
    var num18 = uint8array6;
    var num19 = dataview6;
    var num20 = bigint6;
    var num21 = number7;
    var num22 = string7;
    var num23 = boolean7;
    var num24 = obj7;
    var num25 = function7;
    var num26 = map7;
    var num27 = set7;
    var num28 = undefined7;
    var num29 = array7;
    var num30 = generator7;
    var num31 = regexp7;
    var num32 = arraybuffer7;
    var num33 = uint8array7;
    var num34 = dataview7;
    var num35 = bigint7;
    var num36 = number9;
    var num37 = string9;
    var num38 = boolean9;
    var num39 = obj9;
    var num40 = function9;
    var num41 = map9;
    var num42 = set9;
    var num43 = undefined9;
    var num44 = array9;
    var num45 = generator9;
    var num46 = regexp9;
    var num47 = arraybuffer9;
    var num48 = uint8array9;
    var num49 = dataview9;
    var num50 = bigint9;
    var num51 = number10;
    var num52 = string10;
    var num53 = boolean10;
    var num54 = obj10;
    var num55 = function10;
    var num56 = map10;
    var num57 = set10;
    var num58 = undefined10;
    var num59 = array10;
    var num60 = generator10;
    var num61 = regexp10;
    var num62 = arraybuffer10;
    var num63 = uint8array10;
    var num64 = dataview10;
    var num65 = bigint10;
    var num66 = transit1;
    var num67 = transit2;
    var num68 = transit4;
    var num69 = transit5;
    var num70 = transit6;
    var num71 = transit7;
    var num72 = obj.number3;
}

foo();
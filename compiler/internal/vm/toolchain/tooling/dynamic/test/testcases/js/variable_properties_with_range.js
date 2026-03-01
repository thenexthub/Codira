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
    "testContainers": function() {
        // ArrayList
        var ArrayList = ArkPrivate.Load(ArkPrivate.ArrayList);
        let arrayList = new ArrayList();
        for (let i = 0; i < 150; i++) {
            arrayList.add(i);
        }
        // Deque
        var Deque = ArkPrivate.Load(ArkPrivate.Deque);
        let deque = new Deque();
        for (let i = 0; i < 150; i++) {
            deque.insertEnd(i);
        }
        // HashMap
        var HashMap = ArkPrivate.Load(ArkPrivate.HashMap);
        let hashMap = new HashMap();
        for (let i = 0; i < 150; i++) {
            hashMap.set(i+"", i);
        }
        // HashSet
        var HashSet = ArkPrivate.Load(ArkPrivate.HashSet);
        let hashSet = new HashSet();
        for (let i = 0; i < 150; i++) {
            hashSet.add(i);
        }
        // LightWeightMap
        var LightWeightMap = ArkPrivate.Load(ArkPrivate.LightWeightMap);
        let lightWeightMap = new LightWeightMap();
        for (let i = 0; i < 150; i++) {
            lightWeightMap.set(i+"", i);
        }
        // LightWeightSet
        var LightWeightSet = ArkPrivate.Load(ArkPrivate.LightWeightSet);
        let lightWeightSet = new LightWeightSet();
        for (let i = 0; i < 150; i++) {
            lightWeightSet.add(i);
        }
        // LinkedList
        var LinkedList = ArkPrivate.Load(ArkPrivate.LinkedList);
        let linkedList = new LinkedList();
        for (let i = 0; i < 150; i++) {
            linkedList.add(i);
        }
        // List
        var List = ArkPrivate.Load(ArkPrivate.List);
        let list = new List();
        for (let i = 0; i < 150; i++) {
            list.add(i);
        }
        // PlainArray
        var PlainArray = ArkPrivate.Load(ArkPrivate.PlainArray);
        let plainArray = new PlainArray();
        for (let i = 0; i < 150; i++) {
            plainArray.add(i, i + "");
        }
        // Queue
        var Queue = ArkPrivate.Load(ArkPrivate.Queue);
        let queue = new Queue();
        for (let i = 0; i < 150; i++) {
            queue.add(i);
        }
        // Stack
        var Stack = ArkPrivate.Load(ArkPrivate.Stack);
        let stack = new Stack();
        for (let i = 0; i < 150; i++) {
            stack.push(i);
        }
        // TreeMap
        var TreeMap = ArkPrivate.Load(ArkPrivate.TreeMap);
        let treeMap = new TreeMap();
        for (let i = 0; i < 150; i++) {
            treeMap.set(""+i, i);
        }
        // TreeSet
        var TreeSet = ArkPrivate.Load(ArkPrivate.TreeSet);
        let treeSet = new TreeSet();
        for (let i = 0; i < 150; i++) {
            treeSet.add(i);
        }
        // Vector
        var Vector = ArkPrivate.Load(ArkPrivate.Vector);
        let vector = new Vector();
        for (let i = 0; i < 150; i++) {
            vector.add(i);
        }
        // SendableMap
        let sendableMap = new SendableMap();
        for (let i = 0; i < 150; i++) {
            sendableMap.set(i+"", i);
        }
        // Map
        let map = new Map();
        for (let i = 0; i < 150; i++) {
            map.set(i+"", i);
        }
        // WeakMap
        let weakMap = new WeakMap();
        for (let i = 0; i < 150; i++) {
            weakMap.set(new Object(i), i+"");
        }
        // SendableSet
        let sendableSet = new SendableSet();
        for (let i = 0; i < 150; i++) {
            sendableSet.add(i);
        }
        // Set
        let set = new Set();
        for (let i = 0; i < 150; i++) {
            set.add(i);
        }
        // WeakSet
        let weakSet = new WeakSet();
        for (let i = 0; i < 150; i++) {
            weakSet.add(new Object(i));
        }
        // SendableArray
        let sendableArray = new SendableArray();
        for (let i = 0; i < 150; i++) {
            sendableArray.push(i);
        }
 
        var nop = undefined;
    },
 
    "testArrays": function() {
        // Array
        let array = new Array();
        for (let i = 0; i < 150; i++) {
            array[i] = i;
        }
        // Int8Array
        let int8Array = new Int8Array(150);
        for (let i = 0; i < 150; i++) {
            int8Array[i] = i;
        }
        // Uint8Array
        let uint8Array = new Uint8Array(150);
        for (let i = 0; i < 150; i++) {
            uint8Array[i] = i;
        }
        // Uint8ClampedArray
        let uint8ClampedArray = new Uint8ClampedArray(150);
        for (let i = 0; i < 150; i++) {
            uint8ClampedArray[i] = i;
        }
        // Int16Array
        let int16Array = new Int16Array(150);
        for (let i = 0; i < 150; i++) {
            int16Array[i] = i;
        }
        // Uint16Array
        let uint16Array = new Uint16Array(150);
        for (let i = 0; i < 150; i++) {
            uint16Array[i] = i;
        }
        // Int32Array
        let int32Array = new Int32Array(150);
        for (let i = 0; i < 150; i++) {
            int32Array[i] = i;
        }
        // Uint32Array
        let uint32Array = new Uint32Array(150);
        for (let i = 0; i < 150; i++) {
            uint32Array[i] = i;
        }
        // BigInt64Array
        let bigInt64Array = new BigInt64Array(150);
        for (let i = 0; i < 150; i++) {
            bigInt64Array[i] = 10000000n;
        }
        // BigUint64Array
        let bigUInt64Array = new BigUint64Array(150);
        for (let i = 0; i < 150; i++) {
            bigUInt64Array[i] = 20000000n;
        }
        // Float32Array
        let float32Array = new Float32Array(150);
        for (let i = 0; i < 150; i++) {
            float32Array[i] = 1.1;
        }
        // Float64Array
        let float64Array = new Float64Array(150);
        for (let i = 0; i < 150; i++) {
            float64Array[i] = 1.1;
        }
 
        var nop = undefined;
    }
}
 
o.testContainers()
o.testArrays()
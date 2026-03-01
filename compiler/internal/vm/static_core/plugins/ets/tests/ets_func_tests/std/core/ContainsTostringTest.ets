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


function testMap() {
    let map = new Map<string,int>();
    arktest.assertEQ(map.toString(),"[object Map]");
    map.set("1",2);
    map.set("3",4);
    arktest.assertEQ(map.toString(),"1,2,3,4");
}
  
function testSet() {
    let set = new Set<int>();
    arktest.assertEQ(set.toString(),"[object Set]");
    set.add(1).add(2)
    arktest.assertEQ(set.toString(),"Set[1,2]");
}

function testArrayBuffer() {
  const toStringBuffer = new ArrayBuffer(8);
  arktest.assertEQ(toStringBuffer.toString(),"[object ArrayBuffer]");
  const toStringSliced = toStringBuffer.slice(2, 6);
  arktest.assertEQ(toStringSliced.toString(),"[object ArrayBuffer]");
  toStringSliced.set(1,2)
  arktest.assertEQ(toStringSliced.toString(),"[object ArrayBuffer]");
}

function testWeapMap() {
  let weakMap = new WeakMap<Object,int>();
  arktest.assertEQ(weakMap.toString(),"[object WeakMap]");
  weakMap.set(new WeakMap<Object,int>(),1);
  weakMap.set(new WeakMap<Object,int>(),2);
  arktest.assertEQ(weakMap.toString(),"[object WeakMap]");
}

function testWeapSet() {
  let weakSet = new WeakSet<Object>();
  arktest.assertEQ(weakSet.toString(),"[object WeakSet]");
  weakSet.add(new WeakSet<Object>()).add(new WeakSet<Object>());
  arktest.assertEQ(weakSet.toString(),"[object WeakSet]");
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Contains toString Test");

    suite.addTest("Map toString test", testMap);
    suite.addTest("Set toString test", testSet);
    suite.addTest("ArrayBuffer toString test", testArrayBuffer);
    suite.addTest("WeapMap toString test", testWeapMap);
    suite.addTest("WeapSet toString test", testWeapSet);
    return suite.run();
}
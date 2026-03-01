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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil types.isSetIterator() API tests")

    // test set
    suite.addTest("Verify Set.keys() is identified as SetIterator", testSetKeysIsSetIterator)
    suite.addTest("Verify Set.values() is identified as SetIterator", testSetValuesIsSetIterator)
    suite.addTest("Verify Set.entries() is identified as SetIterator", testSetEntriesIsSetIterator)
    suite.addTest("Verify Set.$_iterator() is identified as SetIterator", testSetIteratorIsSetIterator)
    suite.addTest("Verify Set is not identified as SetIterator", testSetIsNotSetIterator)

    // test map
    suite.addTest("Verify Map.keys() is not identified as SetIterator", testMapKeysIsNotSetIterator)
    suite.addTest("Verify Map.values() is not identified as SetIterator", testMapValuesIsNotSetIterator)
    suite.addTest("Verify Map.entries() is not identified as SetIterator", testMapEntriesIsNotSetIterator)
    suite.addTest("Verify Map.$_iterator() is not identified as SetIterator", testMapIteratorIsNotSetIterator)
    suite.addTest("Verify Map is not identified as SetIterator", testMapIsNotSetIterator)


    return suite.run()
}

function testSetKeysIsSetIterator() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.keys());
  arktest.assertTrue(result, "set.keys() should be SetIterator type");
}

function testSetValuesIsSetIterator() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.values());
  arktest.assertTrue(result, "set.values() should be SetIterator type");
}

function testSetEntriesIsSetIterator() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.entries());
  arktest.assertTrue(result, "set.entries() should be SetIterator type");
}

function testSetIteratorIsSetIterator() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set.$_iterator());
  arktest.assertTrue(result, "set.$_iterator() should be SetIterator type");
}

function testSetIsNotSetIterator() {
  let proc = new util.types();
  const set = new Set<int>();
  let result = proc.isSetIterator(set);
  arktest.assertFalse(result, "set shouldn't be SetIterator type");
}

function testMapIsNotSetIterator() {
  let proc = new util.types();
  const map = new Map<int, string>();
  let result = proc.isSetIterator(map);
  arktest.assertFalse(result, "map shouldn't be SetIterator type");
}

function testMapKeysIsNotSetIterator() {
  let proc = new util.types();
  const map = new Map<int, string>();
  let result = proc.isSetIterator(map.keys());
  arktest.assertFalse(result, "map.keys() shouldn't be SetIterator type");
}

function testMapValuesIsNotSetIterator() {
  let proc = new util.types();
  const map = new Map<int, string>();
  let result = proc.isSetIterator(map.values());
  arktest.assertFalse(result, "map.values() shouldn't be SetIterator type");
}

function testMapEntriesIsNotSetIterator() {
  let proc = new util.types();
  const map = new Map<int, string>();
  let result = proc.isSetIterator(map.entries());
  arktest.assertFalse(result, "map.entries() shouldn't be SetIterator type");
}

function testMapIteratorIsNotSetIterator() {
  let proc = new util.types();
  const map = new Map<int, string>();
  let result = proc.isSetIterator(map.$_iterator());
  arktest.assertFalse(result, "map.$_iterator() shouldn't be SetIterator type");
}

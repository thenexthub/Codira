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

function getMap(): Map<int, int> {
  let map: Map<int, int> = new Map<int, int>()
  for (let i: int = 0; i < 5; i++) {
    map.set(i, i)
  }
  return map;
}

function setItersCounter(bucket: FixedArray<int>, cnt: int): boolean {
    bucket[bucket.length - 1] = cnt
    return true
}

function checkItersCounter(bucket: FixedArray<int>, cnt: int): boolean {
    if (bucket[bucket.length - 1] == cnt) {
        return true
    }
    return false
}

function checkItersCounterClear(map: Map<int, int>): boolean {
    const iter = map.keys();
    map.clear()
    if (iter.next().value == 0) {
        return false
    }
    return true
}

function checkItersCounterRehash(map: Map<int, int>, size: int): boolean {
    const iter = map.keys();
    // used 'size*2' to ensure that rehash is triggered
    for (let i: int = 5; i < (size * 2); i++) {
        map.set(i, i)
    }
    map.delete(0)
    if (iter.next().value == 0) {
        return false
    }
    return true
}

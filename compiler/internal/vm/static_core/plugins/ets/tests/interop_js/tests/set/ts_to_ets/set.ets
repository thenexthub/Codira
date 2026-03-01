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

package set.test;

export function TestSetAdd(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(4);
    set.add(5);
    res = set.has(1) && set.has(2) && set.has(3) &&
          set.has(4) && set.has(5) && (set.size === 5);
    return res;
}

export function TestSetHas(set: Set<number>): boolean {
    let res: boolean = true;
    res = set.has(1) && set.has(2) && set.has(3) &&
          set.has(4) && set.has(5) && !set.has(999);
    return res;
}

export function TestSetDelete(set: Set<number>): boolean {
    let res: boolean = true;
    set.delete(4);
    res = !set.has(4);
    return res;
}

export function TestSetSize(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    res = (set.size === 5);
    return res;
}

export function TestSetKeys(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(5);
    let keys: number[] = [];
    let ss: Set<number> = set as Set<number>;
    for (let key of ss.keys()) {
        keys.push(key);
    }
    res = keys.includes(1) && keys.includes(2) &&
          keys.includes(3) && keys.includes(4) &&
          keys.includes(5) && keys.length === 5;
    return res;
}

export function TestSetValues(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(5);
    let values: number[] = [];
    let ss: Set<number> = set as Set<number>;
    for (let value of ss.values()) {
        values.push(value);
    }
    res = values.includes(1) && values.includes(2) &&
          values.includes(3) && values.includes(4) &&
          values.includes(5) && values.length === 5;
    return res;
}

export function TestSetEntries(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(5);
    for (let item of set.entries()) {
        if (item[0] != item[1]) {
            res = false;
            break;
        }
    }
    return res;
}

export function TestSetForEach(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(5);
    let items: number[] = [];
    const processElement = (value: number, valueAgain: number, setObj: Set<number>): void => {
        items.push(value);
    };
    set.forEach(processElement);
    res = items.includes(1) && items.includes(2) &&
          items.includes(3) && items.includes(4) &&
          items.includes(5) && items.length === 5;
    return res;
}

export function TestSetClear(set: Set<number>): boolean {
    let res: boolean = true;
    set.add(4);
    set.add(5);
    set.clear();
    res = (set.size === 0);
    return res;
}
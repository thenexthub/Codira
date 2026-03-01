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

package map.test;

export function TestMapSet(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.set('cherry', 3);
    res = (map.get('apple') === 1) && (map.get('banana') === 2) &&
          (map.get('cherry') === 3);
    return res;
}

export function TestMapGet(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.set('cherry', 3);
    res = (map.get('apple') === 1) && (map.get('banana') === 2) &&
          (map.get('cherry') === 3) && (map.get('unknown') === undefined);
    return res;
}

export function TestMapHas(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.set('cherry', 3);
    res = map.has('apple') && map.has('banana') &&
          map.has('cherry') && !map.has('unknown');
    return res;
}

export function TestMapDelete(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.delete('banana')
    res = !map.has('banana');
    return res;
}

export function TestMapSize(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    res = map.size === 3;
    return res;
}

export function TestMapKeys(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('cherry', 3);
    map.set('banana', 2);
    let keys: string[] = [];
    let mm: Map<string, number> = map as Map<string, number>;
    for (let key of mm.keys()) {
        keys.push(key);
    }
    res = keys[0] === 'apple' && keys[1] === 'cherry' &&
          keys[2] === 'banana' && keys.length === 3;
    return res;
}

export function TestMapValues(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('cherry', 3);
    map.set('banana', 2);
    let values: number[] = [];
    let mm: Map<string, number> = map as Map<string, number>;
    for (let value of mm.values()) {
        values.push(value);
    }
    res = values[0] === 1 && values[1] === 3 &&
          values[2] === 2 && values.length === 3;
    return res;
}

export function TestMapEntries(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.set('cherry', 3);
    for (let item of map.entries()) {
        if (map.get(item[0]) != item[1]) {
            res = false;
            break;
        }
    }
    return res;
}

export function TestMapForEach(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('cherry', 3);
    map.set('banana', 2);
    let keys: string[] = [];
    let values: number[] = [];
    const processElement = (val: number, key: string, map: Map<string, number>): void => {
        keys.push(key);
        values.push(val);
    };
    map.forEach(processElement);
    res = keys[0] === 'apple' && keys[1] === 'cherry' && keys[2] === 'banana' && keys.length === 3 &&
          values[0] === 1 && values[1] === 3 && values[2] === 2 && values.length === 3;
    return res;
}

export function TestMapClear(map: Map<string, number>): boolean {
    let res: boolean = true;
    map.set('banana', 2);
    map.set('cherry', 3);
    map.clear();
    res = (map.size === 0);
    return res;
}
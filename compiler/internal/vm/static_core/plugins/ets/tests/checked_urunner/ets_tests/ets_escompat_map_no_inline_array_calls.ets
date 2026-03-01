/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER     Verify that array method calls remain non-inlined in a custom class extending Map.
//! RUN         force_jit: true, options: "--compiler-regex='.*test.*'", entry: "ets_escompat_map_no_inline_array_calls.ETSGLOBAL::test"
//! METHOD      "ets_escompat_map_no_inline_array_calls.ETSGLOBAL::test"
//!
//! PASS_BEFORE "Inline"
//! INST        /Call.*MyMap::set/
//! INST        /Call.*MyMap::get/
//!
//! PASS_AFTER  "Inline"
//! INST        /Call.*MyMap::set/
//! INST_NEXT   /LoadObject.*MyMap\.arr/
//! INST_NOT    /Call.*Inlined.*Array::\$_set/
//!
//! INST        /Call.*MyMap::get/
//! INST_NEXT   /LoadObject.*MyMap\.arr/
//! INST_NOT    /Call.*Inlined.*Array::\$_get/

class MyArray<T> extends Array<T> {
    public myArrayGetCounter: number = 0
    public constructor(len: number) {
        super(len.toInt());
    }

    override $_get(index: int): T {
        this.myArrayGetCounter += 1;
        return super.$_get(index);
    }

    override $_set(index: int, val: T): void {
        return super.$_set(index, val);
    }
}

class MyMap<K, V> extends Map<K, V> {
    arr: Array<K>

    public constructor(size: number) {
        super()
        this.arr = new MyArray<K>(size)
    }

    public get(index: number): K {
        return this.arr.$_get(index.toInt())
    }

    public set(index: number, key: K, val: V) {
        this.arr.$_set(index.toInt(), key);
    }
}

function test() {
    let map: MyMap<number, number> = new MyMap<number, number>(5)
    map.set(0, 0, 0)
    map.set(1, 1, 1)
    let result = map.get(1)
}

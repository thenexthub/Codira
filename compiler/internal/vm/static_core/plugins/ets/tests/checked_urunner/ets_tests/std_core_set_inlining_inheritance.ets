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

class MyMap<K, V> extends Map<K, V> {
    public arr: Array<[K, V]> | undefined

    public set(key: K, value: V): this {
        if (this.arr == undefined) {
            this.arr = []
        }
        this.arr?.push([key, value])
        return this
    }

    public has(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem !== undefined && elem[0] == key) {
            return false
        }
        return true;
    }

    public delete(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem !== undefined && elem[0] == key) {
            return false
        }
        return true
    }
}

class MySet<K> extends Set<K> {
    public map: Map<K, K> | undefined

    add(key: K): this {
        if (this.map == undefined) {
            this.map = new MyMap<K, K>()
        }
        this.map?.set(key, key)
        return this;
    }

    public has(key: K): boolean {
        return this.map?.has(key) ?? false
    }

    public delete(key: K): boolean {
        return this.map?.delete(key) ?? false
    }
}

//! CHECKER       AOT: MySet<boolean> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_set|SetHas|SetDelete).*'",
//!               entry: "std_core_set_inlining_inheritance.ETSGLOBAL::testSetBoolOverrided"
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetHasBool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::has/
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetDeleteBool"
//! PASS_AFTER    "Inline"
//! INST          /CallVirtual.*std\.core\.Set::delete/
//!
//! RUN           entry: "std_core_set_inlining_inheritance.ETSGLOBAL::testSetBoolOverrided"

//! CHECKER       AOT PGO: MySet<boolean> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF  entry: "std_core_set_inlining_inheritance.ETSGLOBAL::testSetBoolOverrided"
//! RUN_PGO_PAOC  options: "--compiler-regex='.*(test_set|SetHas|SetDelete).*'"
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetHasBool"
//! PASS_AFTER    "Inline"
//! INST_NOT      /Call.*std\.core\.Map::has/
//! INST          /Call.*std_core_set_inlining_inheritance\.MySet::has/
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetDeleteBool"
//! PASS_AFTER    "Inline"
//! INST_NOT      /Call.*std\.core\.Map::delete/
//! INST          /Call.*std_core_set_inlining_inheritance\.MySet::delete/
//!
//! RUN           entry: "std_core_set_inlining_inheritance.ETSGLOBAL::testSetBoolOverrided"

//! CHECKER       JIT: MySet<boolean> overrided intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_set|SetHas|SetDelete).*'",
//!               entry: "std_core_set_inlining_inheritance.ETSGLOBAL::testSetBoolOverrided"
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetHasBool"
//! PASS_AFTER    "Inline"
//! INST_NOT      /Call.*std\.core\.Map::has/
//! METHOD        "std_core_set_inlining_inheritance.ETSGLOBAL::__noinline__SetDeleteBool"
//! PASS_AFTER    "Inline"
//! INST_NOT      /Call.*std\.core\.Map::delete/

function testSetBoolOverrided() {
    let set: Set<boolean> = new MySet<boolean>()
    set.add(false)
    arktest.assertTrue(__noinline__SetHasBool(set))
    arktest.assertTrue(__noinline__SetDeleteBool(set))
}

function __noinline__SetHasBool(set: Set<boolean>): boolean {
    return set.has(true);
}
function __noinline__SetDeleteBool(set: Set<boolean>): boolean {
    return set.delete(true);
}

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

class MapEntry<K, V> {
    key: K
    val: V

    constructor(key: K, val: V) {
        this.key = key
        this.val = val
    }
}

class MyMap<K, V> extends Map<K, V> {
    public arr: Array<MapEntry<K, V>> | undefined

    set(key: K, val: V): this {
        if (this.arr == undefined)
            this.arr = new Array<MapEntry<K, V>>()
        const newEntry = new MapEntry<K, V>(key, val)
        this.arr?.push(newEntry)
        return this;
    }

    public has(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return false
        return true;
    }

    public get(key: K): V | undefined {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return elem?.val
        return undefined
    }

    public delete(key: K): boolean {
        let elem = this.arr?.pop()
        if (elem?.key == key)
            return false
        return true
    }
}

//! CHECKER       AOT: MyMap<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*std\.core\.Map::has/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*std\.core\.Map::get/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*std\.core\.Map::delete/

//! CHECKER       AOT PGO: MyMap<boolean, double> overrided intrinsics
//! SKIP_IF       @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! RUN_PGO_PAOC options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'"
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*ets_escompat_map_inheritance_intrinsics\.MyMap::has/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*ets_escompat_map_inheritance_intrinsics\.MyMap::get/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*ets_escompat_map_inheritance_intrinsics\.MyMap::delete/

//! CHECKER       JIT: MyMap<boolean, double> overrided intrinsics
//! RUN           force_jit: true, options: "--compiler-regex='.*(test_map|MapHas|MapGet|MapDelete)_.*'", entry: "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::test_map_bool_double_overrided2"
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapHas_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapHas"
//! INST          /CallVirtual.*std\.core\.Map::has/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapGet_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapGet"
//! INST          /CallVirtual.*std\.core\.Map::get/
//! METHOD        "ets_escompat_map_inheritance_intrinsics.ETSGLOBAL::__noinline__MapDelete_bool_double"
//! PASS_AFTER    "Inline"
//! INST_NOT      "Intrinsic.MapDelete"
//! INST          /CallVirtual.*std\.core\.Map::delete/

function test_map_bool_double_overrided2() {
    let map: MyMap<boolean, double> = new MyMap<boolean, double>()
    map.set(false, 1.5)
    arktest.assertTrue(__noinline__MapHas_bool_double(map))
    arktest.assertEQ(__noinline__MapGet_bool_double(map), undefined)
    arktest.assertTrue(__noinline__MapDelete_bool_double(map))
}

function __noinline__MapHas_bool_double(map: Map<boolean, double>): boolean {
    return map.has(true);
}
function __noinline__MapGet_bool_double(map: Map<boolean, double>): double | undefined {
    return map.get(false);
}
function __noinline__MapDelete_bool_double(map: Map<boolean, double>): boolean {
    return map.delete(true);
}

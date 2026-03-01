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

//! CHECKER       JIT: indexOf() and lastIndexOf()
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,  options: "--compiler-regex=.*::test_.*array_.*", entry: "ets_escompat_typed_arrays_index_of_part4.ETSGLOBAL::main"
//! METHOD        "test_uint8array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::lastIndexOf/
//! METHOD        "test_uint8clampedarray_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::lastIndexOf/
//! METHOD        "test_uint16array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::lastIndexOf/
//! METHOD        "test_uint32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::lastIndexOf/
//! METHOD        "test_biguint64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::lastIndexOf/


//! CHECKER       AOT: indexOf() and lastIndexOf()
//! RUN_PAOC      options: "--compiler-regex=.*::test_.*array_.*", entry: "ets_escompat_typed_arrays_index_of_part4.ETSGLOBAL::main"
//! METHOD        "test_uint8array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8Array::lastIndexOf/
//! METHOD        "test_uint8clampedarray_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint8ClampedArray::lastIndexOf/
//! METHOD        "test_uint16array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint16Array::lastIndexOf/
//! METHOD        "test_uint32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Uint32Array::lastIndexOf/
//! METHOD        "test_biguint64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigUint64Array::lastIndexOf/


function test_uint8array_last_index_of() {
    let array = new Uint8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

function test_uint16array_last_index_of() {
    let array = new Uint16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

function test_uint32array_last_index_of() {
    let array = new Uint32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

function test_biguint64array_last_index_of() {
    let array = new BigUint64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 ,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 ,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  8),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -1),  4.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3), -7), -1.);
}

function test_uint8clampedarray_last_index_of() {
    let array = new Uint8ClampedArray([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}


function main(): void {
    test_uint8array_last_index_of()
    test_uint8clampedarray_last_index_of()
    test_uint16array_last_index_of()
    test_uint32array_last_index_of()
    test_biguint64array_last_index_of()
}

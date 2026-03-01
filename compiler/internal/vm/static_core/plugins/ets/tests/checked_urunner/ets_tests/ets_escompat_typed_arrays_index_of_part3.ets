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
//! RUN           force_jit: true,  options: "--compiler-regex=.*::test_.*array_.*", entry: "ets_escompat_typed_arrays_index_of_part3.ETSGLOBAL::main"
//! METHOD        "test_int8array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int8Array::lastIndexOf/
//! METHOD        "test_int16array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int16Array::lastIndexOf/
//! METHOD        "test_int32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int32Array::lastIndexOf/
//! METHOD        "test_bigint64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigInt64Array::lastIndexOf/
//!
//! METHOD        "test_float32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float32Array::lastIndexOf/
//! METHOD        "test_float64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float64Array::lastIndexOf/


//! CHECKER       AOT: indexOf() and lastIndexOf()
//! RUN_PAOC      options: "--compiler-regex=.*::test_.*array_.*", entry: "ets_escompat_typed_arrays_index_of_part3.ETSGLOBAL::main"
//! METHOD        "test_int8array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int8Array::lastIndexOf/
//! METHOD        "test_int16array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int16Array::lastIndexOf/
//! METHOD        "test_int32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Int32Array::lastIndexOf/
//! METHOD        "test_bigint64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.BigInt64Array::lastIndexOf/
//!
//! METHOD        "test_float32array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float32Array::lastIndexOf/
//! METHOD        "test_float64array_last_index_of"
//! PASS_AFTER    "IrBuilder"
//! INST          /Call.* escompat.Float64Array::lastIndexOf/

function test_int8array_last_index_of() {
    let array = new Int8Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3. as double, -1 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3. as double, -7 as int), -1.);
}

function test_int16array_last_index_of() {
    let array = new Int16Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3.,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3.,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int, -1 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int, -7 as int), -1.);
}

function test_int32array_last_index_of() {
    let array = new Int32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int, -1 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}

function test_bigint64array_last_index_of() {
    let array = new BigInt64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  0), -1.);
    arktest.assertEQ(array.lastIndexOf(new BigInt(3),  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3, -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3, -7), -1.);
}

function test_float32array_last_index_of() {
    let array = new Float32Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3.,  0), -1.);
    arktest.assertEQ(array.lastIndexOf(3.,  6),  4.);
    arktest.assertEQ(array.lastIndexOf(3.,  8),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int, -1 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int, -7 as int), -1.);
}

function test_float64array_last_index_of() {
    let array = new Float64Array([1, 2, 3, 3, 3, 4, 5])
    arktest.assertEQ(array.lastIndexOf(3 as int,  0 as int), -1.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  6 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3 as int,  8 as int),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -1),  4.);
    arktest.assertEQ(array.lastIndexOf(3., -7), -1.);
}


function main(): void {
    test_int8array_last_index_of()
    test_int16array_last_index_of()
    test_int32array_last_index_of()
    test_bigint64array_last_index_of()
    test_float32array_last_index_of()
    test_float64array_last_index_of()
}

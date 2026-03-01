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

//! CHECKER       Int8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int8array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_rangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Int8Array, src: Int8Array) {
    dst.set(src)
}

//! CHECKER       Int8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int8array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_with_offset() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_int8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int8array() {
    let sample = Int8Array.of(1, 2, 3, 4)
    let src = new Int8Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int8Array.of(1, 1, 2, 4))
}

//! CHECKER       Int8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int8array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int8array_with_offset_rangeerror() {
    let src = Int8Array.of(1, 2, 3, 4, 5)
    let dst = new Int8Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Int8Array, src: Int8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int16array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_rangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Int16Array, src: Int16Array) {
    dst.set(src)
}

//! CHECKER       Int16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int16array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_with_offset() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_int16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int16array() {
    let sample = Int16Array.of(1, 2, 3, 4)
    let src = new Int16Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int16Array.of(1, 1, 2, 4))
}

//! CHECKER       Int16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int16array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int16array_with_offset_rangeerror() {
    let src = Int16Array.of(1, 2, 3, 4, 5)
    let dst = new Int16Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Int16Array, src: Int16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Int32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Int32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Int32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_rangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Int32Array, src: Int32Array) {
    dst.set(src)
}

//! CHECKER       Int32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_with_offset() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Int32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Int32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_int32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_int32array() {
    let sample = Int32Array.of(1, 2, 3, 4)
    let src = new Int32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Int32Array.of(1, 1, 2, 4))
}

//! CHECKER       Int32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_int32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Int32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_int32array_with_offset_rangeerror() {
    let src = Int32Array.of(1, 2, 3, 4, 5)
    let dst = new Int32Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Int32Array, src: Int32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigInt64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_bigint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, BigInt64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigInt64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_bigint64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_rangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: BigInt64Array, src: BigInt64Array) {
    dst.set(src)
}

//! CHECKER       BigInt64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_bigint64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_with_offset() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, BigInt64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigInt64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_bigint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_bigint64array() {
    let sample = BigInt64Array.of(1, 2, 3, 4)
    let src = new BigInt64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, BigInt64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigInt64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_bigint64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_bigint64array_with_offset_rangeerror() {
    let src = BigInt64Array.of(1, 2, 3, 4, 5)
    let dst = new BigInt64Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: BigInt64Array, src: BigInt64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Float32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_rangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Float32Array, src: Float32Array) {
    dst.set(src)
}

//! CHECKER       Float32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_with_offset() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Float32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_float32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_float32array() {
    let sample = Float32Array.of(1, 2, 3, 4)
    let src = new Float32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Float32Array.of(1, 1, 2, 4))
}

//! CHECKER       Float32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float32array_with_offset_rangeerror() {
    let src = Float32Array.of(1, 2, 3, 4, 5)
    let dst = new Float32Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Float32Array, src: Float32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Float64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Float64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Float64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_rangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Float64Array, src: Float64Array) {
    dst.set(src)
}

//! CHECKER       Float64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_with_offset() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Float64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Float64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_float64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_float64array() {
    let sample = Float64Array.of(1, 2, 3, 4)
    let src = new Float64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Float64Array.of(1, 1, 2, 4))
}

//! CHECKER       Float64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_float64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.Float64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_float64array_with_offset_rangeerror() {
    let src = Float64Array.of(1, 2, 3, 4, 5)
    let dst = new Float64Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Float64Array, src: Float64Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint8Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       AOT Uint8Array set array intrinsic with RangeError
//! RUN_PAOC      options: "--compiler-regex-with-signature='.*::set_array\\(escompat.Uint8Array, escompat.Uint8Array\\)'"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array_rangeerror"

//! CHECKER       Uint8Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_rangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Uint8Array, src: Uint8Array) {
    dst.set(src)
}

//! CHECKER       AOT Uint8Array set array with offset intrinsic
//! RUN_PAOC      options: "--compiler-regex-with-signature='.*::set_array\\(escompat.Uint8Array, escompat.Uint8Array, i32\\)'"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
//! RUN           entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array_with_offset"

//! CHECKER       Uint8Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_with_offset() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint8Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_uint8array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint8array() {
    let sample = Uint8Array.of(1, 2, 3, 4)
    let src = new Uint8Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint8Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint8Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8array_with_offset_rangeerror() {
    let src = Uint8Array.of(1, 2, 3, 4, 5)
    let dst = new Uint8Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Uint8Array, src: Uint8Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint8ClampedArray set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint8ClampedArray.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint8ClampedArray set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8clampedarray_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_rangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Uint8ClampedArray, src: Uint8ClampedArray) {
    dst.set(src)
}

//! CHECKER       Uint8ClampedArray set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8clampedarray_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_with_offset() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint8ClampedArray.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint8ClampedArray set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_uint8clampedarray"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint8clampedarray() {
    let sample = Uint8ClampedArray.of(1, 2, 3, 4)
    let src = new Uint8ClampedArray(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint8ClampedArray.of(1, 1, 2, 4))
}

//! CHECKER       Uint8ClampedArray set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint8clampedarray_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt8ClampedArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint8clampedarray_with_offset_rangeerror() {
    let src = Uint8ClampedArray.of(1, 2, 3, 4, 5)
    let dst = new Uint8ClampedArray(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Uint8ClampedArray, src: Uint8ClampedArray, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint16Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint16Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint16Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint16array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_rangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Uint16Array, src: Uint16Array) {
    dst.set(src)
}

//! CHECKER       Uint16Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint16array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_with_offset() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint16Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint16Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_uint16array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint16array() {
    let sample = Uint16Array.of(1, 2, 3, 4)
    let src = new Uint16Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint16Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint16Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint16array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt16ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint16array_with_offset_rangeerror() {
    let src = Uint16Array.of(1, 2, 3, 4, 5)
    let dst = new Uint16Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Uint16Array, src: Uint16Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       Uint32Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, Uint32Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       Uint32Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint32array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_rangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: Uint32Array, src: Uint32Array) {
    dst.set(src)
}

//! CHECKER       Uint32Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint32array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_with_offset() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, Uint32Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       Uint32Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_uint32array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_uint32array() {
    let sample = Uint32Array.of(1, 2, 3, 4)
    let src = new Uint32Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, Uint32Array.of(1, 1, 2, 4))
}

//! CHECKER       Uint32Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_uint32array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.UInt32ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_uint32array_with_offset_rangeerror() {
    let src = Uint32Array.of(1, 2, 3, 4, 5)
    let dst = new Uint32Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

function set_array(dst: Uint32Array, src: Uint32Array, offset: int) {
    dst.set(src, offset)
}

//! CHECKER       BigUint64Array set array intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_biguint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    set_array(dst, src)
    check_array(dst, BigUint64Array.of(1, 2, 3, 4, 5, 77, 77, 77, 77, 77))
}

//! CHECKER       BigUint64Array set array intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_biguint64array_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValues "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_rangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(4)
    arktest.expectThrow(() => { set_array(dst, src) }, expectedRangeErrors)
}

function set_array(dst: BigUint64Array, src: BigUint64Array) {
    dst.set(src)
}

//! CHECKER       BigUint64Array set array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_biguint64array_with_offset"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_with_offset() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(10)
    dst.fill(77)
    set_array(dst, src, 4)
    check_array(dst, BigUint64Array.of(77, 77, 77, 77, 1, 2, 3, 4, 5, 77))
}

//! CHECKER       BigUint64Array set the same array with offset intrinsic
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_same_biguint64array"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_same_biguint64array() {
    let sample = BigUint64Array.of(1, 2, 3, 4)
    let src = new BigUint64Array(sample.buffer, 0, 2)
    set_array(sample, src, 1)
    check_array(sample, BigUint64Array.of(1, 1, 2, 4))
}

//! CHECKER       BigUint64Array set array with offest intrinsic with RangeError
//! RUN           force_jit: true, options: "--compiler-regex=.*::set_array", entry: "ets_escompat_typed_arrays_set_values.ETSGLOBAL::test_set_biguint64array_with_offset_rangeerror"
//! METHOD        "ets_escompat_typed_arrays_set_values.ETSGLOBAL::set_array"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.BigUInt64ArraySetValuesWithOffset "
//! INST_NOT      /Call.*Array::set/
function test_set_biguint64array_with_offset_rangeerror() {
    let src = BigUint64Array.of(1, 2, 3, 4, 5)
    let dst = new BigUint64Array(5)
    arktest.expectThrow(() => { set_array(dst, src, 1) }, expectedRangeErrors)
}

// Helpers
const expectedRangeErrors = (e: Error): boolean => {
    return (e instanceof RangeError)
}

function set_array(dst: BigUint64Array, src: BigUint64Array, offset: int) {
    dst.set(src, offset)
}

function check_array(arr: ArrayLike<number>, expected: ArrayLike<number>) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigInt64Array, expected: BigInt64Array) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

function check_array(arr: BigUint64Array, expected: BigUint64Array) {
    if (arr.length != expected.length) {
        throw new Error("Unexpected array length")
    }
    for (let i = 0; i < expected.length; i++) {
        if (arr[i] != expected[i]) {
            throw new Error("Unexpected element with index " + i)
        }
    }
}

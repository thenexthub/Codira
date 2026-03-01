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
import * as js from "indirect_js"

function Test_indirect_call_type_ref_array_call(): boolean {
    const ARG: FixedArray<number> = [42, 73];

    let result = js.indirectCallTypeByRefArray.call(null, ARG) as FixedArray<number>;
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_array_apply(): boolean {
    const ARG: FixedArray<number> = [42, 73];

    let result = js.indirectCallTypeByRefArray.apply(null, [ARG] as FixedArray<FixedArray<number>>) as FixedArray<number>;
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_array_bind_with_arg(): boolean {
    const ARG: FixedArray<number> = [42, 73];

    let boundWithArg = js.indirectCallTypeByRefArray.bind(null, ARG);
    let result = boundWithArg() as FixedArray<number>;

    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_array_bind_without_arg(): boolean {
    const ARG: FixedArray<number> = [42, 73];

    let boundWithoutArg = js.indirectCallTypeByRefArray.bind(null);
    let result = boundWithoutArg(ARG) as FixedArray<number>;

    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_tuple_call(): boolean {
    const ARG: [number, string] = [42, '73'];

    let result = js.indirectCallTypeByRefTuple.call(null, ARG) as [number, string];
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_tuple_apply(): boolean {
    const ARG: [number, string] = [42, '73'];

    let result = js.indirectCallTypeByRefTuple.apply(null, [ARG] as FixedArray<[number, string]>) as [number, string];
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_tuple_bind_with_arg(): boolean {
    const ARG: [number, string] = [42, '73'];

    let boundWithArg = js.indirectCallTypeByRefTuple.bind(null, ARG);
    let result = boundWithArg() as [number, string];
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_tuple_bind_without_arg(): boolean {
    const ARG: [number, string] = [42, '73'];

    let boundWithoutArg = js.indirectCallTypeByRefTuple.bind(null);
    let result = boundWithoutArg(ARG) as [number, string];
    arktest.assertTrue(JSON.stringify(result) == JSON.stringify(ARG));

    return true;
}

function Test_indirect_call_type_ref_map_call(): boolean {
    const KEY1 = 'key1';
    const KEY2 = 'key2';
    const EXPECTED_VALUE = 2;

    let arg = new Map<string, number>();
    arg.set(KEY1, 1);

    let result = js.indirectCallTypeByRefMap.call(null, arg) as Map<string, number>;

    arktest.assertTrue(result.get(KEY2) == EXPECTED_VALUE);

    return true;
}

function Test_indirect_call_type_ref_map_apply(): boolean {
    const KEY1 = 'key1';
    const KEY2 = 'key2';
    const EXPECTED_VALUE = 2;

    let arg = new Map<string, number>();
    arg.set(KEY1, 1);

    let result = js.indirectCallTypeByRefMap.apply(null, [arg]) as Map<string, number>;

    arktest.assertTrue(result.get(KEY2) == EXPECTED_VALUE);

    return true;
}

function Test_indirect_call_type_ref_map_bind_with_arg(): boolean {
    const KEY1 = 'key1';
    const KEY2 = 'key2';
    const EXPECTED_VALUE = 2;

    let arg = new Map<string, number>();
    arg.set(KEY1, 1);

    let boundWithArg = js.indirectCallTypeByRefMap.bind(null, arg);
    let result = boundWithArg() as Map<string, number>;

    arktest.assertTrue(result.get(KEY2) == EXPECTED_VALUE);

    return true;
}

function Test_indirect_call_type_ref_map_bind_without_arg(): boolean {
    const KEY1 = 'key1';
    const KEY2 = 'key2';
    const EXPECTED_VALUE = 2;

    let arg = new Map<string, number>();
    arg.set(KEY1, 1);

    let boundWithArg = js.indirectCallTypeByRefMap.bind(null);
    let result = boundWithArg(arg) as Map<string, number>;

    arktest.assertTrue(result.get(KEY2) == EXPECTED_VALUE);

    return true;
}

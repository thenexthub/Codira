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

function Test_indirect_call_type_extra_set_call(): boolean {
    const ARG_STRING = 'foo';
    const EXPECTED_STRING = 'foobar'
    const EXPECTED_NUMBER = 2;

    let arg: [string, number] = [ARG_STRING, 1 as number];
    let reply: [string, number] = js.indirectCallTypeExtraSet.call(null, arg);

    let result_string: string;
    let result_number: number;
    // [result_string, result_number] = reply; // Segmentation fault (core dumped)

    // arktest.assertTrue(result_string == EXPECTED_STRING);
    // arktest.assertTrue(result_number == EXPECTED_NUMBER);

    // return true;
    return false;
}

function Test_indirect_call_type_extra_set_apply(): boolean {
    const ARG_STRING = 'foo';
    const EXPECTED_STRING = 'foobar'
    const EXPECTED_NUMBER = 2;

    let arg: [string, number] = [ARG_STRING, 1];
    let reply: [string, number] = js.indirectCallTypeExtraSet.apply(null, [arg]);

    let result_string: string;
    let result_number: number;
    // [result_string, result_number] = reply; // Segmentation fault (core dumped)

    // arktest.assertTrue(result_string == EXPECTED_STRING);
    // arktest.assertTrue(result_number == EXPECTED_NUMBER);

    // return true;
    return false;
}

function Test_indirect_call_type_extra_set_bind_with_arg(): boolean {
    const ARG_STRING = 'foo';
    const EXPECTED_STRING = 'foobar'
    const EXPECTED_NUMBER = 2;

    let arg: [string, number] = [ARG_STRING, 1];
    let boundWithArg = js.indirectCallTypeExtraSet.bind(null, arg);
    let reply: [string, number] = boundWithArg();

    let result_string: string;
    let result_number: number;

    // [result_string, result_number] = reply; // Segmentation fault (core dumped)
    // arktest.assertTrue(result_string == EXPECTED_STRING);
    // arktest.assertTrue(result_number == EXPECTED_NUMBER);

    // return true;
    return false;
}

function Test_indirect_call_type_extra_set_bind_without_arg(): boolean {
    const ARG_STRING = 'foo';
    const EXPECTED_STRING = 'foobar'
    const EXPECTED_NUMBER = 2;

    let arg: [string, number] = [ARG_STRING, 1];
    let boundWithoutArg = js.indirectCallTypeExtraSet.bind(null);
    let reply: [string, number] = boundWithoutArg(arg);

    let result_string: string;
    let result_number: number;

    // [result_string, result_number] = reply; // Segmentation fault (core dumped)
    // arktest.assertTrue(result_string == EXPECTED_STRING);
    // arktest.assertTrue(result_number == EXPECTED_NUMBER);

    // return true;
    return false;
}

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

function Test_indirect_call_type_union_call_arg_numeric(): boolean {
    const NUMERIC_VALUE: (number | string) = 1;

    let result: (number | string) = js.indirectCallUnion.call(null, NUMERIC_VALUE) as (number | string);
    arktest.assertTrue(typeof result == "number" && result as number == NUMERIC_VALUE);

    return true;
}

function Test_indirect_call_type_union_call_arg_string(): boolean {
    const STRING_VALUE: (number | string) = '1';

    let result: (number | string) = js.indirectCallUnion.call(null, STRING_VALUE) as (number | string);
    arktest.assertTrue(typeof result == "string" && result as string == STRING_VALUE);

    return true;
}

function Test_indirect_call_type_union_apply_arg_numeric(): boolean {
    const NUMERIC_VALUE: (number | string) = 1;

    let result: (number | string) = js.indirectCallUnion.apply(null, [NUMERIC_VALUE] as FixedArray<(number | string)>) as (number | string);
    arktest.assertTrue(typeof result == "number" && result as number == NUMERIC_VALUE);

    return true;
}

function Test_indirect_call_type_union_apply_arg_string(): boolean {
    const STRING_VALUE: (number | string) = '1';

    let result: (number | string)  = js.indirectCallUnion.apply(null, [STRING_VALUE] as FixedArray<(number | string)>) as (number | string);
    arktest.assertTrue(typeof result == "string" && result as string == STRING_VALUE);

    return true;
}

function Test_indirect_call_type_union_bind_with_arg_numeric(): boolean {
    const NUMERIC_VALUE: (number | string) = 1;

    let boundWithNumericArg = js.indirectCallUnion.bind(null, NUMERIC_VALUE);
    let result: (number | string) = boundWithNumericArg() as (number | string);
    arktest.assertTrue(typeof result == "number" && result as number == NUMERIC_VALUE);

    return true;
}

function Test_indirect_call_type_union_bind_with_arg_string(): boolean {
    const STRING_VALUE: (number | string) = '1';

    let boundWithStringArg = js.indirectCallUnion.bind(null, STRING_VALUE);
    let result: (number | string) = boundWithStringArg() as (number | string);
    arktest.assertTrue(typeof result == "string" && result as string == STRING_VALUE);

    return true;
}

function Test_indirect_call_type_union_bind_without_arg_numeric(): boolean {
    const NUMERIC_VALUE: (number | string) = 1;

    let boundWithoutArg = js.indirectCallUnion.bind(null);
    let result: (number | string) = boundWithoutArg(NUMERIC_VALUE) as (number | string);
    arktest.assertTrue(typeof result == "number" && result as number == NUMERIC_VALUE);

    return true;
}

function Test_indirect_call_type_union_bind_without_arg_string(): boolean {
    const STRING_VALUE: (number | string) = '1';

    let boundWithoutArg = js.indirectCallUnion.bind(null);
    let result: (number | string) = boundWithoutArg(STRING_VALUE) as (number | string);
    arktest.assertTrue(typeof result == "string" && result as string == STRING_VALUE);

    return true;
}

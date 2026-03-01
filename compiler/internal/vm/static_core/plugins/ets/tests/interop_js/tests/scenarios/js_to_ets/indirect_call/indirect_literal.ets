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

function Test_indirect_call_type_literal_call(): boolean {
    const ARG = 'foo';
    const EXPECTED_RESULT = 'foobar'

    let result: string = js.indirectCallTypeLiteral.call(null, ARG);
    arktest.assertTrue(result == EXPECTED_RESULT);

    return true;
}

function Test_indirect_call_type_literal_apply(): boolean {
    const ARG = 'foo';
    const EXPECTED_RESULT = 'foobar'

    let result: string = js.indirectCallTypeLiteral.apply(null, [ARG] as FixedArray<string>);
    arktest.assertTrue(result == EXPECTED_RESULT);

    return true;
}

function Test_indirect_call_type_literal_bind_with_arg(): boolean {
    const ARG = 'foo';
    const EXPECTED_RESULT = 'foobar'

    let boundWithArg = js.indirectCallTypeLiteral.bind(null, ARG);
    arktest.assertTrue(boundWithArg() == EXPECTED_RESULT);

    return true;
}

function Test_indirect_call_type_literal_bind_without_arg(): boolean {
    const ARG = 'foo';
    const EXPECTED_RESULT = 'foobar'

    let boundWithoutArg = js.indirectCallTypeLiteral.bind(null);
    arktest.assertTrue(boundWithoutArg(ARG) == EXPECTED_RESULT);

    return true;
}

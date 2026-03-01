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

function dummy(): number {
    return 1;
}

function Test_indirect_call_test_return_value(): boolean {
    // NOTE #18409 remove this comment block after interop will return valid value from the test.
    // Current test always returns true after following line invocation,
    // even if there is explicit 'return false' statement.
    dummy();
    return false;
}

function Test_indirect_call_type_any_call(): boolean {
    let result = js.indirectCallTypeAny.call(null, 1) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_any_apply(): boolean {
    let result = js.indirectCallTypeAny.apply(null, [1] as FixedArray<int>) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_any_bind_with_arg(): boolean {
    let boundWithArg = js.indirectCallTypeAny.bind(null, 1);
    arktest.assertTrue(boundWithArg() as int == 1);

    return true;
}

function Test_indirect_call_type_any_bind_without_arg(): boolean {
    let boundWithoutArg = js.indirectCallTypeAny.bind(null);
    arktest.assertTrue(boundWithoutArg(1) as int == 1);

    return true;
}

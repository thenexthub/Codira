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
import { UserClass } from "indirect_js"

function Test_indirect_call_type_user_class_call(): boolean {
    let obj = new UserClass();
    let result = obj.method.call(null, 1) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_class_apply(): boolean {
    let obj = new UserClass();
    let result = obj.method.apply(null, [1] as FixedArray<int>) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_class_bind_with_arg(): boolean {
    let obj = new UserClass();

    let boundWithArg = obj.method.bind(null, 1);
    let result = boundWithArg() as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_class_bind_without_arg(): boolean {
    let obj = new UserClass();

    let boundWithoutArg = obj.method.bind(null);
    let result = boundWithoutArg(1) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_interface_call(): boolean {
    let result = js.interfaceFunc().interfaceMethod.call(null, 1) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_interface_apply(): boolean {
    let result = js.interfaceFunc().interfaceMethod.apply(null, [1] as FixedArray<int>) as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_interface_bind_with_arg(): boolean {

    let boundWithArg = js.interfaceFunc().interfaceMethod.bind(null, 1);
    let result = boundWithArg() as int;
    arktest.assertTrue(result == 1);

    return true;
}

function Test_indirect_call_type_user_interface_bind_without_arg(): boolean {
    let boundWithoutArg = js.interfaceFunc().interfaceMethod.bind(null);
    let result = boundWithoutArg(1) as int;
    arktest.assertTrue(result == 1);

    return true;
}

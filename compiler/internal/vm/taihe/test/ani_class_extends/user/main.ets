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
import {BusinessError} from "@ohos.base";
import * as foo from "foo";

loadLibrary("ani_class_extends");

function check_struct_class_extend() {
    let data = foo.makeDerivedDataClass();
    arktest.assertEQ(data.a, "base");
    arktest.assertEQ(data.p, "foo");
    arktest.assertEQ(data.q, "bar");
    arktest.assertEQ(data.x, 42);
    arktest.assertEQ(data.y, 56);
}

function check_interface_class_extend() {
    let obj = new foo.DerivedMethodClass();
    arktest.assertEQ(obj.base(), "base");
    arktest.assertEQ(obj.foo(), "foo");
    arktest.assertEQ(obj.bar(), "bar");
    arktest.assertEQ(obj.derived(), "derived");
}

function main() {
    const suite = new arktest.ArkTestsuite("class extend ut")
    suite.addTest("check struct class extend", check_struct_class_extend)
    suite.addTest("check interface class extend", check_interface_class_extend)
    exit(suite.run())
}
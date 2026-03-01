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
import * as test from "inject_test";
import * as moduleA from "moduleA";

loadLibrary("ani_inject");

function main() {
    let res1: String = moduleA.toplevelFunc("hello");
    console.log("toplevelFunc: " + res1);
    let res2: int = moduleA.foo.foo(100);
    console.log("foo.foo: " + res2);
    let foo = new test.Foo();
    foo.callWithThis();
    foo.callWithThis();
    let structObj: test.RGB = {
        color: "red",
        r: 255,
        g: 0,
        b: 0,
    };
    console.log(structObj.color + " " + structObj.r);
}

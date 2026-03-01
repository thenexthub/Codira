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
import * as Test from "finalization_test";

loadLibrary("ani_finalization");

function doFullGC() {
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
    } catch (e) {
        arktest.assertTrue(false, "Unexpected exception during GC");
    }
}

function test_finalization() {
    let flag: int = 0;

    let foo: Test.Foo|null = new Test.Foo();

    foo.addCallback(() => {
        console.log("Bye!");
    })

    foo.addCallback(() => {
        flag = 42;
    })

    foo = null;
    console.log("Doing full GC after setting foo to null...");
    doFullGC()
    Coroutine.Schedule()

    arktest.assertEQ(flag, 42);
}

function main() {
    console.log("begin")

    const suite = new arktest.ArkTestsuite("FuncTest Test");

    suite.addTest("test finalization", test_finalization);

    exit(suite.run());
}

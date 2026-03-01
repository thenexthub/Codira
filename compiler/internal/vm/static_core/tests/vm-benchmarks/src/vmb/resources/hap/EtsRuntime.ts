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

import hilog from "@ohos.hilog";

export function InitEtsRuntime(bench_file: string, bench_mode: string): void {
    globalThis.Panda = globalThis.requireNapi("ets_interop_js_napi", true);
    const libs = "/data/storage/el1/bundle/libs/arm64";
    const bench = libs + "/" + bench_file;
    const stdlib = libs + "/etsstdlib.abc.so";
    const opts = {
        "panda-files": bench,
        "boot-panda-files": stdlib + ":" + bench,
        "gc-trigger-type": "heap-trigger",
        "load-runtimes": "ets",
        "compiler-enable-jit": "false",
        "run-gc-in-place": "true"
    };
    if ("aot" == bench_mode) {
        opts["enable-an:force"] = "true";
        opts["aot-file"] = libs + "/aot_file.an.so";
    }
    if ("jit" == bench_mode) {
        opts["compiler-enable-jit"] = "true";
        opts["no-async-jit"] = "false";
    }
    if ("int" == bench_mode) {
        opts["compiler-enable-jit"] = "false";
    }
    if (!globalThis.Panda.createRuntime(opts)) {
        hilog.info(0, "VMB", "%{public}s", "ETS RUNTIME CREATE ERROR");
        throw new Error("ERROR CREATING ETS RUNTIME");
    } else {
        hilog.info(0, "VMB", "%{public}s", "ETS RUNTIME CREATE OK");
    }
}

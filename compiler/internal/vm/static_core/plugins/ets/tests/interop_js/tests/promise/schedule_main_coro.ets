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

import { launch } from "std/concurrency"
import { Job } from "std/core"

let was_scheduled = false;
let js_is_ready = false;

function wasScheduled(): boolean {
    return was_scheduled;
}

function jsIsReady() {
    js_is_ready = true;
}

async function asyncFoo(p: Job<void>): Promise<void> {
    p.Await();
    was_scheduled = true;
    return Promise.resolve();
}

function foo() {
    // it is expected that foo was launched on NON_MAIN WORKER
    while (!js_is_ready) {}
}

function waitUntillJsIsReady() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    let p = launch<void, () => void>(foo);
    asyncFoo(p);
}

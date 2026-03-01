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
function delay(timeoutMs: number): void {
    const startTime = Date.now();
    while (true) {
        const currentTime = Date.now();
        const elapsedTime = currentTime - startTime;
        if (elapsedTime >= timeoutMs) {
            break;
        }
        for (let i = 0; i < 1e3; i++) {}
    }
}

function func() {
    console.log("hello");
    console.log("world");
}

function returnAbc(): string {
    delay(1500)
    func(); // #BP{1}
    func();
    delay(5000)
    return (() => "a")() + (() => "b")() + (() => "c")();
}

function returnDef(): string {
    delay(10000)
    return (() => "de")() + (() => "f")();
}

function main(): int {
    let g2 = new taskpool.TaskGroup("g2");
    let t1 = new taskpool.Task(returnAbc);
    let t2 = new taskpool.Task(returnDef);
    g2.addTask(t1);
    g2.addTask(t2);
    let res2 = await taskpool.execute(g2);
    return 0;   // #BP{2}
}

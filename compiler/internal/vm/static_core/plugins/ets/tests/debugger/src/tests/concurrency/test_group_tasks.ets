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

function returnAbc(): string {
    return (() => "a")() + (() => "b")() + (() => "c")();
}

function returnDef(): string {
    return (() => "de")() + (() => "f")();
}

function sum(a: int, b: int): int {
    return a + b;
}

function max(a: int, b: int): int {
    return (a > b) ? a : b;
}

function main(): int {
    let g2 = new taskpool.TaskGroup("g2");
    let t1 = new taskpool.Task(returnAbc);
    let t2 = new taskpool.Task(returnDef);
    let t3 = new taskpool.Task(sum, 10, 20);
    let t4 = new taskpool.Task(max, 80, 30);
    g2.addTask(t1);
    g2.addTask(t2);
    g2.addTask(t3);
    g2.addTask(t4);
    let res2 = await taskpool.execute(g2);
    // Run group without tasks
    let g0 = new taskpool.TaskGroup("g0");
    let res0 = await taskpool.execute(g0);
    return 0;   // #BP
}

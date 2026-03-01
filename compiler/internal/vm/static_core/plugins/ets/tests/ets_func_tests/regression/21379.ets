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

/*---
desc: Issue 21379. Issues with Partial
tags: []
---*/


interface Filler {
    push(value: long)
}

class StaticWorker0 implements Filler {
    private tasks: containers.BlockingQueue<long>;
    constructor () {
        this.tasks = new containers.LinkedBlockingQueue<long>()
    }

    push(value: long) {
        this.tasks.push(value)
    }
}

class StaticWorker1 implements Filler {
    private tasks: containers.BlockingQueue<long> = new containers.LinkedBlockingQueue<long>()
    push(value: long) {
        this.tasks.push(value)
    }
}

function main() {
    let worker0: Filler = new StaticWorker0
    worker0.push(42)
    let worker1: Filler = new StaticWorker1
    worker1.push(42)
}


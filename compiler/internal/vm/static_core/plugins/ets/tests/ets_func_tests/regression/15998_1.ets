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
desc: Issue 15998. Contravariant arg overriding for functional type
tags: []
---*/

let test: int = 0;

interface RA<out T> {
    foo(fn: (x: RA<T>) => void): void;
}

class RAImpl<T> implements RA<T> {
    override foo(fn: (x: RAImpl<T>) => void): void { fn(this) }
}

function main() {
    test = 0;

    const work = new RAImpl<String>()
    work.foo((x: RAImpl<String>) => { test++ })

    arktest.assertEQ(1, test)
}

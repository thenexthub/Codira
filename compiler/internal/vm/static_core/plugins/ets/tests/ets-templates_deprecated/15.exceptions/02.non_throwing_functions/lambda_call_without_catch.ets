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
desc: Throwing function call if a call of a throwing function is not enclosed in a try statement;.
assert: Error propagation occurs in two cases. If a call of a throwing function is not enclosed in a try statement; If the enclosed try statement does not contain a clause that catches the raised error.
tags: [compile-only, negative]
---*/

class TestError extends Error {}
type func = (x: int) => int;

function main(): void {
  let bar: func = (x: int): int => { throw new TestError(); return 5; };

  let i: int = bar(3);
}

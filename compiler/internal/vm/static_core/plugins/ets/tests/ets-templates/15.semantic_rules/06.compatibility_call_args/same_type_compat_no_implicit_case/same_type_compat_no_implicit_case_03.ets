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
desc: T1 is compatible with T2 if implicitly convertible. Class does not implement interface.
---*/

function checkTypeIdentity(a: Any, b: Any): boolean {
    return Class.of(a as Object) === Class.of(b as Object);
}

function main(): void {
  const val1: boolean = true;
  const val2: boolean = false;
  const diffTypeVal: Any = "Hello, World!";

  const result1 = checkTypeIdentity(val1, val2);

  const result2 = checkTypeIdentity(val1, diffTypeVal);

  arktest.assertEQ(result1, true);
  arktest.assertEQ(result2, false);
}
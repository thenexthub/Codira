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
desc: null safety -> Not-null operator
assert: For an expression e!, if the type of e is nullable, an ensure not-null expression checks whether the evaluation of e is equal to null and, if it is, throws a NullPointerError.
---*/

function foo(arg: Object) : int {
  return 1;
}

function foo(arg: Object|null) : int {
  return 2;
}

function main(): void {
  let a: Object|null = new Object();
  let b: Object|null = null;

  arktest.assertEQ(foo(a), 1) // smart-casting works here
  arktest.assertEQ(foo(a!), 1)
  arktest.assertEQ(foo(b), 2)
}

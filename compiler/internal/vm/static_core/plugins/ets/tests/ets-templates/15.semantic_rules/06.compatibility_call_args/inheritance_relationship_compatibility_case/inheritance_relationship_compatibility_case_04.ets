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
desc: Verifying compatibility in generic class inheritance.
---*/

class Parent<T> {}
class Child<T> extends Parent<T> {}

function checkTypeIdentity(a: Any, b: Any): boolean {
    return Class.of(a as Object) === Class.of(b as Object);
}

function main(): void {
  const val1: Child<string> = new Child<string>();
  const val2: Parent<string> = new Parent<string>();

  const result1 = checkTypeIdentity(val1, val2);

  arktest.assertEQ(result1, false);
}
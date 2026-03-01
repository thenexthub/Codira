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
desc: Verifying compatibility between interface and implementing class.
---*/

interface MyInterface {
  method(): void;
}

class MyClass implements MyInterface {
  method(): void {}
}

function main(): void {
  const val1: MyClass = new MyClass();
  const val2: MyInterface = { method(): void {} };

  arktest.assertTrue(val1 instanceof MyClass);
  arktest.assertTrue(val1 instanceof MyInterface);
  arktest.assertFalse(val2 instanceof MyClass);
  arktest.assertTrue(val2 instanceof MyInterface);
}
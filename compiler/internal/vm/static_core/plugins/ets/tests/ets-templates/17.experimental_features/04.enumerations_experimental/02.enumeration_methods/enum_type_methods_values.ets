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
desc: values() returns an array of enumeration constants in the order of declaration.
name: enum_type_methods_values
---*/

enum Color { Red, Green, blue }

function f(a: FixedArray<Color>): number {
  return a.length;
}

function f(a: Object): int {
  return 2;
}

function main(): void {
  let colors = Color.values();
  arktest.assertEQ(colors[0], Color.Red);
  arktest.assertEQ(colors[1], Color.Green);
  arktest.assertEQ(colors[2], Color.blue);
  arktest.assertEQ(f(colors), 3);
}

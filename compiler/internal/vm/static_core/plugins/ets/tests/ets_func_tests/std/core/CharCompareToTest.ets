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

function Char_compareTo_basic_cases(): void {
  const a = new Char(c'a');
  const b = new Char(c'a');
  const c = new Char(c'b');
  const z = new Char(c'z');

  arktest.assertEQ(a.compareTo(b), 0, "Char: equal should be 0");
  arktest.assertEQ(a.compareTo(c), -1, "Char: 'a' < 'b' => -1");
  arktest.assertEQ(z.compareTo(a), 1, "Char: 'z' > 'a' => 1");
}

function Char_compareTo_unicode_points(): void {
  // Using code units to ensure numerical ordering behavior
  const u0041 = new Char(c'\u0041'); // 'A'
  const u00E9 = new Char(c'\u00E9'); // 'é'
  arktest.assertEQ(u0041.compareTo(u00E9), -1, "Char: 'A' should be <= 'é' by code unit order");
}

function Char_compareTo_antisymmetry(): void {
  const x = new Char(c'c');
  const y = new Char(c'd');
  arktest.assertEQ(x.compareTo(y), -1, "Char antisym: c<d => -1");
  arktest.assertEQ(y.compareTo(x), 1, "Char antisym: d>c => 1");
}
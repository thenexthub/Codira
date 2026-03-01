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

function Byte_compareTo_basic_cases(): void {
  const a = new Byte(1);
  const b = new Byte(1);
  const c = new Byte(2);
  const n = new Byte(-1);

  arktest.assertEQ(a.compareTo(b), 0, "Byte: equal should be 0");
  arktest.assertEQ(a.compareTo(c), -1, "Byte: 1 < 2 => -1");
  arktest.assertEQ(c.compareTo(a), 1, "Byte: 2 > 1 => 1");
  arktest.assertEQ(n.compareTo(a), -1, "Byte: -1 < 1 => -1");
  arktest.assertEQ(a.compareTo(n), 1, "Byte: 1 > -1 => 1");
}

function Byte_compareTo_extremes_and_sign(): void {
  const minv = new Byte(-128);
  const maxv = new Byte(127);

  arktest.assertEQ(minv.compareTo(maxv), -1, "Byte: MIN < MAX => -1");
  arktest.assertEQ(maxv.compareTo(minv), 1, "Byte: MAX > MIN => 1");
}

function Byte_compareTo_antisymmetry(): void {
  const x = new Byte(-128);
  const y = new Byte(127);
  arktest.assertEQ(x.compareTo(y), -1, "Byte antisym: x<y => -1");
  arktest.assertEQ(y.compareTo(x), 1, "Byte antisym: y>x => 1");
}
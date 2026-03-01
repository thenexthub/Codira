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

function Short_compareTo_basic_cases(): void {
  const a = new Short(1);
  const b = new Short(1);
  const c = new Short(2);
  const n = new Short(-1);

  arktest.assertEQ(a.compareTo(b), 0, "Short: equal should be 0");
  arktest.assertEQ(a.compareTo(c), -1, "Short: 1 < 2 => -1");
  arktest.assertEQ(c.compareTo(a), 1, "Short: 2 > 1 => 1");
  arktest.assertEQ(n.compareTo(a), -1, "Short: -1 < 1 => -1");
  arktest.assertEQ(a.compareTo(n), 1, "Short: 1 > -1 => 1");
}

function Short_compareTo_extremes_and_sign(): void {
  const minv = new Short(-32768);
  const maxv = new Short(32767);

  arktest.assertEQ(minv.compareTo(maxv), -1, "Short: MIN < MAX => -1");
  arktest.assertEQ(maxv.compareTo(minv), 1, "Short: MAX > MIN => 1");

  // Ensure it returns only -1/0/1
  const res = maxv.compareTo(minv);
  arktest.assertTrue(res === -1 || res === 0 || res === 1, "Short: result must be -1/0/1");
}

function Short_compareTo_antisymmetry(): void {
  const x = new Short(-12345);
  const y = new Short(12345);
  arktest.assertEQ(x.compareTo(y), -1, "Short antisym: x<y => -1");
  arktest.assertEQ(y.compareTo(x), 1, "Short antisym: y>x => 1");
}
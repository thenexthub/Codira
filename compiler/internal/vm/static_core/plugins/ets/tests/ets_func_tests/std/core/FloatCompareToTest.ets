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

function Float_compareTo_basic_and_sign(): void {
  const a = new Float(1.0);
  const b = new Float(1.0);
  const c = new Float(2.0);
  const n = new Float(-1.0);

  arktest.assertEQ(a.compareTo(b), 0, "Float: equal should be 0");
  arktest.assertEQ(a.compareTo(c), -1, "Float: 1.0 < 2.0 => -1");
  arktest.assertEQ(c.compareTo(a), 1, "Float: 2.0 > 1.0 => 1");
  arktest.assertEQ(n.compareTo(a), -1, "Float: -1.0 < 1.0 => -1");
  arktest.assertEQ(a.compareTo(n), 1, "Float: 1.0 > -1.0 => 1");
}

function Float_compareTo_epsilon_equality(): void {
  const base = new Float(42.0);
  const within = new Float(42.0 + Float.EPSILON * 0.5);
  const justOver = new Float(42.0 + Float.EPSILON * 2.0);

  arktest.assertEQ(base.compareTo(within), 0, "Float: |diff| < EPSILON => 0");
  const s1 = base.compareTo(justOver);
  arktest.assertTrue(s1 === -1 || s1 === 1, "Float: |diff| > EPSILON => not equal");
}

function Float_compareTo_nan_semantics(): void {
  const n = Float.NaN;
  const v = new Float(1.0);
  const m = Float.NaN;
  const a = Float.POSITIVE_INFINITY;
  const b = Float.NEGATIVE_INFINITY;

  arktest.assertEQ(n.compareTo(m), 0, "Float: NaN vs NaN => 0");
  arktest.assertEQ(n.compareTo(v), 1, "Float: this=NaN, other=val => 1");
  arktest.assertEQ(v.compareTo(n), -1, "Float: this=val, other=NaN => -1");
  arktest.assertEQ(n.compareTo(a), 1, "Float: NaN > +Inf => Return 1");
  arktest.assertEQ(a.compareTo(n), -1, "Float: +Inf < NaN => Return -1");
  arktest.assertEQ(n.compareTo(b), 1, "Float: NaN > -Inf => Return 1");
  arktest.assertEQ(b.compareTo(n), -1, "Float: -Inf < NaN => Return -1");
}

function Float_compareTo_zero_and_infinities(): void {
  const pz = new Float(+0.0);
  const nz = new Float(-0.0);
  arktest.assertEQ(pz.compareTo(nz), 0, "Float: +0.0 == -0.0 => 0");

  const pinf = Float.POSITIVE_INFINITY;
  const ninf = Float.NEGATIVE_INFINITY;
  const one = new Float(1.0);

  arktest.assertEQ(pinf.compareTo(one), 1, "Float: +Inf > finite => 1");
  arktest.assertEQ(one.compareTo(pinf), -1, "Float: finite < +Inf => -1");
  arktest.assertEQ(ninf.compareTo(one), -1, "Float: -Inf < finite => -1");
  arktest.assertEQ(one.compareTo(ninf), 1, "Float: finite > -Inf => 1");
  arktest.assertEQ(pinf.compareTo(pinf), 0, "Float: +Inf == +Inf => 0");
  arktest.assertEQ(ninf.compareTo(ninf), 0, "Float: -Inf == -Inf => 0");
}

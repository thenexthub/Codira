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

function Double_compareTo_basic_and_sign(): void {
  const a = new Double(1.0);
  const b = new Double(1.0);
  const c = new Double(2.0);
  const n = new Double(-1.0);

  arktest.assertEQ(a.compareTo(b), 0, "Double: equal should be 0");
  arktest.assertEQ(a.compareTo(c), -1, "Double: 1.0 < 2.0 => -1");
  arktest.assertEQ(c.compareTo(a), 1, "Double: 2.0 > 1.0 => 1");
  arktest.assertEQ(n.compareTo(a), -1, "Double: -1.0 < 1.0 => -1");
  arktest.assertEQ(a.compareTo(n), 1, "Double: 1.0 > -1.0 => 1");
}

function Double_compareTo_epsilon_equality(): void {
  const base = new Double(1.0);
  const within = new Double(1.0 + Double.EPSILON * 0.5);
  const justOver = new Double(1.0 + Double.EPSILON * 2.0);

  arktest.assertEQ(base.compareTo(within), 0, "Double: |diff| < EPSILON => Return0");
  arktest.assertEQ(base.compareTo(justOver), -1, "Double: |diff| > EPSILON => Return -1");
}

function Double_compareTo_nan_semantics(): void {
  const n = Double.NaN;
  const v = 1.0;
  const m = Double.NaN;
  const a = Double.POSITIVE_INFINITY;
  const b = Double.NEGATIVE_INFINITY;

  arktest.assertEQ(n.compareTo(m), 0, "Double: NaN vs NaN => Return 0");
  arktest.assertEQ(n.compareTo(v), 1, "Double: this=NaN, other=val => Return 1");
  arktest.assertEQ(v.compareTo(n), -1, "Double: this=val, other=NaN => Return -1");
  arktest.assertEQ(a.compareTo(b), 1, "Double: +Inf > -Inf => Return 1");
  arktest.assertEQ(b.compareTo(a), -1, "Double: -Inf < +Inf => Return -1");
  arktest.assertEQ(n.compareTo(a), 1, "Double: NaN > +Inf => Return 1");
  arktest.assertEQ(a.compareTo(n), -1, "Double: +Inf < NaN => Return -1");
  arktest.assertEQ(n.compareTo(b), 1, "Double: NaN > -Inf => Return 1");
  arktest.assertEQ(b.compareTo(n), -1, "Double: -Inf < NaN => Return -1");
}

function Double_compareTo_zero_and_infinities(): void {
  const pz = new Double(+0.0);
  const nz = new Double(-0.0);
  arktest.assertEQ(pz.compareTo(nz), 0, "Double: +0.0 == -0.0 => 0");

  const pinf = Double.POSITIVE_INFINITY;
  const ninf = Double.NEGATIVE_INFINITY;
  const one = 1.0;

  arktest.assertEQ(pinf.compareTo(one), 1, "Double: +Inf > finite => 1");
  arktest.assertEQ(one.compareTo(pinf), -1, "Double: finite < +Inf => -1");
  arktest.assertEQ(ninf.compareTo(one), -1, "Double: -Inf < finite => -1");
  arktest.assertEQ(one.compareTo(ninf), 1, "Double: finite > -Inf => 1");
  arktest.assertEQ(pinf.compareTo(pinf), 0, "Double: +Inf == +Inf => 0");
  arktest.assertEQ(ninf.compareTo(ninf), 0, "Double: -Inf == -Inf => 0");
}
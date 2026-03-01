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

class IsIntegerFloatTest {
  public rightInteger: float;
  public wrongInteger: float;
}

class IsIntegerDoubleTest {
  public rightInteger: double;
  public wrongInteger: double;
}

function main(): int {
    let ft = new IsIntegerFloatTest();
    ft.rightInteger = 1.0f;
    ft.wrongInteger = 1.0f + Float.DELTA + Float.DELTA;

    if (Float.isInteger(Float.NaN) != false) {
      return 1;
    }
    if (Float.isInteger(Float.POSITIVE_INFINITY) != false) {
      return 2;
    }
    if (Float.isInteger(Float.NEGATIVE_INFINITY) != false) {
      return 3;
    }
    if (Float.isInteger(ft.rightInteger) != true) {
      return 4;
    }
    if (Float.isInteger(ft.wrongInteger) != false) {
      return 5;
    }

    let dt = new IsIntegerDoubleTest();
    dt.rightInteger = 1.0;
    dt.wrongInteger = 1.0 as double + Double.DELTA + Double.DELTA;

    if (Double.isInteger(Double.NaN) != false) {
      return 21;
    }
    if (Double.isInteger(Double.POSITIVE_INFINITY) != false) {
      return 22;
    }
    if (Double.isInteger(Double.NEGATIVE_INFINITY) != false) {
      return 23;
    }
    if (Double.isInteger(dt.rightInteger) != true) {
      return 24;
    }
    if (Double.isInteger(dt.wrongInteger) != false) {
      return 25;
    }
    return 0;
}

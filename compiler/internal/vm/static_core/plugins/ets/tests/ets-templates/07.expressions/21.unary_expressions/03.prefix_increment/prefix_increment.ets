/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Prefix increment expression
---*/

function main(): void {

  let n: number = -3.3;
  arktest.assertEQ(++n, -2.3)
  arktest.assertEQ(n, -2.3)

  // special values
  n = NaN
  arktest.assertTrue(Number.isNaN(++n))
  arktest.assertTrue(Number.isNaN(n))

  n = +Infinity
  arktest.assertEQ(++n, +Infinity)
  arktest.assertEQ(n, +Infinity)

  let maxByte: byte = 127
  arktest.assertEQ(++maxByte, -128)
  arktest.assertEQ(maxByte, -128)

  let maxShort: short = 32767
  arktest.assertEQ(++maxShort, -32768)
  arktest.assertEQ(maxShort, -32768)

  let maxInt: int = 2147483647
  arktest.assertEQ(++maxInt, -2147483648)
  arktest.assertEQ(maxInt, -2147483648)

  let maxLong: long = 9223372036854775807
  arktest.assertEQ(++maxLong, -9223372036854775808)
  arktest.assertEQ(maxLong, -9223372036854775808)

  // operator precedence
  let l: long = -13;
  arktest.assertEQ(++l, -12)
  arktest.assertEQ(l, -12)
  arktest.assertEQ(-++l, 11)
  arktest.assertEQ(+(++l), -10)
  arktest.assertEQ(l+(++l), -10-9)

  // array
  let an: number[] = [-1.5, 3.14]
  arktest.assertEQ(++an[0], -0.5)
  arktest.assertEQ(an[0], -0.5)

  let ad: double[] = [-1.5, 3.14]
  arktest.assertEQ(++ad[0], -0.5)
  arktest.assertEQ(ad[0], -0.5)

  let af: float[] = [-1.5f, 3.14f]
  arktest.assertEQ(++af[0], -0.5)
  arktest.assertEQ(af[0], -0.5)

  let al: long[] = [13, -42]
  arktest.assertEQ(++al[1], -41)
  arktest.assertEQ(al[1], -41)

  let ai: int[] = [13, -42]
  arktest.assertEQ(++ai[1], -41)
  arktest.assertEQ(ai[1], -41)

  let sa: short[] = [13, -42]
  arktest.assertEQ(++sa[1], -41)
  arktest.assertEQ(sa[1], -41)

  let ab: byte[] = [13, -42]
  arktest.assertEQ(++ab[1], -41)
  arktest.assertEQ(ab[1], -41)
}

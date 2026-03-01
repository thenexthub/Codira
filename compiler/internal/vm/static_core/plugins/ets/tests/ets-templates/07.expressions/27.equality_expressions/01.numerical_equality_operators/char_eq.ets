/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: |-
    A comparison that uses the operators '==' and '===' is evaluated to true if
    both operands are of Type char and have the same value, i.e., both operands represent the same Unicode code point.
    The result produced by a != b and !(a == b) is the same in all cases.
    The result produced by a !== b and !(a === b) is the same.
---*/

function checkEq(c1: char, c2: char): boolean {
  return c1 == c2
}

function checkEqStrict(c1: char, c2: char): boolean {
  return c1 === c2
}

function checkNeq(c1: char, c2: char): boolean {
  return c1 != c2
}

function checkNotEq(c1: char, c2: char): boolean {
  return !(c1 == c2)
}

function checkNeqStrict(c1: char, c2: char): boolean {
  return c1 !== c2
}

function checkNotEqStrict(c1: char, c2: char): boolean {
  return !(c1 === c2)
}

function main(): void {
  let c1 = c'a';
  let c2 = c'a';
  let c3 = c'b';

  // Test equality with same values
  arktest.assertTrue( checkEq(c1, c2) )
  arktest.assertTrue( checkEqStrict(c1, c2) )

  // Test inequality with different values
  arktest.assertTrue( checkNeq(c1, c3) )
  arktest.assertTrue( checkNotEq(c1, c3) )
  arktest.assertTrue( checkNeqStrict(c1, c3) )
  arktest.assertTrue( checkNotEqStrict(c1, c3) )
}

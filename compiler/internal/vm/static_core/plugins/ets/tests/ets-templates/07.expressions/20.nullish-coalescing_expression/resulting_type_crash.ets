/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: The type of a nullish-coalescing expression is a normalized union type (see
  Union Types) formed from the following:\n\u2022 Non-nullish variant of the type
  of the left-hand-side expression; and\n\u2022 Type of the right-hand-side expression.
  #30583 - crash, not CTE
---*/

type int_nullish = int | null | undefined

enum Enum { A = 1.2, B = 0.0}

function main(): void {
  let lhs: Array<Enum> = [Enum.A, Enum.B];
  let rhs: string = "aa";

  let x = lhs[1] ?? rhs;
  arktest.assertTrue(x instanceof double|string)
  // Type double|string leads to crash (not CTE type error, see #30583). Please keep this as separate case
  // With Enum|string it works fine and added to normal tests
  let y: double|string = x
}

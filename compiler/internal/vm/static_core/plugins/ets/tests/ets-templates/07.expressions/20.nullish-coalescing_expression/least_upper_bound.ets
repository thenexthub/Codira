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
desc: The type of a nullish-coalescing expression is the least upper bound (see Least Upper Bound) of the non-nullish variant of the types of the left-hand-side and right-hand-side expressions.
---*/

abstract class ACA {}
abstract class ACB {}
class CAA extends ACA {}
class CAB extends ACA {}
class CAAA extends CAA {}
class CAAB extends CAA {}

class CBA extends ACB {}
class CBB extends ACB {}
class CBBA extends CBB {}
class CBBB extends CBB {}

function main(): void {
  let a: CAAA | null = null;
  let b: CBBB = new CBBB();
  let c = a ?? b;
  arktest.assertTrue( c instanceof ACB )
  arktest.assertFalse( c instanceof ACA )
  //verify exact type
  let d: CAAA | CBBB = c
}

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
desc: Optional chaining declaration with null result.
assert: if ’?.’ is provided and expression is evaluated as null then arguments are not evaluated and the call is not performed and result of the functionCallExpression is null too.
---*/

class A {
  public evaluate(value: int): Int {
    return new Int(value);
  }
}

function getX(): Int|undefined {
  let a: A|undefined;
  let res: Int|undefined = a?.evaluate(5);

  return res;
}

function main(): void {
  arktest.assertTrue( getX() === undefined )
}

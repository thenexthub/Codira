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
desc: ClassCastError is thrown by a cast expression
  if at run time a cast cannot be performed.
---*/

function main() {

  let a = 1;
  let b = 1;
  let t: Long | null = (): Long | null => { return null; }();

  let v: long = (a = 2) + (t === null ? 0 : t) + (b = 2);
  arktest.assertEQ(a, 2);
  arktest.assertEQ(b, 2);
    
}

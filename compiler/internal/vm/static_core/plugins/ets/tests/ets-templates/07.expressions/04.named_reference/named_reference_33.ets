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
desc: The type parameters of the generic class method are not explicit, resulting in a compilation error.
tags:
- negative
- compile-only
---*/

class Myclass<T> {
  value: T;
  constructor(v: T) {
    this.value = v;
  }
  getValue(): T {
    return this.value;
  }
}

function main(): void {
  const stringBox = new Myclass("hello");
  const value = stringBox.getValue();
  arktest.assertEQ(value, "hello");
}
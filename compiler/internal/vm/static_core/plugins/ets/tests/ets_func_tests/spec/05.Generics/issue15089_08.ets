/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 05.Generics
 name: issue15089_01.ets
 tags:
 ---*/

class A<T> {
  data: T
  constructor (p: T) {
    this.data = p
  }
}

function main(): void {
  let x1 = new A<Byte>(1)
  arktest.assertTrue( x1.data instanceof Byte)
  let x2 = new A<Short>(2)
  arktest.assertTrue( x2.data instanceof Short)
  let x3 = new A<Int>(3)
  arktest.assertTrue( x3.data instanceof Int)
  let x4 = new A<Long>(4)
  arktest.assertTrue( x4.data instanceof Long)
  let x5 = new A<BigInt>(5)
  arktest.assertTrue( x5.data instanceof BigInt)
}

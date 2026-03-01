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
class Test {
  test() {
    console.log("test")
  }

  test2() {
  }

  test2(a: int) {
    console.log("test2")
    if(a > 0) {
      return 1
    }
    return -1;
  }

  test3(a: int) {
    try {
      a = a + 1
    } catch(e){
      a = a - 1
      return a
    }
    return a
  }
}

function test0() {
}

function test0(a: int) {
  console.log("test0 ", a)
}

function test1(b: String) {
}

function main() {
  const test = new Test()
  test.test()
  test.test2(1)
  test0()
  test1("hgf")
  test.test2()
}

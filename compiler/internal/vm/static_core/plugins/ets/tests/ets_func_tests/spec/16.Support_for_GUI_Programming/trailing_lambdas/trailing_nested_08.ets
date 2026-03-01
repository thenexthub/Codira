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

/*---
desc: >-
    17.10 Trailing lambda
    16.6.2 Callable Types with Instantiate Method
tags: []
---*/

let global = 0

class A {
  _width: int
  _color: string = ""
  __initializeStruct(): A {
    global++
    return this
  }
  width(value: int): A {
    global++
    this._width = value
    return this
  }
  color(value: string): A {
    global++
    this._color = value
    return this
  }
}

class B extends A{
  static $_instantiate(factory: () => B, content: () => void): B {
    global++
    const instance = factory()
    instance.__initializeStruct()
    content()
    return instance
  }
  constructor() {
    global++
  }
}

class C extends A{
  static $_instantiate(factory: () => C, content: () => void): C {
    global++
    const instance = factory()
    instance.__initializeStruct()
    content()
    return instance
  }
  constructor() {
    global++
  }
}

function main(): void {
    global = 0

    B() {
      C() {
      }
      .color("navy")
    }
    .width(20)
    // Each function called once and A.__initializeStruct twice
    arktest.assertEQ( global, 8)
}

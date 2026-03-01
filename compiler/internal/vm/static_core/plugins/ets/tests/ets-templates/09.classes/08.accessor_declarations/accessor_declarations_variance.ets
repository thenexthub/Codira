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
    In the process of inheriting and overriding, accessors behave as methods. The getter parameter type follows
    the covariance pattern, and the setter parameter type follows the contravariance pattern.
tags: []
---*/

/*
  # inheriting and overriding done right: covariance for the return type
  # and contravariance for parameters
*/

class Base {
  get field(): Base { return new Base }
  set field(a_field: Derived) {}
}

class Derived extends Base {
  override get field(): Derived { return new Derived }
  override set field(a_field: Base) {}
}

function foo (base: Base) {
  base.field = new Derived // setter is called
  let b: Base = base.field // getter is called
  arktest.assertTrue(base.field instanceof Derived)
  arktest.assertTrue(b instanceof Base)
}

function main() {
    foo (new Derived)
}


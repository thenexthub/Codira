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
desc: Derived class may have type compatible constraint of type parameters
---*/

class Base {}
class Derived extends Base {}

class A1 <CovariantTypeParameter extends Base> {}
class B1 <CovariantTypeParameter extends Derived> extends A1<CovariantTypeParameter> {}

let a1 = new A1<Base>
let b1 = new B1<Derived>

function main() {
    arktest.assertTrue(a1 instanceof A1)
    arktest.assertTrue(b1 instanceof B1)
}

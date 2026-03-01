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
desc: Function types conversion is the conversion of one function type to another.
tags: [compile-only]
---*/

class Base {}
class Derived extends Base {}

type FuncTypeBaseBase = (p: Base) => Base
type FuncTypeBaseDerived = (p: Base) => Derived
type FuncTypeDerivedBase = (p: Derived) => Base
type FuncTypeDerivedDerived = (p: Derived) => Derived

function foo (
    bb: FuncTypeBaseBase, bd: FuncTypeBaseDerived, db: FuncTypeDerivedBase, dd: FuncTypeDerivedDerived
) {
    bb = bd  /* OK: identical (invariant) parameter types, and compatible return type */
    db = bd  /* OK: contravariant parameter types, and compatible return type */
}

function main() {
    return
}

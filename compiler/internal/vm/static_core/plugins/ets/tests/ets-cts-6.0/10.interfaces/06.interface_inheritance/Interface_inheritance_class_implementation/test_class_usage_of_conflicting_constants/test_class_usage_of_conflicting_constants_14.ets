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
    Verify that a class can distinguish the same-named constants
    between an interface constant and a protected class property by subclass access.
---*/

interface ProtectedPropConflictInterface {
  readonly CONST: number;
}

class ProtectedPropConflictBaseClass implements ProtectedPropConflictInterface {
  protected readonly _CONST: number = 51;
  readonly CONST: number = 52;
}

class ProtectedPropConflictSubClass extends ProtectedPropConflictBaseClass {
  getProtectedValue() {
    return this._CONST;
  }
}

function main(): void {
  const instance = new ProtectedPropConflictSubClass();
  arktest.assertEQ(instance.getProtectedValue(), 51);
  arktest.assertEQ(instance.CONST, 52);
}

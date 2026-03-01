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
desc: Implement two interfaces, and check cast.
assert: >-
    Given a (possibly generic) class declaration C<F1,...,Fn> (n ≥ 0, C ≠ Object), the direct superinterfaces
    of the class type C<F1,...,Fn> are the types given in the implements clause of the declaration of C,
    if an implements clause is present.
---*/

interface A {}

interface B<T> {}

final class C<T extends Object> implements B<T>, A {}

function main(): void {
  let a: C<String> = new C<String>();
  let b: C<String> = new C<String>();
}

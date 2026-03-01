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
desc: Declare two class with same parameters, A - superclass of B, B - subclass of A.
assert: >-
    Given a generic class declaration C<F1,...,Fn> (n > 0), the direct superclass of the parameterized
    class type C<T1,...,Tn>, where Ti (1 ≤ i ≤ n) is a type, is D<U1 θ,...,Uk θ>, where D<U1,...,Uk> i
    s the direct superclass of C<F1,...,Fn> and θ is the substitution [F1:=T1,...,Fn:=Tn].
tags: [compile-only]
---*/

class C {}

class A<T extends Object, G extends C> {}

final class B<T extends Object, G extends C> extends A<T, G> {}

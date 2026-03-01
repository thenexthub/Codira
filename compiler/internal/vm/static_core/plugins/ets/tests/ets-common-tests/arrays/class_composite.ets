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

class C0 {
   x0: boolean
}

function test1() : C0 {
   return ([{x0: true}])[0]
}

function test2() : C0 {
   return (([[{x0: true}]])[0])[0];
}

function test3() {
    let x0: C0 = ([{x0: true}])[0];
    if (false) {
        return x0;
    }

    // Expecting type to be inferred from the other return expression
    return ([{x0: true}])[0];
}

function testParameter(x : C0) {
}

function main() : void {
   let x0: C0 = ([{x0: true}])[0];
   arktest.assertTrue(x0.x0, 'Incorrect class composite property #1!');

   let x1: C0 = (([[{x0: true}]])[0])[0];
   arktest.assertTrue(x1.x0, 'Incorrect class composite property #2!');

   let x2: C0 = ((([[[{x0: true}]]])[0])[0])[0];
   arktest.assertTrue(x2.x0, 'Incorrect class composite property #3!');

   let f1: C0 = test1();
   arktest.assertTrue(f1.x0, 'Incorrect class composite property #4!');

   let f2: C0 = test2();
   arktest.assertTrue(f2.x0, 'Incorrect class composite property #5!');

   let f3: C0 = test3();
   arktest.assertTrue(f3.x0, 'Incorrect class composite property #6!');

   testParameter(([{x0: true}])[0]);
   testParameter((([[{x0: true}]])[0])[0]);
}

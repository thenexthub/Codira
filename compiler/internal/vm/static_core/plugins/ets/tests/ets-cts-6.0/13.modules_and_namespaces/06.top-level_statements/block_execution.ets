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
desc: A module may contain any number of blocks that merge logically into a single block

---*/

let x: string = '';


pass('H')

{
  pass("e");
}
const foo: String = 'Foo'
{
  pass('l');
}

interface IMyInterface {
    name: String
}

{
  pass('l');
}

class SomeClass {
}

pass('o')
pass(' '); pass('World');

function main(): void {
  arktest.assertEQ('Hello World', x)
  let vi: IMyInterface = {name: 'Bob'}
  arktest.assertEQ('final+final', last)
}

let last: String = "final"
{
    last += "+final"
}

function pass(cc: string): void {
  x += cc;
  arktest.assertTrue( true )
}

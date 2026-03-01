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

package union.test;

type basicUnion = number | string | boolean;

export function testBasicUnion(value: basicUnion) {
  return value;
}

class UnionClass {
  name: string = 'John';
}

class Child1 extends UnionClass {
  sex: string = 'male';
}

class Child2 extends UnionClass {
  language: string = 'English';
}

type childUnion = Child1 | Child2;

export function testChildUnion(arg: childUnion) {
  return true;
}

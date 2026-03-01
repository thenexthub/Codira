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
class C1 {
  public c1M1(){}
  public static c1M2(){}
}

function main() {
  console.log('hello from classes_static.ets')
}

interface I1 {
  i1F1: string
}

@interface A1 {
  a1F1: string
}

@A1({a1F1: 'Hello'})
class C3 {}

final class C2 extends C1 implements I1 {
  public f1: string = 'Hello';
  public f2: number = 1;
  public i1F1: string = 'World';
}

abstract class C4 {}
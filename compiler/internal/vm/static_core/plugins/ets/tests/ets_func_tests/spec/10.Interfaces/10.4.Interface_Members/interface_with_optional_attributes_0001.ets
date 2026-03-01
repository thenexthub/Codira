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
 desc: 10.04.Interface_Members
 name: 10.Interfaces/10.04.Interface_Members/interface_with_optional_attributes_0001.ets
 tags:
 ---*/

interface IStudent {
  name: string;
  age: int;
  desc?: string | undefined;
}

class Student implements IStudent {
  name: string;
  age: int;
  desc?: string | undefined;

  constructor(name: string, age: int, desc: string) {
    this.name = name;
    this.age = age;
    this.desc = desc
  }

  constructor(name: string, age: int) {
    this.name = name;
    this.age = age;
  }
}

function main() : void {
  let s : Student = new Student("Klara", 18, "an excellent student");
  let s1 : Student = new Student("Karl", 18);

  arktest.assertEQ( s.name, "Klara")
  arktest.assertEQ( s.age, 18)
  arktest.assertEQ( s.desc, "an excellent student")
  arktest.assertEQ( s1.name, "Karl")
  arktest.assertEQ( s1.age, 18)
  arktest.assertEQ( s1.desc, undefined)
}

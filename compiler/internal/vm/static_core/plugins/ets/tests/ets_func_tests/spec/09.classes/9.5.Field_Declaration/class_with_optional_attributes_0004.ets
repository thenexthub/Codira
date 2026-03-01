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
 desc: 09.05.Field_Ddeclarations
 name: 09.Classes/09.05.Field_Ddeclarations/class_with_optional_attributes_0004.ets
 tags:
 ---*/

class Teacher {
  name: string;

  constructor(name: string) {
    this.name = name;
  }
}

class Student {
  name?: string;
  age?: int;
  desc?: string;
  teacher?: Teacher

  constructor(name: string, age: int, desc: string, teacher: Teacher) {
    this.name = name;
    this.age = age;
    this.desc = desc
    this.teacher = teacher
  }
}

function main() : void {
  let s0 : Student = new Student("Klara", 18, "an excellent student", new Teacher("Anna"));

  arktest.assertEQ( s0.name, "Klara")
  arktest.assertEQ( s0.age, 18)
  arktest.assertEQ( s0.desc, "an excellent student")
  arktest.assertEQ( s0.teacher?.name, "Anna")
}

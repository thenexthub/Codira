/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
/*---
 desc: 13.03 Import Directives
 name: 13.compilation_units/13.03.import_directives/to_export
 tags: [not-a-test]
 ---*/

class T {
  field1: string;
  field2: number;

  constructor (field1: string, field2: number){
    this.field1 = field1
    this.field2 = field2
  }
}

export function Foo1(bar: T): string{
    return bar.field1
}

export function Foo2(f1: string, f2: number): T {
    return new T(f1, f2)
}

export let Origin = new T("text", 0)

 class NotExported {
  fieldNE1: string;
  fieldNE2: number;

  constructor (fieldNE1: string, fieldNE2: number){
    this.fieldNE1 = fieldNE1
    this.fieldNE2 = fieldNE2
  }
}

export class S extends T {
  constructor (field3: string, field4: number){
    super(field3, field4);
  }

}

export class P {
  field5: string = "someString";
  filed6: number = 23;
  filed7: string = "otherString";

  constructor (field5: string, field6: number, filed7: string){
    this.field5 = field5
    this.field6 = field6
    this.field7 = field7
  }
}



/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Declare field with name used in superinterface.
assert: >-
    If a class declares a field with a certain name, then the declaration of that field is said to hide any and all
    accessible declarations of field with the same name in superclasses and superinterfaces if the class.
---*/

interface A {
  get x(): boolean;
}

class B implements A {
  public x: boolean;

  public getX(): boolean {
    return this.x;
  }
}

function main(): void {
    let instance: B = new B();

    arktest.assertEQ( instance.getX(), false );
}

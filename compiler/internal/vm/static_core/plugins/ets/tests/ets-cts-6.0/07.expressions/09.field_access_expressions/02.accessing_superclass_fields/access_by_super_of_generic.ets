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
desc: >-
    The field access expression super.f is handled in the same way as the expression this.f in the body of class S.
     Assuming that super.f appears within class C, f is accessible in S from class C (see Scopes) while
     the direct superclass of the class denoted by T is a class with S as its fully qualified name.
---*/

class A {
  private _x: int = 1;

  get x(): int{
    return this._x;
  }
}

class B extends A{
  public y: int;
}

final class C<T> extends A {
  public bar(): int {
    return super.x;
  }
}

function main(): void {
  let instance: C<B> = new C<B>();
  arktest.assertEQ( instance.bar(), 1 )
}

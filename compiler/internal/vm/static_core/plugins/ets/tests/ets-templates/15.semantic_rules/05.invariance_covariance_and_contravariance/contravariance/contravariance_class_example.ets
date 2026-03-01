/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    Variance is how subtyping between class types relates to subtyping between class member signatures
    (types of parameters, return type). Variance can be of three kinds:
    - Invariance,
    - Covariance, and
    - Contravariance.
    Contravariance is the ability to use a type that is more general than originally specified.

    contravariance for parameter types: Base is a supertype for Derived
---*/

let res: int = 0

class Base {
  foo(p: Derived): Derived {
    res = 1
    return p;
  }
}

class Derived extends Base {
  override foo(p: Base): Derived {
    res = 2
    return new Derived();
  }
}

function main() {
  let a = new Base();
  let b = new Derived();
  a.foo(b)
  arktest.assertEQ( res, 1 )
  b.foo(a)
  arktest.assertEQ( res, 2 )
}

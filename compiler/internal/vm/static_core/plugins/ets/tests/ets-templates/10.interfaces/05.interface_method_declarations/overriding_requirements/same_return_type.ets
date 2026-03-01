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
desc: Override public default method from superinterface with same return type.
assert: >-
      The relationship between the return type of an interface method and the return types
      of any overridden interface methods is specified in link to class’ requirements
      in overriding and hiding.
tags: [compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): int {
    return 2;
  }
}

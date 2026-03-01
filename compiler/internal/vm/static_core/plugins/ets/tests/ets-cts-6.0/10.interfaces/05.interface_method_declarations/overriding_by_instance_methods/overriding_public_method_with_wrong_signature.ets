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
desc: Override public default method from superinterface with wrong signature.
assert: >-
      An instance method mI declared in or inherited by interface I,
      overrides from I another instance method mJ declared in interface J,
      if all of the following are true - The signature of mI is a subsignature of the signature of mJ.
tags: [negative, compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): boolean {
    return true;
  }
}

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
desc: Check interface with internal access modifier in package.
assert: >-
    The modifier internal indicates that a class member, a constructor, or an interface member is accessible
    within its compilation unit only. If the compilation unit is a package, then internal members can be used
    in any package module.
tags: [negative, compile-only]
---*/

package Test;

class C {
  internal interface I { fee(): int }
}

class B implements C.I {
  public override fee(): int {
    return 1;
  }
}

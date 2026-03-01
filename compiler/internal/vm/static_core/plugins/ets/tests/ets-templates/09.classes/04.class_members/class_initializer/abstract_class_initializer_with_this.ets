/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc:  >-
      In addition, a compile-time error occurs if a class initializer contains the following:
      - Keyword this (see this Expression);
      Abstract class case.
tags: [compile-only, negative]
---*/

abstract class A {
  public x: int;

  static { 
    this.x = 5;
  }
}

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
desc: Constructors and class initializers are not members and are not inherited.
tags: [negative, compile-only]
---*/

interface IA {
  static a: int = 0;
  static aa(): int { return 1;}
}

class CA {
  static f: int = 0;
  static ff(): void {};
}

class CB extends CA {
}

function main(): void {
    let a = new CA();
    let b = new CB();
    a.f;
    a.ff();
    a.a;
    a.aa();
    b.f;
    b.ff();
    b.a;
    b.aa();
}

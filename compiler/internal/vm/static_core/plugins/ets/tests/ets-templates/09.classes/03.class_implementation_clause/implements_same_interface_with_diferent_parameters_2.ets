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
desc: Class implements two instances of generic superinterface.
assert: >-
    A compile-time error occurs if a class is at the same time a subtype of the instantiation of a generic interface,
    and a raw type that names the a generic interface.
tags: [compile-only, negative]
---*/

class Base {}
class Derived extends Base {}

interface IA <T> {}

class IC implements IA<Base>, IA<Derived> {}

/*---
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
---*/

{% for case in cases %}

/*---
desc: >-
    If no constraint is declared, then the type parameter is not compatible with Object
    If type parameter T has type constraint S, then the
    actual type of the generic instantiation must be assignable to S (see Assignability).
tags: [{{case.tags}}]
---*/

class Base {}
class Derived extends Base { }
class SomeType { }

class G<T extends Base> { }
class H<T extends Base|SomeType> {}
class Exotic<T extends "aa"| "bb"> {}

class A {
  f1: number = 0
  f2: string = ""
  f3: boolean = false
}

class B <T extends keyof A> {}

{{case.decl}}

{% endfor %}

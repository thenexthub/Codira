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

/*---
desc: >-
    No type parameter has a default value, and initialization is mandatory for variables and fields of a type parameter (see
    Field Initialization)
tags: [compile-only, negative]
---*/

class G<T extends String> {
  foo() {
    let t: T
    let foo = t + "bar" // TypeError: variable 't' is used before being assigned
  }
}

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

{% for c in cases %}

/*---
desc: >-
    If there are two classes, Base and Derived, and class Derived
    overrides the method foo() of Base, then foo() in Base has signature
    S1 <V1 , ... Vk > (U1 , ..., Un ) :Un+1
    and foo() in Derived has signature
    S2 <W1 , ... Wl > (T1 , ..., Tm ) :Tm+1
    The signature S2 is override-compatible with S1 only if all of the
    following conditions are met:
    - Number of parameters of both methods is the same, i.e., n = m.
    - Each type Ti is override-compatible with type Ui for i in 1..n+1.
    Type override compatibility is defined below.
    - Number of type parameters of either method is the same, i.e., k = l.
    - Constraints of W1 , . . . Wl are to be contravariant (see Contravariance)
    to the appropriate constraints of V1 , . . . Vk .
tags: [{{c.tags}}]
---*/

let res: number = 0;

class Base {
    {{c.base|indent}}
}

class Derived extends Base {
    {{c.derived|indent}}
}

function main() {
    {{(c.call or '')|indent}}
}
{% endfor %}

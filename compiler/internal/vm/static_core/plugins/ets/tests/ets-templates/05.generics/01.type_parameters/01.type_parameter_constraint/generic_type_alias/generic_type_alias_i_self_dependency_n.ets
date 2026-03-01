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

{% for sd in self_dependency %}
{% for kind in kinds %}

/*---
desc: A compile-time error occurs if a type parameter in the type parameter section depends on itself.
tags: [compile-only, negative]
params: >
  {{sd}}
---*/
{{kind}}  SomeType {}
type T = SomeType

{{kind}}  SomeType2<Z> {}
type T2<Z> = SomeType2<Z>

{{kind}}  SomeType3<Z> {}
type T3<Z> = SomeType3<Z>

{{kind}}  SomeType4<Z> {}
type T4<Z> = SomeType4<Z>

interface X<{{sd}}> {}

function main(): void {}

{% endfor %}
{% endfor %}
